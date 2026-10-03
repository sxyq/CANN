#include <acl/acl.h>
#include "judge_abi.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <type_traits>
#include <vector>

extern "C" void run_kernel(
    GM_ADDR x, const TensorGroupInfo& infoX,
    GM_ADDR residual, const TensorGroupInfo& infoResidual,
    GM_ADDR gamma, const TensorGroupInfo& infoGamma,
    GM_ADDR bias, const TensorGroupInfo& infoBias,
    GM_ADDR output, const TensorGroupInfo& infoOutput,
    int64_t availableCoreNum, aclrtStream stream, float epsilon);

namespace {

constexpr float kEpsilon = 1.0e-5f;

uint16_t FloatToHalf(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    const uint16_t sign = static_cast<uint16_t>((bits >> 16) & 0x8000u);
    const uint32_t exponent = (bits >> 23) & 0xffu;
    uint32_t mantissa = bits & 0x7fffffu;
    if (exponent == 0xffu) {
        if (mantissa == 0) {
            return static_cast<uint16_t>(sign | 0x7c00u);
        }
        return static_cast<uint16_t>(sign | 0x7c00u | (mantissa >> 13) | 1u);
    }

    int32_t halfExponent = static_cast<int32_t>(exponent) - 127 + 15;
    if (halfExponent >= 31) {
        return static_cast<uint16_t>(sign | 0x7c00u);
    }
    if (halfExponent <= 0) {
        if (halfExponent < -10) {
            return sign;
        }
        mantissa |= 0x800000u;
        const uint32_t shift = static_cast<uint32_t>(14 - halfExponent);
        uint32_t rounded = mantissa >> shift;
        const uint32_t remainder = mantissa & ((1u << shift) - 1u);
        const uint32_t halfway = 1u << (shift - 1u);
        if (remainder > halfway || (remainder == halfway && (rounded & 1u))) {
            ++rounded;
        }
        return static_cast<uint16_t>(sign | rounded);
    }

    uint32_t rounded = mantissa >> 13;
    const uint32_t remainder = mantissa & 0x1fffu;
    if (remainder > 0x1000u || (remainder == 0x1000u && (rounded & 1u))) {
        ++rounded;
        if (rounded == 0x400u) {
            rounded = 0;
            ++halfExponent;
            if (halfExponent >= 31) {
                return static_cast<uint16_t>(sign | 0x7c00u);
            }
        }
    }
    return static_cast<uint16_t>(sign | (static_cast<uint16_t>(halfExponent) << 10) |
                                 static_cast<uint16_t>(rounded));
}

float HalfToFloat(uint16_t value)
{
    const float sign = (value & 0x8000u) ? -1.0f : 1.0f;
    const uint32_t exponent = (value >> 10) & 0x1fu;
    const uint32_t mantissa = value & 0x3ffu;
    if (exponent == 0) {
        return sign * std::ldexp(static_cast<float>(mantissa), -24);
    }
    if (exponent == 0x1fu) {
        return mantissa == 0 ? sign * INFINITY : NAN;
    }
    return sign * std::ldexp(static_cast<float>(1024u + mantissa),
                             static_cast<int>(exponent) - 25);
}

uint16_t FloatToBfloat(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    if ((bits & 0x7f800000u) == 0x7f800000u && (bits & 0x007fffffu) != 0) {
        return static_cast<uint16_t>((bits >> 16) | 1u);
    }
    bits += 0x7fffu + ((bits >> 16) & 1u);
    return static_cast<uint16_t>(bits >> 16);
}

float BfloatToFloat(uint16_t value)
{
    const uint32_t bits = static_cast<uint32_t>(value) << 16;
    float result = 0.0f;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

template <typename Storage>
Storage Encode(float value, int32_t dtype)
{
    if constexpr (std::is_same<Storage, float>::value) {
        return value;
    } else {
        return dtype == 1 ? FloatToHalf(value) : FloatToBfloat(value);
    }
}

template <typename Storage>
float Decode(Storage value, int32_t dtype)
{
    if constexpr (std::is_same<Storage, float>::value) {
        return value;
    } else {
        return dtype == 1 ? HalfToFloat(value) : BfloatToFloat(value);
    }
}

struct DeviceBuffer {
    void* pointer = nullptr;

    ~DeviceBuffer()
    {
        if (pointer != nullptr) {
            aclrtFree(pointer);
        }
    }

    bool Allocate(size_t bytes, const char* label)
    {
        const aclError rc = aclrtMalloc(&pointer, bytes, ACL_MEM_MALLOC_HUGE_FIRST);
        if (rc != ACL_SUCCESS) {
            std::cerr << "ACL_ALLOC=" << label << " RC=" << rc << '\n';
            return false;
        }
        return true;
    }
};

float InputX(size_t index)
{
    const int value = static_cast<int>((index * 37u + 19u) % 113u) - 56;
    return static_cast<float>(value) * (1.0f / 128.0f);
}

float InputResidual(size_t index)
{
    const int value = static_cast<int>((index * 23u + 7u) % 89u) - 44;
    return static_cast<float>(value) * (1.0f / 256.0f);
}

float InputGamma(size_t index)
{
    return 0.75f + static_cast<float>((index * 5u + 3u) % 33u) * (1.0f / 128.0f);
}

float InputBias(size_t index)
{
    const int value = static_cast<int>((index * 11u + 2u) % 17u) - 8;
    return static_cast<float>(value) * (1.0f / 256.0f);
}

template <typename Storage>
bool RunTypedCase(const char* name, int32_t dtype, int32_t deviceId,
                  int64_t availableCoreNum, uint64_t rows, uint64_t width,
                  aclrtStream stream)
{
    const size_t elementCount = static_cast<size_t>(rows * width);
    const size_t parameterCount = static_cast<size_t>(width);
    const size_t dataBytes = elementCount * sizeof(Storage);
    const size_t parameterBytes = parameterCount * sizeof(Storage);
    std::vector<Storage> x(elementCount);
    std::vector<Storage> residual(elementCount);
    std::vector<Storage> gamma(parameterCount);
    std::vector<Storage> bias(parameterCount);
    std::vector<Storage> output(elementCount);
    std::vector<float> expected(elementCount);

    for (size_t i = 0; i < elementCount; ++i) {
        x[i] = Encode<Storage>(InputX(i), dtype);
        residual[i] = Encode<Storage>(InputResidual(i), dtype);
    }
    for (size_t i = 0; i < parameterCount; ++i) {
        gamma[i] = Encode<Storage>(InputGamma(i), dtype);
        bias[i] = Encode<Storage>(InputBias(i), dtype);
    }

    const float invWidth = 1.0f / static_cast<float>(width);
    for (uint64_t row = 0; row < rows; ++row) {
        double squareSum = 0.0;
        for (uint64_t col = 0; col < width; ++col) {
            const size_t index = static_cast<size_t>(row * width + col);
            float value = Decode(x[index], dtype) + Decode(residual[index], dtype);
            if (dtype == 1) {
                value = HalfToFloat(FloatToHalf(value));
            }
            squareSum += static_cast<double>(value) * static_cast<double>(value);
        }
        const float sum = static_cast<float>(squareSum);
        const float meanSquare = sum * invWidth + kEpsilon;
        const float invRms = 1.0f / std::sqrt(meanSquare);
        for (uint64_t col = 0; col < width; ++col) {
            const size_t index = static_cast<size_t>(row * width + col);
            float value = Decode(x[index], dtype) + Decode(residual[index], dtype);
            if (dtype == 1) {
                value = HalfToFloat(FloatToHalf(value));
            }
            float result = value * invRms;
            if (dtype == 1) {
                result = HalfToFloat(FloatToHalf(result));
                result = HalfToFloat(FloatToHalf(result * Decode(gamma[col], dtype)));
                result = HalfToFloat(FloatToHalf(result + Decode(bias[col], dtype)));
            } else {
                result = result * Decode(gamma[col], dtype);
                result = result + Decode(bias[col], dtype);
                if (dtype == 2) {
                    result = BfloatToFloat(FloatToBfloat(result));
                }
            }
            expected[index] = result;
        }
    }

    DeviceBuffer deviceX;
    DeviceBuffer deviceResidual;
    DeviceBuffer deviceGamma;
    DeviceBuffer deviceBias;
    DeviceBuffer deviceOutput;
    if (!deviceX.Allocate(dataBytes, "x") ||
        !deviceResidual.Allocate(dataBytes, "residual") ||
        !deviceGamma.Allocate(parameterBytes, "gamma") ||
        !deviceBias.Allocate(parameterBytes, "bias") ||
        !deviceOutput.Allocate(dataBytes, "output")) {
        return false;
    }

    auto copyToDevice = [](void* destination, size_t bytes, const void* source,
                           const char* label) {
        const aclError rc = aclrtMemcpy(destination, bytes, source, bytes,
                                        ACL_MEMCPY_HOST_TO_DEVICE);
        if (rc != ACL_SUCCESS) {
            std::cerr << "ACL_COPY_IN=" << label << " RC=" << rc << '\n';
            return false;
        }
        return true;
    };
    if (!copyToDevice(deviceX.pointer, dataBytes, x.data(), "x") ||
        !copyToDevice(deviceResidual.pointer, dataBytes, residual.data(), "residual") ||
        !copyToDevice(deviceGamma.pointer, parameterBytes, gamma.data(), "gamma") ||
        !copyToDevice(deviceBias.pointer, parameterBytes, bias.data(), "bias")) {
        return false;
    }

    const int64_t shape[2] = {static_cast<int64_t>(rows), static_cast<int64_t>(width)};
    const int64_t parameterShape[1] = {static_cast<int64_t>(width)};
    const TensorInfo xInfo{shape, 2, dtype};
    const TensorInfo residualInfo{shape, 2, dtype};
    const TensorInfo gammaInfo{parameterShape, 1, dtype};
    const TensorInfo biasInfo{parameterShape, 1, dtype};
    const TensorInfo outputInfo{shape, 2, dtype};
    const TensorGroupInfo infoX{&xInfo, 1};
    const TensorGroupInfo infoResidual{&residualInfo, 1};
    const TensorGroupInfo infoGamma{&gammaInfo, 1};
    const TensorGroupInfo infoBias{&biasInfo, 1};
    const TensorGroupInfo infoOutput{&outputInfo, 1};

    run_kernel(deviceX.pointer, infoX, deviceResidual.pointer, infoResidual,
               deviceGamma.pointer, infoGamma, deviceBias.pointer, infoBias,
               deviceOutput.pointer, infoOutput, availableCoreNum, stream, kEpsilon);
    aclError rc = aclrtSynchronizeStream(stream);
    if (rc != ACL_SUCCESS) {
        std::cerr << "CASE=" << name << " RC=2 ACL_SYNC=" << rc << '\n';
        return false;
    }
    rc = aclrtMemcpy(output.data(), dataBytes, deviceOutput.pointer, dataBytes,
                     ACL_MEMCPY_DEVICE_TO_HOST);
    if (rc != ACL_SUCCESS) {
        std::cerr << "CASE=" << name << " RC=2 ACL_COPY_OUT=" << rc << '\n';
        return false;
    }

    const float rtol = dtype == 0 ? std::ldexp(1.0f, -10) :
                       dtype == 1 ? std::ldexp(1.0f, -9) : std::ldexp(1.0f, -6);
    const float atol = dtype == 0 ? std::ldexp(1.0f, -16) :
                       dtype == 1 ? std::ldexp(1.0f, -9) : std::ldexp(1.0f, -6);
    const float maxAbsLimit = dtype == 0 ? 1.0e-2f : dtype == 1 ? 1.0e-1f : 1.0f;
    size_t matched = 0;
    float maxAbsError = 0.0f;
    float maxExcess = -INFINITY;
    for (size_t i = 0; i < elementCount; ++i) {
        const float actual = Decode(output[i], dtype);
        const float error = std::fabs(actual - expected[i]);
        const float tolerance = atol + rtol * std::fabs(expected[i]);
        maxAbsError = std::max(maxAbsError, error);
        maxExcess = std::max(maxExcess, error - tolerance);
        matched += error <= tolerance ? 1u : 0u;
    }
    const double matchedRatio = static_cast<double>(matched) /
                               static_cast<double>(elementCount);
    const bool passed = matchedRatio >= 0.99 && maxAbsError <= maxAbsLimit;
    std::cout << "CASE=" << name << " RC=" << (passed ? 0 : 1)
              << " device=" << deviceId << " dtype=" << dtype
              << " M=" << rows << " D=" << width
              << " matched=" << matched << '/' << elementCount
              << " matched_ratio=" << matchedRatio
              << " max_abs_error=" << maxAbsError
              << " max_tolerance_excess=" << maxExcess
              << " atol=" << atol << " rtol=" << rtol
              << " max_abs_limit=" << maxAbsLimit << '\n';
    return passed;
}

bool RunCase(const char* name, int32_t dtype, int32_t deviceId,
             int64_t availableCoreNum, uint64_t rows, uint64_t width,
             aclrtStream stream)
{
    if (dtype == 0) {
        return RunTypedCase<float>(name, dtype, deviceId, availableCoreNum,
                                   rows, width, stream);
    }
    return RunTypedCase<uint16_t>(name, dtype, deviceId, availableCoreNum,
                                  rows, width, stream);
}

}  // namespace

int main(int argc, char** argv)
{
    const int32_t deviceId = argc > 1 ? std::atoi(argv[1]) : 3;
    std::cout << "NPU_TASK=CASE47-SMALL-CLUSTER-CHAMPION-X_V001_CORRECTNESS"
              << " device=" << deviceId << "\n";

    aclError rc = aclInit(nullptr);
    if (rc != ACL_SUCCESS) {
        std::cerr << "ACL_INIT_RC=" << rc << '\n';
        return 2;
    }
    rc = aclrtSetDevice(deviceId);
    if (rc != ACL_SUCCESS) {
        std::cerr << "ACL_SET_DEVICE_RC=" << rc << '\n';
        aclFinalize();
        return 2;
    }

    int64_t availableCoreNum = 0;
    rc = aclrtGetDeviceInfo(deviceId, ACL_DEV_ATTR_VECTOR_CORE_NUM,
                            &availableCoreNum);
    if (rc != ACL_SUCCESS || availableCoreNum <= 0) {
        std::cerr << "ACL_GET_VECTOR_CORE_NUM_RC=" << rc
                  << " value=" << availableCoreNum << '\n';
        aclrtResetDevice(deviceId);
        aclFinalize();
        return 2;
    }
    std::cout << "A_VECTOR_CORE_NUM=" << availableCoreNum << '\n';

    aclrtStream stream = nullptr;
    rc = aclrtCreateStream(&stream);
    if (rc != ACL_SUCCESS) {
        std::cerr << "ACL_CREATE_STREAM_RC=" << rc << '\n';
        aclrtResetDevice(deviceId);
        aclFinalize();
        return 2;
    }

    const uint64_t twoRows = static_cast<uint64_t>(2 * availableCoreNum);
    const uint64_t oneRow = static_cast<uint64_t>(availableCoreNum);
    bool passed = true;
    passed &= RunCase("PROXY_D257_FP32_M2A", 0, deviceId, availableCoreNum,
                      twoRows, 257, stream);
    passed &= RunCase("PROXY_D257_FP16_M2A", 1, deviceId, availableCoreNum,
                      twoRows, 257, stream);
    passed &= RunCase("PROXY_D257_BF16_M2A", 2, deviceId, availableCoreNum,
                      twoRows, 257, stream);
    passed &= RunCase("PROXY_D257_FP32_MA_FALLBACK", 0, deviceId,
                      availableCoreNum, oneRow, 257, stream);
    passed &= RunCase("PROXY_D256_FP32_M2A_ALIGNED_CONTROL", 0, deviceId,
                      availableCoreNum, twoRows, 256, stream);

    aclrtDestroyStream(stream);
    aclrtResetDevice(deviceId);
    aclFinalize();
    std::cout << "CORRECTNESS_RESULT=" << (passed ? "PASS" : "FAIL") << '\n';
    return passed ? 0 : 1;
}
