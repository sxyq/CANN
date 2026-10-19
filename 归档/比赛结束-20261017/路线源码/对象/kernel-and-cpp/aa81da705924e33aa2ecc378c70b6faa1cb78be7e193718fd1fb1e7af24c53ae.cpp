#include "judge_types.hpp"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

using GM_ADDR = void*;

using KernelFn = void (*)(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);

struct Kernel {
    const char* name;
    KernelFn call;
};

namespace {

constexpr int32_t kFp32 = 0;
constexpr int32_t kFp16 = 1;
constexpr int32_t kBf16 = 2;
constexpr int64_t kAvailableCores = 40;
constexpr float kEpsilon = 1.0e-5f;

struct TestCase {
    int32_t dtype;
    int64_t width;
    int64_t rows;
};

bool Check(aclError status, const char* operation)
{
    if (status == ACL_SUCCESS) return true;
    std::fprintf(stderr, "%s failed: ACL status %d\n", operation, static_cast<int>(status));
    return false;
}

uint16_t FloatToBf16(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    bits += 0x7fffU + ((bits >> 16) & 1U);
    return static_cast<uint16_t>(bits >> 16);
}

float Bf16ToFloat(uint16_t value)
{
    const uint32_t bits = static_cast<uint32_t>(value) << 16;
    float result = 0.0f;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

uint16_t FloatToFp16(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    const uint16_t sign = static_cast<uint16_t>((bits >> 16) & 0x8000U);
    const uint32_t exponent = (bits >> 23) & 0xffU;
    uint32_t mantissa = bits & 0x7fffffU;
    if (exponent == 0xffU) {
        return static_cast<uint16_t>(sign | 0x7c00U | (mantissa == 0 ? 0 : 0x0200U));
    }
    const int32_t halfExponent = static_cast<int32_t>(exponent) - 127 + 15;
    if (halfExponent >= 31) return static_cast<uint16_t>(sign | 0x7c00U);
    if (halfExponent <= 0) {
        if (halfExponent < -10) return sign;
        mantissa |= 0x800000U;
        const uint32_t shift = static_cast<uint32_t>(14 - halfExponent);
        uint32_t result = mantissa >> shift;
        const uint32_t remainder = mantissa & ((1U << shift) - 1U);
        const uint32_t halfway = 1U << (shift - 1U);
        if (remainder > halfway || (remainder == halfway && (result & 1U))) ++result;
        return static_cast<uint16_t>(sign | result);
    }
    uint32_t result = (static_cast<uint32_t>(halfExponent) << 10) | (mantissa >> 13);
    const uint32_t remainder = mantissa & 0x1fffU;
    if (remainder > 0x1000U || (remainder == 0x1000U && (result & 1U))) ++result;
    return static_cast<uint16_t>(sign | result);
}

float Fp16ToFloat(uint16_t value)
{
    const uint32_t sign = static_cast<uint32_t>(value & 0x8000U) << 16;
    uint32_t exponent = (value >> 10) & 0x1fU;
    uint32_t mantissa = value & 0x03ffU;
    uint32_t bits = sign;
    if (exponent == 0) {
        if (mantissa != 0) {
            int32_t unbiasedExponent = -14;
            while ((mantissa & 0x0400U) == 0) {
                mantissa <<= 1;
                --unbiasedExponent;
            }
            mantissa &= 0x03ffU;
            bits |= static_cast<uint32_t>(unbiasedExponent + 127) << 23;
            bits |= mantissa << 13;
        }
    } else if (exponent == 0x1fU) {
        bits |= 0x7f800000U | (mantissa << 13);
    } else {
        exponent = exponent - 15 + 127;
        bits |= exponent << 23;
        bits |= mantissa << 13;
    }
    float result = 0.0f;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

size_t ElementSize(int32_t dtype)
{
    return dtype == kFp32 ? sizeof(float) : sizeof(uint16_t);
}

void Store(std::vector<uint8_t>& data, size_t index, int32_t dtype, float value)
{
    if (dtype == kFp32) {
        std::memcpy(data.data() + index * sizeof(value), &value, sizeof(value));
    } else if (dtype == kFp16) {
        const uint16_t packed = FloatToFp16(value);
        std::memcpy(data.data() + index * sizeof(packed), &packed, sizeof(packed));
    } else {
        const uint16_t packed = FloatToBf16(value);
        std::memcpy(data.data() + index * sizeof(packed), &packed, sizeof(packed));
    }
}

float Load(const std::vector<uint8_t>& data, size_t index, int32_t dtype)
{
    if (dtype == kFp32) {
        float value = 0.0f;
        std::memcpy(&value, data.data() + index * sizeof(value), sizeof(value));
        return value;
    }
    uint16_t packed = 0;
    std::memcpy(&packed, data.data() + index * sizeof(packed), sizeof(packed));
    return dtype == kFp16 ? Fp16ToFloat(packed) : Bf16ToFloat(packed);
}

float Quantize(float value, int32_t dtype)
{
    if (dtype == kFp32) return value;
    std::vector<uint8_t> packed(sizeof(uint16_t));
    Store(packed, 0, dtype, value);
    return Load(packed, 0, dtype);
}

const char* DTypeName(int32_t dtype)
{
    if (dtype == kFp32) return "fp32";
    return dtype == kFp16 ? "fp16" : "bf16";
}

bool RunCase(const Kernel& kernel, aclrtStream stream, int device, const TestCase& test)
{
    const size_t inputElements = static_cast<size_t>(test.rows * test.width);
    const size_t vectorElements = static_cast<size_t>(test.width);
    const size_t inputBytes = inputElements * ElementSize(test.dtype);
    const size_t vectorBytes = vectorElements * ElementSize(test.dtype);
    std::vector<uint8_t> x(inputBytes), residual(inputBytes), gamma(vectorBytes), bias(vectorBytes), output(inputBytes);
    for (size_t i = 0; i < inputElements; ++i) {
        Store(x, i, test.dtype, static_cast<float>(static_cast<int>((i * 17) % 101) - 50) * 0.0015f);
        Store(residual, i, test.dtype, static_cast<float>(static_cast<int>((i * 13) % 71) - 35) * 0.0011f);
    }
    for (size_t j = 0; j < vectorElements; ++j) {
        Store(gamma, j, test.dtype, 0.8f + static_cast<float>(j % 19) * 0.003f);
        Store(bias, j, test.dtype, static_cast<float>(static_cast<int>(j % 23) - 11) * 0.001f);
    }

    void* deviceX = nullptr;
    void* deviceResidual = nullptr;
    void* deviceGamma = nullptr;
    void* deviceBias = nullptr;
    void* deviceOutput = nullptr;
    void** pointers[] = {&deviceX, &deviceResidual, &deviceGamma, &deviceBias, &deviceOutput};
    const size_t sizes[] = {inputBytes, inputBytes, vectorBytes, vectorBytes, inputBytes};
    const auto release = [&]() {
        for (void** pointer : pointers) {
            if (*pointer != nullptr) {
                aclrtFree(*pointer);
                *pointer = nullptr;
            }
        }
    };
    for (size_t i = 0; i < 5; ++i) {
        if (!Check(aclrtMalloc(pointers[i], sizes[i], ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc")) {
            release();
            return false;
        }
    }
    if (!Check(aclrtMemcpy(deviceX, inputBytes, x.data(), inputBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy x") ||
        !Check(aclrtMemcpy(deviceResidual, inputBytes, residual.data(), inputBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy residual") ||
        !Check(aclrtMemcpy(deviceGamma, vectorBytes, gamma.data(), vectorBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma") ||
        !Check(aclrtMemcpy(deviceBias, vectorBytes, bias.data(), vectorBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy bias") ||
        !Check(aclrtMemset(deviceOutput, inputBytes, 0xa5, inputBytes), "init output")) {
        release();
        return false;
    }

    const int64_t inputShape[] = {test.rows, test.width};
    const int64_t vectorShape[] = {test.width};
    const TensorInfo inputInfo{inputShape, 2, test.dtype};
    const TensorInfo vectorInfo{vectorShape, 1, test.dtype};
    const TensorGroupInfo inputGroup{&inputInfo, 1};
    const TensorGroupInfo vectorGroup{&vectorInfo, 1};
    kernel.call(deviceX, inputGroup, deviceResidual, inputGroup,
                deviceGamma, vectorGroup, deviceBias, vectorGroup,
                deviceOutput, inputGroup, kAvailableCores, stream, kEpsilon);
    if (!Check(aclrtSynchronizeStream(stream), "aclrtSynchronizeStream") ||
        !Check(aclrtMemcpy(output.data(), inputBytes, deviceOutput, inputBytes, ACL_MEMCPY_DEVICE_TO_HOST), "copy output")) {
        release();
        return false;
    }

    const float tolerance = test.dtype == kFp32 ? 1.0e-4f
                                                : (test.dtype == kFp16 ? 2.5e-3f : 1.5e-2f);
    float maxError = 0.0f;
    size_t mismatches = 0;
    for (int64_t row = 0; row < test.rows; ++row) {
        double squareSum = 0.0;
        for (int64_t col = 0; col < test.width; ++col) {
            const size_t index = static_cast<size_t>(row * test.width + col);
            const float value = Load(x, index, test.dtype) + Load(residual, index, test.dtype);
            squareSum += static_cast<double>(value) * value;
        }
        const float invRms = 1.0f / std::sqrt(static_cast<float>(squareSum / test.width) + kEpsilon);
        for (int64_t col = 0; col < test.width; ++col) {
            const size_t index = static_cast<size_t>(row * test.width + col);
            const float value = (Load(x, index, test.dtype) + Load(residual, index, test.dtype)) * invRms *
                                Load(gamma, static_cast<size_t>(col), test.dtype) +
                                Load(bias, static_cast<size_t>(col), test.dtype);
            const float expected = Quantize(value, test.dtype);
            const float actual = Load(output, index, test.dtype);
            const float error = std::abs(actual - expected);
            maxError = std::max(maxError, error);
            if (!std::isfinite(actual) || !std::isfinite(expected) || error > tolerance) ++mismatches;
        }
    }
    std::printf("CORRECTNESS side=%s device=%d rows=%lld width=%lld dtype=%s max_abs_error=%.9g tolerance=%.9g mismatches=%zu %s\n",
                kernel.name, device, static_cast<long long>(test.rows), static_cast<long long>(test.width),
                DTypeName(test.dtype), maxError, tolerance, mismatches, mismatches == 0 ? "PASS" : "FAIL");
    release();
    return mismatches == 0;
}

}  // namespace

extern "C" void run_kernel_parent(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);
extern "C" void run_kernel_candidate(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);

int main(int argc, char** argv)
{
    const int device = argc > 1 ? std::atoi(argv[1]) : 7;
    const TestCase tests[] = {
        // Generic D=6144: rows=40 gives localRows=1 and rows=80 gives
        // localRows=2 with the fixed 40-core launch contract.
        {kFp32, 6144, 40},
        {kFp32, 6144, 80},
        {kBf16, 6144, 40},
        {kBf16, 6144, 80},
        {kFp16, 6144, 40},
        {kFp16, 6144, 80},
    };
    if (!Check(aclInit(nullptr), "aclInit") || !Check(aclrtSetDevice(device), "aclrtSetDevice")) return 1;
    aclrtStream stream = nullptr;
    if (!Check(aclrtCreateStream(&stream), "aclrtCreateStream")) {
        aclFinalize();
        return 1;
    }
    bool passed = true;
    const Kernel kernels[] = {
        {"parent", run_kernel_parent},
        {"candidate", run_kernel_candidate},
    };
    for (const Kernel& kernel : kernels) {
        for (const TestCase& test : tests) {
            if (!RunCase(kernel, stream, device, test)) passed = false;
        }
    }
    if (!Check(aclrtDestroyStream(stream), "aclrtDestroyStream")) passed = false;
    if (!Check(aclFinalize(), "aclFinalize")) passed = false;
    std::printf("CORRECTNESS_RESULT=%s\n", passed ? "PASS" : "FAIL");
    return passed ? 0 : 1;
}
