#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#define ACL_CHECK(expr) do { \
    const aclError result = (expr); \
    if (result != ACL_SUCCESS) { \
        std::fprintf(stderr, "ACL failure %s=%d at %s:%d\n", #expr, result, __FILE__, __LINE__); \
        return 1; \
    } \
} while (0)

namespace {
constexpr int32_t kBfloat16 = 2;
constexpr float kEpsilon = 1.0e-5f;
constexpr double kAtol = 0.015625;
constexpr double kRtol = 0.015625;
constexpr double kRequiredMatchedRatio = 0.99;
constexpr double kMaxAbsError = 1.0;

float InputValue(int64_t index, int multiplier, int offset)
{
    return static_cast<float>((index * multiplier + offset) % 997) / 498.0f - 1.0f;
}

uint16_t FloatToBfloat16(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    bits += 0x7fffU + ((bits >> 16) & 1U);
    return static_cast<uint16_t>(bits >> 16);
}

float Bfloat16ToFloat(uint16_t value)
{
    const uint32_t bits = static_cast<uint32_t>(value) << 16;
    float result = 0.0f;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

bool IsTargetWidth(int64_t width)
{
    return width == 2049 || width == 3073 || width == 4095;
}
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        std::fprintf(stderr, "usage: %s device_id width\n", argv[0]);
        return 2;
    }

    const int deviceId = std::atoi(argv[1]);
    const int64_t width = std::strtoll(argv[2], nullptr, 10);
    if (deviceId != 1 || !IsTargetWidth(width)) {
        std::fprintf(stderr, "unsupported probe device=%d width=%lld\n",
                     deviceId, static_cast<long long>(width));
        return 2;
    }

    ACL_CHECK(aclInit(nullptr));
    ACL_CHECK(aclrtSetDevice(deviceId));

    int64_t availableCoreNum = 0;
    ACL_CHECK(aclrtGetDeviceInfo(deviceId, ACL_DEV_ATTR_VECTOR_CORE_NUM, &availableCoreNum));
    if (availableCoreNum <= 0 || availableCoreNum > INT64_MAX / 2) {
        std::fprintf(stderr, "invalid vector core count=%lld\n",
                     static_cast<long long>(availableCoreNum));
        return 2;
    }
    const int64_t rows = 2 * availableCoreNum;
    const size_t elements = static_cast<size_t>(rows * width);
    const size_t dataBytes = elements * sizeof(uint16_t);
    const size_t paramBytes = static_cast<size_t>(width) * sizeof(uint16_t);

    aclrtStream stream = nullptr;
    ACL_CHECK(aclrtCreateStream(&stream));

    int64_t xyShape[] = {rows, width};
    int64_t paramShape[] = {width};
    TensorInfo xTensor[] = {{xyShape, 2, kBfloat16}};
    TensorInfo residualTensor[] = {{xyShape, 2, kBfloat16}};
    TensorInfo gammaTensor[] = {{paramShape, 1, kBfloat16}};
    TensorInfo biasTensor[] = {{paramShape, 1, kBfloat16}};
    TensorInfo outputTensor[] = {{xyShape, 2, kBfloat16}};
    const TensorGroupInfo infoX = {xTensor, 1};
    const TensorGroupInfo infoResidual = {residualTensor, 1};
    const TensorGroupInfo infoGamma = {gammaTensor, 1};
    const TensorGroupInfo infoBias = {biasTensor, 1};
    const TensorGroupInfo infoOutput = {outputTensor, 1};

    std::vector<uint16_t> hostX(elements);
    std::vector<uint16_t> hostResidual(elements);
    std::vector<uint16_t> hostGamma(static_cast<size_t>(width));
    std::vector<uint16_t> hostBias(static_cast<size_t>(width));
    std::vector<uint16_t> hostOutput(elements);

    for (size_t i = 0; i < elements; ++i) {
        hostX[i] = FloatToBfloat16(InputValue(static_cast<int64_t>(i), 37, 11));
        hostResidual[i] = FloatToBfloat16(InputValue(static_cast<int64_t>(i), 17, 3));
    }
    for (int64_t i = 0; i < width; ++i) {
        hostGamma[static_cast<size_t>(i)] = FloatToBfloat16(
            0.75f + static_cast<float>((i * 13) % 100) / 200.0f);
        hostBias[static_cast<size_t>(i)] = FloatToBfloat16(
            static_cast<float>((i * 7) % 100) / 400.0f - 0.125f);
    }

    uint8_t* deviceX = nullptr;
    uint8_t* deviceResidual = nullptr;
    uint8_t* deviceGamma = nullptr;
    uint8_t* deviceBias = nullptr;
    uint8_t* deviceOutput = nullptr;
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceX), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceResidual), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceGamma), paramBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceBias), paramBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceOutput), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMemset(deviceOutput, dataBytes, 0, dataBytes));
    ACL_CHECK(aclrtMemcpy(deviceX, dataBytes, hostX.data(), dataBytes,
                          ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceResidual, dataBytes, hostResidual.data(), dataBytes,
                          ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceGamma, paramBytes, hostGamma.data(), paramBytes,
                          ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceBias, paramBytes, hostBias.data(), paramBytes,
                          ACL_MEMCPY_HOST_TO_DEVICE));

    run_kernel(deviceX, infoX,
               deviceResidual, infoResidual,
               deviceGamma, infoGamma,
               deviceBias, infoBias,
               deviceOutput, infoOutput,
               availableCoreNum, stream, kEpsilon);
    ACL_CHECK(aclrtSynchronizeStream(stream));
    ACL_CHECK(aclrtMemcpy(hostOutput.data(), dataBytes, deviceOutput, dataBytes,
                          ACL_MEMCPY_DEVICE_TO_HOST));

    size_t matched = 0;
    size_t bad = 0;
    double maxAbsError = 0.0;
    const double invWidth = 1.0 / static_cast<double>(width);
    std::vector<double> sums(static_cast<size_t>(rows), 0.0);
    for (int64_t row = 0; row < rows; ++row) {
        double sumSquares = 0.0;
        const size_t rowOffset = static_cast<size_t>(row * width);
        for (int64_t col = 0; col < width; ++col) {
            const size_t index = rowOffset + static_cast<size_t>(col);
            const double value = static_cast<double>(Bfloat16ToFloat(hostX[index])) +
                                 static_cast<double>(Bfloat16ToFloat(hostResidual[index]));
            sumSquares += value * value;
        }
        sums[static_cast<size_t>(row)] = sumSquares * invWidth + kEpsilon;
    }

    for (int64_t row = 0; row < rows; ++row) {
        const size_t rowOffset = static_cast<size_t>(row * width);
        const double invRms = 1.0 / std::sqrt(sums[static_cast<size_t>(row)]);
        for (int64_t col = 0; col < width; ++col) {
            const size_t index = rowOffset + static_cast<size_t>(col);
            const double value = static_cast<double>(Bfloat16ToFloat(hostX[index])) +
                                 static_cast<double>(Bfloat16ToFloat(hostResidual[index]));
            const double expected = value * invRms *
                static_cast<double>(Bfloat16ToFloat(hostGamma[static_cast<size_t>(col)])) +
                static_cast<double>(Bfloat16ToFloat(hostBias[static_cast<size_t>(col)]));
            const double actual = Bfloat16ToFloat(hostOutput[index]);
            const double error = std::fabs(actual - expected);
            maxAbsError = std::max(maxAbsError, error);
            if (std::isfinite(actual) && std::isfinite(expected) &&
                error <= kAtol + kRtol * std::fabs(expected)) {
                ++matched;
            } else {
                ++bad;
            }
        }
    }

    const double matchedRatio = static_cast<double>(matched) / static_cast<double>(elements);
    const bool passed = matchedRatio >= kRequiredMatchedRatio && maxAbsError <= kMaxAbsError;
    std::printf("device=%d dtype=BF16 rows=%lld width=%lld vector_cores=%lld "
                "block_count=%lld local_rows=2 matched_ratio=%.8f bad=%zu "
                "max_abs_error=%.9g atol=%.8g rtol=%.8g max_abs_error_limit=%.1f result=%s\n",
                deviceId, static_cast<long long>(rows), static_cast<long long>(width),
                static_cast<long long>(availableCoreNum),
                static_cast<long long>(availableCoreNum), matchedRatio, bad, maxAbsError,
                kAtol, kRtol, kMaxAbsError, passed ? "PASS" : "FAIL");

    ACL_CHECK(aclrtFree(deviceX));
    ACL_CHECK(aclrtFree(deviceResidual));
    ACL_CHECK(aclrtFree(deviceGamma));
    ACL_CHECK(aclrtFree(deviceBias));
    ACL_CHECK(aclrtFree(deviceOutput));
    ACL_CHECK(aclrtDestroyStream(stream));
    ACL_CHECK(aclFinalize());
    return passed ? 0 : 3;
}
