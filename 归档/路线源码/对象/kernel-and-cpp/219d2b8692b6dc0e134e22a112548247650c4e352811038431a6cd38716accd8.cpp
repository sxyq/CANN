#include "local_abi_shim.h"

#include <acl/acl.h>

#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <vector>

extern "C" void run_kernel(
    GM_ADDR x, const TensorGroupInfo& info_x,
    GM_ADDR residual, const TensorGroupInfo& info_residual,
    GM_ADDR gamma, const TensorGroupInfo& info_gamma,
    GM_ADDR bias, const TensorGroupInfo& info_bias,
    GM_ADDR output, const TensorGroupInfo& info_output,
    int64_t availableCoreNum, aclrtStream stream, float epsilon);

namespace {
constexpr int32_t kFp32 = 0;
constexpr int32_t kFp16 = 1;
constexpr int32_t kBf16 = 2;
constexpr int64_t kRows = 2;
constexpr float kEpsilon = 1.0e-5f;

size_t ElementSize(int32_t dtype)
{
    return dtype == kFp32 ? sizeof(float) : sizeof(uint16_t);
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

void Store(std::vector<uint8_t>& data, size_t index, int32_t dtype, float value)
{
    if (dtype == kFp32) {
        std::memcpy(data.data() + index * sizeof(float), &value, sizeof(value));
    } else if (dtype == kFp16) {
        const _Float16 halfValue = static_cast<_Float16>(value);
        std::memcpy(data.data() + index * sizeof(uint16_t), &halfValue, sizeof(halfValue));
    } else {
        const uint16_t bf16Value = FloatToBf16(value);
        std::memcpy(data.data() + index * sizeof(uint16_t), &bf16Value, sizeof(bf16Value));
    }
}

float Load(const std::vector<uint8_t>& data, size_t index, int32_t dtype)
{
    if (dtype == kFp32) {
        float value = 0.0f;
        std::memcpy(&value, data.data() + index * sizeof(float), sizeof(value));
        return value;
    }
    uint16_t bits = 0;
    std::memcpy(&bits, data.data() + index * sizeof(bits), sizeof(bits));
    if (dtype == kFp16) {
        _Float16 value = 0;
        std::memcpy(&value, &bits, sizeof(bits));
        return static_cast<float>(value);
    }
    return Bf16ToFloat(bits);
}

bool CheckAcl(aclError status, const char* operation)
{
    if (status == ACL_SUCCESS) return true;
    std::fprintf(stderr, "%s failed: ACL status %d\n", operation, static_cast<int>(status));
    return false;
}

bool RunCase(aclrtStream stream, int32_t dtype, int64_t width)
{
    const size_t inputElements = static_cast<size_t>(kRows * width);
    const size_t vectorElements = static_cast<size_t>(width);
    const size_t inputBytes = inputElements * ElementSize(dtype);
    const size_t vectorBytes = vectorElements * ElementSize(dtype);
    std::vector<uint8_t> x(inputBytes), residual(inputBytes), gamma(vectorBytes), bias(vectorBytes), output(inputBytes);
    for (size_t i = 0; i < inputElements; ++i) {
        Store(x, i, dtype, static_cast<float>(static_cast<int>((i * 17) % 101) - 50) * 0.0015f);
        Store(residual, i, dtype, static_cast<float>(static_cast<int>((i * 13) % 71) - 35) * 0.0011f);
    }
    for (size_t j = 0; j < vectorElements; ++j) {
        Store(gamma, j, dtype, 0.8f + static_cast<float>(j % 19) * 0.003f);
        Store(bias, j, dtype, static_cast<float>(static_cast<int>(j % 23) - 11) * 0.001f);
    }

    void *deviceX = nullptr, *deviceResidual = nullptr, *deviceGamma = nullptr;
    void *deviceBias = nullptr, *deviceOutput = nullptr;
    const size_t allocationBytes[] = {inputBytes, inputBytes, vectorBytes, vectorBytes, inputBytes};
    void** devicePointers[] = {&deviceX, &deviceResidual, &deviceGamma, &deviceBias, &deviceOutput};
    const auto release = [&]() {
        for (void** pointer : devicePointers) {
            if (*pointer != nullptr) {
                aclrtFree(*pointer);
                *pointer = nullptr;
            }
        }
    };
    for (size_t i = 0; i < 5; ++i) {
        if (!CheckAcl(aclrtMalloc(devicePointers[i], allocationBytes[i], ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc")) {
            release();
            return false;
        }
    }
    if (!CheckAcl(aclrtMemcpy(deviceX, inputBytes, x.data(), inputBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy x") ||
        !CheckAcl(aclrtMemcpy(deviceResidual, inputBytes, residual.data(), inputBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy residual") ||
        !CheckAcl(aclrtMemcpy(deviceGamma, vectorBytes, gamma.data(), vectorBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma") ||
        !CheckAcl(aclrtMemcpy(deviceBias, vectorBytes, bias.data(), vectorBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy bias")) {
        release();
        return false;
    }

    const int64_t inputShape[] = {kRows, width};
    const int64_t vectorShape[] = {width};
    const TensorInfo inputInfo{inputShape, 2, dtype};
    const TensorInfo vectorInfo{vectorShape, 1, dtype};
    const TensorGroupInfo inputGroup{&inputInfo, 1};
    const TensorGroupInfo vectorGroup{&vectorInfo, 1};
    run_kernel(deviceX, inputGroup, deviceResidual, inputGroup, deviceGamma, vectorGroup,
               deviceBias, vectorGroup, deviceOutput, inputGroup, 1, stream, kEpsilon);
    if (!CheckAcl(aclrtSynchronizeStream(stream), "aclrtSynchronizeStream") ||
        !CheckAcl(aclrtMemcpy(output.data(), inputBytes, deviceOutput, inputBytes, ACL_MEMCPY_DEVICE_TO_HOST), "copy output")) {
        release();
        return false;
    }

    const float tolerance = dtype == kFp32 ? 3.0e-5f : (dtype == kFp16 ? 2.5e-3f : 1.5e-2f);
    float maxError = 0.0f;
    std::vector<uint8_t> expected(ElementSize(dtype));
    for (int64_t row = 0; row < kRows; ++row) {
        double squareSum = 0.0;
        for (int64_t j = 0; j < width; ++j) {
            const size_t index = static_cast<size_t>(row * width + j);
            const float value = Load(x, index, dtype) + Load(residual, index, dtype);
            squareSum += static_cast<double>(value) * value;
        }
        const float invRms = 1.0f / std::sqrt(static_cast<float>(squareSum / width) + kEpsilon);
        for (int64_t j = 0; j < width; ++j) {
            const size_t index = static_cast<size_t>(row * width + j);
            const float value = (Load(x, index, dtype) + Load(residual, index, dtype)) * invRms *
                                Load(gamma, static_cast<size_t>(j), dtype) + Load(bias, static_cast<size_t>(j), dtype);
            Store(expected, 0, dtype, value);
            const float error = std::abs(Load(output, index, dtype) - Load(expected, 0, dtype));
            if (error > maxError) maxError = error;
            if (error > tolerance) {
                std::fprintf(stderr, "dtype=%d width=%lld mismatch at %zu: got %.8g expected %.8g error %.8g\n",
                             dtype, static_cast<long long>(width), index, Load(output, index, dtype),
                             Load(expected, 0, dtype), error);
                release();
                return false;
            }
        }
    }
    std::printf("NPU correctness passed: dtype=%d width=%lld rows=%lld max_abs_error=%.8g\n",
                dtype, static_cast<long long>(width), static_cast<long long>(kRows), maxError);
    release();
    return true;
}
}  // namespace

int main()
{
    if (!CheckAcl(aclInit(nullptr), "aclInit") || !CheckAcl(aclrtSetDevice(4), "aclrtSetDevice")) return 1;
    aclrtStream stream = nullptr;
    if (!CheckAcl(aclrtCreateStream(&stream), "aclrtCreateStream")) return 1;
    bool passed = true;
    for (int32_t dtype : {kFp32, kFp16, kBf16}) {
        for (int64_t width : {2048, 16384, 32768}) {
            if (!RunCase(stream, dtype, width)) {
                passed = false;
                break;
            }
        }
        if (!passed) break;
    }
    CheckAcl(aclrtDestroyStream(stream), "aclrtDestroyStream");
    CheckAcl(aclFinalize(), "aclFinalize");
    return passed ? 0 : 1;
}
