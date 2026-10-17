#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

namespace {
constexpr int32_t kDeviceId = 0;
constexpr int64_t kWidth = 4096;
constexpr float kEpsilon = 1.0e-6f;
constexpr float kAtol = 1.0f / 512.0f;
constexpr float kRtol = 1.0f / 128.0f;
constexpr float kMaxAbsError = 0.05f;

bool CheckAcl(aclError status, const char *call)
{
    if (status == ACL_SUCCESS) {
        return true;
    }
    std::fprintf(stderr, "ACL_ERROR call=%s status=%d\n", call, static_cast<int>(status));
    return false;
}

uint16_t ToHalfBits(float value)
{
    const aclFloat16 halfValue = aclFloatToFloat16(value);
    uint16_t bits = 0;
    std::memcpy(&bits, &halfValue, sizeof(bits));
    return bits;
}

float FromHalfBits(uint16_t bits)
{
    aclFloat16 halfValue;
    std::memcpy(&halfValue, &bits, sizeof(bits));
    return aclFloat16ToFloat(halfValue);
}

bool CopyToDevice(void *dst, const void *src, size_t bytes)
{
    return CheckAcl(aclrtMemcpy(dst, bytes, src, bytes, ACL_MEMCPY_HOST_TO_DEVICE),
                    "aclrtMemcpy(H2D)");
}

bool VerifyOutput(const std::vector<uint16_t> &x, const std::vector<uint16_t> &residual,
                  const std::vector<uint16_t> &gamma, const std::vector<uint16_t> &bias,
                  const std::vector<uint16_t> &output, int64_t rows)
{
    size_t matched = 0;
    float maxAbsError = 0.0f;
    for (int64_t row = 0; row < rows; ++row) {
        const size_t rowOffset = static_cast<size_t>(row * kWidth);
        float squareSum = 0.0f;
        for (int64_t col = 0; col < kWidth; ++col) {
            const size_t index = rowOffset + static_cast<size_t>(col);
            const float value = FromHalfBits(ToHalfBits(
                FromHalfBits(x[index]) + FromHalfBits(residual[index])));
            squareSum += value * value;
        }
        const float invRms = 1.0f / std::sqrt(squareSum / static_cast<float>(kWidth) + kEpsilon);
        for (int64_t col = 0; col < kWidth; ++col) {
            const size_t index = rowOffset + static_cast<size_t>(col);
            const float value = FromHalfBits(ToHalfBits(
                FromHalfBits(x[index]) + FromHalfBits(residual[index])));
            const float normalized = FromHalfBits(ToHalfBits(value * invRms));
            const float scaled = FromHalfBits(ToHalfBits(
                normalized * FromHalfBits(gamma[static_cast<size_t>(col)])));
            const float golden = FromHalfBits(ToHalfBits(
                scaled + FromHalfBits(bias[static_cast<size_t>(col)])));
            const float error = std::fabs(FromHalfBits(output[index]) - golden);
            maxAbsError = std::max(maxAbsError, error);
            if (error <= kAtol + kRtol * std::fabs(golden)) {
                ++matched;
            }
        }
    }
    const size_t elementCount = output.size();
    const double matchedRatio = static_cast<double>(matched) / static_cast<double>(elementCount);
    const bool passed = matchedRatio >= 0.99 && maxAbsError <= kMaxAbsError;
    std::printf("CORRECTNESS_RESULT={\"status\":\"%s\",\"device\":%d,\"dtype\":\"fp16\","
                "\"shape\":[%lld,%lld],\"vector_cores\":%lld,\"matched_ratio\":%.9f,"
                "\"atol\":%.9g,\"rtol\":%.9g,\"max_abs_error\":%.9g,"
                "\"max_abs_error_limit\":%.9g}\n",
                passed ? "PASS" : "FAIL", kDeviceId, static_cast<long long>(rows),
                static_cast<long long>(kWidth), 0LL, matchedRatio, kAtol, kRtol,
                maxAbsError, kMaxAbsError);
    return passed;
}
}  // namespace

int main()
{
    if (!CheckAcl(aclInit(nullptr), "aclInit") ||
        !CheckAcl(aclrtSetDevice(kDeviceId), "aclrtSetDevice")) {
        return 1;
    }
    int64_t vectorCoreNum = 0;
    if (!CheckAcl(aclrtGetDeviceInfo(kDeviceId, ACL_DEV_ATTR_VECTOR_CORE_NUM, &vectorCoreNum),
                  "aclrtGetDeviceInfo") || vectorCoreNum <= 0) {
        return 1;
    }
    const int64_t rows = std::max<int64_t>(128, vectorCoreNum * 2);
    const size_t elementCount = static_cast<size_t>(rows * kWidth);
    const size_t dataBytes = elementCount * sizeof(uint16_t);
    const size_t parameterBytes = static_cast<size_t>(kWidth) * sizeof(uint16_t);
    std::vector<uint16_t> x(elementCount);
    std::vector<uint16_t> residual(elementCount);
    std::vector<uint16_t> gamma(static_cast<size_t>(kWidth));
    std::vector<uint16_t> bias(static_cast<size_t>(kWidth));
    std::vector<uint16_t> output(elementCount, 0);
    for (size_t i = 0; i < elementCount; ++i) {
        const float index = static_cast<float>(i);
        x[i] = ToHalfBits(0.45f * std::sin(index * 0.0137f) - 0.1f);
        residual[i] = ToHalfBits(0.27f * std::cos(index * 0.0091f + 0.3f));
    }
    for (int64_t col = 0; col < kWidth; ++col) {
        const float index = static_cast<float>(col);
        gamma[static_cast<size_t>(col)] = ToHalfBits(
            0.8f + 0.35f * std::sin(index * 0.0023f));
        bias[static_cast<size_t>(col)] = ToHalfBits(0.04f * std::cos(index * 0.0031f));
    }

    void *xDevice = nullptr;
    void *residualDevice = nullptr;
    void *gammaDevice = nullptr;
    void *biasDevice = nullptr;
    void *outputDevice = nullptr;
    const bool allocated =
        CheckAcl(aclrtMalloc(&xDevice, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(x)") &&
        CheckAcl(aclrtMalloc(&residualDevice, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST),
                 "aclrtMalloc(residual)") &&
        CheckAcl(aclrtMalloc(&gammaDevice, parameterBytes, ACL_MEM_MALLOC_HUGE_FIRST),
                 "aclrtMalloc(gamma)") &&
        CheckAcl(aclrtMalloc(&biasDevice, parameterBytes, ACL_MEM_MALLOC_HUGE_FIRST),
                 "aclrtMalloc(bias)") &&
        CheckAcl(aclrtMalloc(&outputDevice, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST),
                 "aclrtMalloc(output)");
    if (!allocated || !CopyToDevice(xDevice, x.data(), dataBytes) ||
        !CopyToDevice(residualDevice, residual.data(), dataBytes) ||
        !CopyToDevice(gammaDevice, gamma.data(), parameterBytes) ||
        !CopyToDevice(biasDevice, bias.data(), parameterBytes)) {
        return 1;
    }

    aclrtStream stream = nullptr;
    if (!CheckAcl(aclrtCreateStream(&stream), "aclrtCreateStream")) {
        return 1;
    }
    const int64_t xShape[] = {rows, kWidth};
    const int64_t parameterShape[] = {kWidth};
    const TensorInfo xTensor{xShape, 2, 1};
    const TensorInfo residualTensor{xShape, 2, 1};
    const TensorInfo gammaTensor{parameterShape, 1, 1};
    const TensorInfo biasTensor{parameterShape, 1, 1};
    const TensorInfo outputTensor{xShape, 2, 1};
    const TensorGroupInfo xGroup{&xTensor, 1};
    const TensorGroupInfo residualGroup{&residualTensor, 1};
    const TensorGroupInfo gammaGroup{&gammaTensor, 1};
    const TensorGroupInfo biasGroup{&biasTensor, 1};
    const TensorGroupInfo outputGroup{&outputTensor, 1};

    run_kernel((GM_ADDR)xDevice, xGroup,
               (GM_ADDR)residualDevice, residualGroup,
               (GM_ADDR)gammaDevice, gammaGroup,
               (GM_ADDR)biasDevice, biasGroup,
               (GM_ADDR)outputDevice, outputGroup,
               vectorCoreNum, stream, kEpsilon);
    if (!CheckAcl(aclrtSynchronizeStream(stream), "aclrtSynchronizeStream") ||
        !CheckAcl(aclrtMemcpy(output.data(), dataBytes, outputDevice, dataBytes,
                              ACL_MEMCPY_DEVICE_TO_HOST), "aclrtMemcpy(D2H)")) {
        return 1;
    }
    const bool passed = VerifyOutput(x, residual, gamma, bias, output, rows);
    aclrtDestroyStream(stream);
    aclrtFree(outputDevice);
    aclrtFree(biasDevice);
    aclrtFree(gammaDevice);
    aclrtFree(residualDevice);
    aclrtFree(xDevice);
    aclrtResetDevice(kDeviceId);
    aclFinalize();
    return passed ? 0 : 2;
}
