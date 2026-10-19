#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <vector>

namespace {
constexpr int32_t kDeviceId = 0;
constexpr int64_t kWidth = 3072;
constexpr float kEpsilon = 1.0e-6f;
constexpr float kAtol = 1.0f / 512.0f;
constexpr float kRtol = 1.0f / 128.0f;
constexpr float kMaxAbsError = 0.05f;
constexpr int kWarmups = 20;
constexpr int kSamples = 32;
constexpr size_t kFullLogicalIoBytes = 4743168;

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
    std::printf("VERIFY label=%s status=%s matched_ratio=%.9f max_abs_error=%.9g\n",
                V035_RUNNER_LABEL, passed ? "PASS" : "FAIL", matchedRatio, maxAbsError);
    return passed;
}

float Percentile(const std::vector<float> &sorted, float fraction)
{
    const float position = fraction * static_cast<float>(sorted.size() - 1);
    const size_t lower = static_cast<size_t>(position);
    const size_t upper = std::min(sorted.size() - 1, lower + 1);
    const float weight = position - static_cast<float>(lower);
    return sorted[lower] + weight * (sorted[upper] - sorted[lower]);
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
    const int64_t launchBlockCount = std::min<int64_t>(rows, vectorCoreNum);
    const int64_t minLocalRows = rows / launchBlockCount;
    const int64_t maxLocalRows = minLocalRows + (rows % launchBlockCount != 0 ? 1 : 0);
    const bool targetDispatch = kWidth == 3072 && minLocalRows > 1;
    const bool candidateBuild = std::strcmp(V035_RUNNER_LABEL, "CANDIDATE") == 0;
    std::printf("DISPATCH_AUDIT={\"label\":\"%s\",\"dtype\":\"fp16\","
                "\"shape\":[%lld,%lld],\"vector_cores\":%lld,\"launch_blocks\":%lld,"
                "\"local_rows_min\":%lld,\"local_rows_max\":%lld,"
                "\"selected_branch\":\"ProcessNarrowMidOverlap_FP16\","
                "\"candidate_source_delta_executed\":%s}\n",
                V035_RUNNER_LABEL, static_cast<long long>(rows),
                static_cast<long long>(kWidth), static_cast<long long>(vectorCoreNum),
                static_cast<long long>(launchBlockCount), static_cast<long long>(minLocalRows),
                static_cast<long long>(maxLocalRows),
                candidateBuild && targetDispatch ? "true" : "false");
    if (!targetDispatch) {
        std::fprintf(stderr, "DISPATCH_AUDIT=FAIL target branch predicates not satisfied\n");
        return 3;
    }
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

    for (int i = 0; i < kWarmups; ++i) {
        run_kernel((GM_ADDR)xDevice, xGroup, (GM_ADDR)residualDevice, residualGroup,
                   (GM_ADDR)gammaDevice, gammaGroup, (GM_ADDR)biasDevice, biasGroup,
                   (GM_ADDR)outputDevice, outputGroup, vectorCoreNum, stream, kEpsilon);
    }
    if (!CheckAcl(aclrtSynchronizeStream(stream), "aclrtSynchronizeStream(warmup)")) {
        return 1;
    }

    aclrtEvent start = nullptr;
    aclrtEvent end = nullptr;
    if (!CheckAcl(aclrtCreateEvent(&start), "aclrtCreateEvent(start)") ||
        !CheckAcl(aclrtCreateEvent(&end), "aclrtCreateEvent(end)")) {
        return 1;
    }
    std::vector<float> timings;
    timings.reserve(kSamples);
    for (int i = 0; i < kSamples; ++i) {
        if (!CheckAcl(aclrtRecordEvent(start, stream), "aclrtRecordEvent(start)")) {
            return 1;
        }
        run_kernel((GM_ADDR)xDevice, xGroup, (GM_ADDR)residualDevice, residualGroup,
                   (GM_ADDR)gammaDevice, gammaGroup, (GM_ADDR)biasDevice, biasGroup,
                   (GM_ADDR)outputDevice, outputGroup, vectorCoreNum, stream, kEpsilon);
        if (!CheckAcl(aclrtRecordEvent(end, stream), "aclrtRecordEvent(end)") ||
            !CheckAcl(aclrtSynchronizeEvent(end), "aclrtSynchronizeEvent(end)")) {
            return 1;
        }
        float elapsedMs = 0.0f;
        if (!CheckAcl(aclrtEventElapsedTime(&elapsedMs, start, end),
                      "aclrtEventElapsedTime")) {
            return 1;
        }
        timings.push_back(elapsedMs * 1000.0f);
    }
    if (!CheckAcl(aclrtMemcpy(output.data(), dataBytes, outputDevice, dataBytes,
                              ACL_MEMCPY_DEVICE_TO_HOST), "aclrtMemcpy(D2H)") ||
        !VerifyOutput(x, residual, gamma, bias, output, rows)) {
        return 2;
    }

    std::vector<float> sorted = timings;
    std::sort(sorted.begin(), sorted.end());
    float sum = 0.0f;
    for (float value : timings) {
        sum += value;
    }
    const float mean = sum / static_cast<float>(kSamples);
    const float median = (sorted[kSamples / 2 - 1] + sorted[kSamples / 2]) * 0.5f;
    float mad = 0.0f;
    float variance = 0.0f;
    for (float value : timings) {
        mad += std::fabs(value - median);
        variance += (value - mean) * (value - mean);
    }
    mad /= static_cast<float>(kSamples);
    const float stdev = std::sqrt(variance / static_cast<float>(kSamples - 1));
    const float cv = stdev / mean;
    std::printf("RUNNER_RESULT={\"label\":\"%s\",\"device\":%d,\"dtype\":\"fp16\","
                "\"shape\":[%lld,%lld],\"vector_cores\":%lld,\"warmups\":%d,"
                "\"samples\":%d,\"full_logical_io_bytes\":%zu,\"raw_device_us\":[",
                V035_RUNNER_LABEL, kDeviceId, static_cast<long long>(rows),
                static_cast<long long>(kWidth), static_cast<long long>(vectorCoreNum),
                kWarmups, kSamples, kFullLogicalIoBytes);
    for (int i = 0; i < kSamples; ++i) {
        std::printf("%s%.6f", i == 0 ? "" : ",", timings[i]);
    }
    std::printf("],\"median_device_us\":%.6f,\"mean_device_us\":%.6f,"
                "\"sample_stdev_device_us\":%.6f,\"cv\":%.6f,"
                "\"mad_device_us\":%.6f,\"p10_device_us\":%.6f,\"p90_device_us\":%.6f,"
                "\"min_device_us\":%.6f,\"max_device_us\":%.6f}\n",
                median, mean, stdev, cv, mad, Percentile(sorted, 0.10f),
                Percentile(sorted, 0.90f), sorted.front(), sorted.back());

    aclrtDestroyEvent(end);
    aclrtDestroyEvent(start);
    aclrtDestroyStream(stream);
    aclrtFree(outputDevice);
    aclrtFree(biasDevice);
    aclrtFree(gammaDevice);
    aclrtFree(residualDevice);
    aclrtFree(xDevice);
    aclrtResetDevice(kDeviceId);
    aclFinalize();
    return 0;
}
