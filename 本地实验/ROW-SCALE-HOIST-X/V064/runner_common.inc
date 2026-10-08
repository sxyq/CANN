#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <vector>

#ifndef V064_LOCAL_MODE
#define V064_LOCAL_MODE 0
#endif

namespace {
constexpr int32_t kDeviceId = 0;
constexpr int64_t kRows = 128;
constexpr int64_t kWidth = 128;
constexpr float kEpsilon = 1.0e-6f;
constexpr float kAtol = 1.0f / 65536.0f;
constexpr float kRtol = 1.0f / 1024.0f;
constexpr float kMaxAbsError = 1.0e-2f;
constexpr int kWarmups = 45;
constexpr int kSamples = 32;

bool CheckAcl(aclError status, const char *call)
{
    if (status == ACL_SUCCESS) {
        return true;
    }
    std::fprintf(stderr, "ACL_ERROR call=%s status=%d\n", call, static_cast<int>(status));
    return false;
}

bool CopyToDevice(void *dst, const void *src, size_t bytes)
{
    return CheckAcl(aclrtMemcpy(dst, bytes, src, bytes, ACL_MEMCPY_HOST_TO_DEVICE),
                    "aclrtMemcpy(H2D)");
}
}  // namespace

int main()
{
    if (!CheckAcl(aclInit(nullptr), "aclInit") ||
        !CheckAcl(aclrtSetDevice(kDeviceId), "aclrtSetDevice")) {
        return 1;
    }

    std::printf("RUNNER_SOURCE_IDENTITY={\"label\":\"%s\",\"source_sha256\":\"%s\"}\n",
                V064_RUNNER_LABEL, V064_SOURCE_SHA256);

    int64_t vectorCoreNum = 0;
    if (!CheckAcl(aclrtGetDeviceInfo(kDeviceId, ACL_DEV_ATTR_VECTOR_CORE_NUM, &vectorCoreNum),
                  "aclrtGetDeviceInfo") || vectorCoreNum <= 0) {
        return 1;
    }
    const int64_t launchBlocks = std::min<int64_t>(kRows, vectorCoreNum);
    const int64_t localRowsMin = kRows / launchBlocks;
    const int64_t localRowsMax = localRowsMin + (kRows % launchBlocks != 0 ? 1 : 0);
    const bool targetDispatch = vectorCoreNum == 40 && launchBlocks == 40 &&
                                localRowsMin == 3 && localRowsMax == 4 && kWidth == 128;
    const bool candidateSourceDelta =
        std::strcmp(V064_RUNNER_LABEL, "CANDIDATE") == 0;
    std::printf("DISPATCH_AUDIT={\"label\":\"%s\",\"dtype\":\"fp32\","
                "\"shape\":[%lld,%lld],\"vector_cores\":%lld,\"launch_blocks\":%lld,"
                "\"local_rows_min\":%lld,\"local_rows_max\":%lld,"
                "\"selected_branch\":\"ProcessSmallFp32ContiguousBatched\","
                "\"candidate_source_delta_executed\":%s}\n",
                V064_RUNNER_LABEL, static_cast<long long>(kRows), static_cast<long long>(kWidth),
                static_cast<long long>(vectorCoreNum), static_cast<long long>(launchBlocks),
                static_cast<long long>(localRowsMin), static_cast<long long>(localRowsMax),
                candidateSourceDelta && targetDispatch ? "true" : "false");
    if (!targetDispatch) {
        std::fprintf(stderr, "DISPATCH_AUDIT=FAIL target branch predicates not satisfied\n");
        return 3;
    }

    const size_t elementCount = static_cast<size_t>(kRows * kWidth);
    const size_t dataBytes = elementCount * sizeof(float);
    const size_t parameterBytes = static_cast<size_t>(kWidth) * sizeof(float);
    std::vector<float> x(elementCount);
    std::vector<float> residual(elementCount);
    std::vector<float> gamma(static_cast<size_t>(kWidth));
    std::vector<float> bias(static_cast<size_t>(kWidth));
    std::vector<float> output(elementCount, 0.0f);
    std::vector<float> golden(elementCount, 0.0f);

    for (size_t i = 0; i < elementCount; ++i) {
        const float index = static_cast<float>(i);
        x[i] = 0.45f * std::sin(index * 0.0137f) - 0.1f;
        residual[i] = 0.27f * std::cos(index * 0.0091f + 0.3f);
    }
    for (int64_t col = 0; col < kWidth; ++col) {
        const float index = static_cast<float>(col);
        gamma[static_cast<size_t>(col)] = 0.8f + 0.35f * std::sin(index * 0.0023f);
        bias[static_cast<size_t>(col)] = 0.04f * std::cos(index * 0.0031f);
    }
    for (int64_t row = 0; row < kRows; ++row) {
        const size_t rowOffset = static_cast<size_t>(row * kWidth);
        float squareSum = 0.0f;
        for (int64_t col = 0; col < kWidth; ++col) {
            const size_t index = rowOffset + static_cast<size_t>(col);
            const float value = x[index] + residual[index];
            squareSum += value * value;
        }
        const float invRms = 1.0f / std::sqrt(squareSum / static_cast<float>(kWidth) + kEpsilon);
        for (int64_t col = 0; col < kWidth; ++col) {
            const size_t index = rowOffset + static_cast<size_t>(col);
            const float value = x[index] + residual[index];
            golden[index] = (value * invRms) * gamma[static_cast<size_t>(col)] +
                            bias[static_cast<size_t>(col)];
        }
    }

    void *xDevice = nullptr;
    void *residualDevice = nullptr;
    void *gammaDevice = nullptr;
    void *biasDevice = nullptr;
    void *outputDevice = nullptr;
    if (!CheckAcl(aclrtMalloc(&xDevice, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(x)") ||
        !CheckAcl(aclrtMalloc(&residualDevice, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(residual)") ||
        !CheckAcl(aclrtMalloc(&gammaDevice, parameterBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(gamma)") ||
        !CheckAcl(aclrtMalloc(&biasDevice, parameterBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(bias)") ||
        !CheckAcl(aclrtMalloc(&outputDevice, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(output)") ||
        !CopyToDevice(xDevice, x.data(), dataBytes) ||
        !CopyToDevice(residualDevice, residual.data(), dataBytes) ||
        !CopyToDevice(gammaDevice, gamma.data(), parameterBytes) ||
        !CopyToDevice(biasDevice, bias.data(), parameterBytes)) {
        return 1;
    }

    aclrtStream stream = nullptr;
    if (!CheckAcl(aclrtCreateStream(&stream), "aclrtCreateStream")) {
        return 1;
    }
    const int64_t xShape[] = {kRows, kWidth};
    const int64_t parameterShape[] = {kWidth};
    const TensorInfo xTensor{xShape, 2, 0};
    const TensorInfo residualTensor{xShape, 2, 0};
    const TensorInfo gammaTensor{parameterShape, 1, 0};
    const TensorInfo biasTensor{parameterShape, 1, 0};
    const TensorInfo outputTensor{xShape, 2, 0};
    const TensorGroupInfo xGroup{&xTensor, 1};
    const TensorGroupInfo residualGroup{&residualTensor, 1};
    const TensorGroupInfo gammaGroup{&gammaTensor, 1};
    const TensorGroupInfo biasGroup{&biasTensor, 1};
    const TensorGroupInfo outputGroup{&outputTensor, 1};

#if V064_LOCAL_MODE
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
        if (!CheckAcl(aclrtEventElapsedTime(&elapsedMs, start, end), "aclrtEventElapsedTime")) {
            return 1;
        }
        timings.push_back(elapsedMs * 1000.0f);
    }
#else
    run_kernel((GM_ADDR)xDevice, xGroup, (GM_ADDR)residualDevice, residualGroup,
               (GM_ADDR)gammaDevice, gammaGroup, (GM_ADDR)biasDevice, biasGroup,
               (GM_ADDR)outputDevice, outputGroup, vectorCoreNum, stream, kEpsilon);
#endif
    if (!CheckAcl(aclrtSynchronizeStream(stream), "aclrtSynchronizeStream(correctness)") ||
        !CheckAcl(aclrtMemcpy(output.data(), dataBytes, outputDevice, dataBytes,
                              ACL_MEMCPY_DEVICE_TO_HOST), "aclrtMemcpy(D2H)")) {
        return 1;
    }

    size_t matched = 0;
    float maxAbsError = 0.0f;
    for (size_t i = 0; i < elementCount; ++i) {
        const float error = std::fabs(output[i] - golden[i]);
        maxAbsError = std::max(maxAbsError, error);
        if (error <= kAtol + kRtol * std::fabs(golden[i])) {
            ++matched;
        }
    }
    const double matchedRatio = static_cast<double>(matched) / static_cast<double>(elementCount);
    const bool passed = matchedRatio >= 0.99 && maxAbsError <= kMaxAbsError;
    std::printf("CORRECTNESS_RESULT={\"label\":\"%s\",\"status\":\"%s\","
                "\"device\":%d,\"dtype\":\"fp32\",\"shape\":[%lld,%lld],"
                "\"vector_cores\":%lld,\"matched_ratio\":%.9f,"
                "\"atol\":%.9g,\"rtol\":%.9g,\"max_abs_error\":%.9g,"
                "\"max_abs_error_limit\":%.9g}\n",
                V064_RUNNER_LABEL, passed ? "PASS" : "FAIL", kDeviceId,
                static_cast<long long>(kRows), static_cast<long long>(kWidth),
                static_cast<long long>(vectorCoreNum), matchedRatio, kAtol, kRtol,
                maxAbsError, kMaxAbsError);

#if V064_LOCAL_MODE
    std::vector<float> sorted = timings;
    std::sort(sorted.begin(), sorted.end());
    float sum = 0.0f;
    for (float value : timings) {
        sum += value;
    }
    const float median = (sorted[kSamples / 2 - 1] + sorted[kSamples / 2]) * 0.5f;
    float mad = 0.0f;
    for (float value : timings) {
        mad += std::fabs(value - median);
    }
    mad /= static_cast<float>(kSamples);
    std::printf("RUNNER_RESULT={\"label\":\"%s\",\"device\":%d,"
                "\"dtype\":\"fp32\",\"shape\":[%lld,%lld],"
                "\"vector_cores\":%lld,\"warmups\":%d,\"samples\":%d,"
                "\"raw_device_us\":[",
                V064_RUNNER_LABEL, kDeviceId, static_cast<long long>(kRows),
                static_cast<long long>(kWidth), static_cast<long long>(vectorCoreNum),
                kWarmups, kSamples);
    for (int i = 0; i < kSamples; ++i) {
        std::printf("%s%.6f", i == 0 ? "" : ",", timings[i]);
    }
    std::printf("],\"median_device_us\":%.6f,\"mean_device_us\":%.6f,"
                "\"mad_device_us\":%.6f}\n", median, sum / static_cast<float>(kSamples), mad);
    aclrtDestroyEvent(end);
    aclrtDestroyEvent(start);
#endif

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
