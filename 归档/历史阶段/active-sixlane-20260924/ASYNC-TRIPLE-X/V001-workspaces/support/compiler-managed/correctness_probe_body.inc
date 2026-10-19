#include <math.h>
#include <stdint.h>
#include <stdio.h>

namespace {

const uint64_t kRows = 1;
const uint64_t kWidth = 1024;
const float kEpsilon = 1.0e-5f;

float InputValue(uint64_t index, uint32_t stream)
{
    const int32_t centered = static_cast<int32_t>(
        (index * (stream * 19u + 23u) + stream * 71u) % 2001u) - 1000;
    const float value = static_cast<float>(centered) / 2500.0f;
    if (stream == 2) {
        return 0.75f + value * 0.25f;
    }
    if (stream == 3) {
        return value * 0.15f;
    }
    return value * 0.5f;
}

int ReportAcl(aclError status, const char* expression)
{
    if (status == ACL_SUCCESS) {
        return 0;
    }
    const char* detail = aclGetRecentErrMsg();
    fprintf(stderr, "ACL_ERROR step=%s code=%d detail=%s\n", expression,
        static_cast<int>(status), detail == nullptr ? "<none>" : detail);
    return 1;
}

}  // namespace

int main()
{
    const size_t elements = static_cast<size_t>(kRows * kWidth);
    const size_t bytes = elements * sizeof(float);
    float xHost[1024];
    float residualHost[1024];
    float gammaHost[1024];
    float biasHost[1024];
    float outputHost[1024] = {};
    int64_t dataShape[2] = {1, 1024};
    int64_t vectorShape[1] = {1024};
    TensorInfo dataTensor = {dataShape, 2, 0};
    TensorInfo vectorTensor = {vectorShape, 1, 0};
    TensorGroupInfo dataGroup = {&dataTensor, 1};
    TensorGroupInfo vectorGroup = {&vectorTensor, 1};
    uint8_t* xDevice = nullptr;
    uint8_t* residualDevice = nullptr;
    uint8_t* gammaDevice = nullptr;
    uint8_t* biasDevice = nullptr;
    uint8_t* outputDevice = nullptr;
    aclrtStream stream = nullptr;
    int aclInitialized = 0;
    int deviceSelected = 0;
    int exitCode = 2;
    aclError syncStatus = ACL_SUCCESS;

    printf("CASE dtype=FP32 rows=%llu width=%llu blocks=1 warmup=0 repeats=0\n",
        static_cast<unsigned long long>(kRows),
        static_cast<unsigned long long>(kWidth));
    if (ReportAcl(aclInit(nullptr), "aclInit") != 0) {
        goto cleanup;
    }
    aclInitialized = 1;
    if (ReportAcl(aclrtSetDevice(0), "aclrtSetDevice(0)") != 0) {
        goto cleanup;
    }
    deviceSelected = 1;
    if (ReportAcl(aclrtCreateStream(&stream), "aclrtCreateStream") != 0) {
        goto cleanup;
    }

    for (uint64_t i = 0; i < kWidth; ++i) {
        xHost[i] = InputValue(i, 0);
        residualHost[i] = InputValue(i, 1);
        gammaHost[i] = InputValue(i, 2);
        biasHost[i] = InputValue(i, 3);
    }

#define ALLOC_DEVICE(pointer, size, label) \
    if (ReportAcl(aclrtMalloc(reinterpret_cast<void**>(&(pointer)), (size), \
            ACL_MEM_MALLOC_HUGE_FIRST), (label)) != 0) { \
        goto cleanup; \
    }
    ALLOC_DEVICE(xDevice, bytes, "aclrtMalloc(x)");
    ALLOC_DEVICE(residualDevice, bytes, "aclrtMalloc(residual)");
    ALLOC_DEVICE(gammaDevice, bytes, "aclrtMalloc(gamma)");
    ALLOC_DEVICE(biasDevice, bytes, "aclrtMalloc(bias)");
    ALLOC_DEVICE(outputDevice, bytes, "aclrtMalloc(output)");
#undef ALLOC_DEVICE

    if (ReportAcl(aclrtMemcpy(xDevice, bytes, xHost, bytes, ACL_MEMCPY_HOST_TO_DEVICE),
            "aclrtMemcpy(x,H2D)") != 0 ||
        ReportAcl(aclrtMemcpy(residualDevice, bytes, residualHost, bytes,
            ACL_MEMCPY_HOST_TO_DEVICE), "aclrtMemcpy(residual,H2D)") != 0 ||
        ReportAcl(aclrtMemcpy(gammaDevice, bytes, gammaHost, bytes,
            ACL_MEMCPY_HOST_TO_DEVICE), "aclrtMemcpy(gamma,H2D)") != 0 ||
        ReportAcl(aclrtMemcpy(biasDevice, bytes, biasHost, bytes,
            ACL_MEMCPY_HOST_TO_DEVICE), "aclrtMemcpy(bias,H2D)") != 0) {
        goto cleanup;
    }

    printf("CALL run_kernel wrapper=V001-or-direct-parent availableCoreNum=1\n");
    run_kernel((GM_ADDR)xDevice, dataGroup,
        (GM_ADDR)residualDevice, dataGroup,
        (GM_ADDR)gammaDevice, vectorGroup,
        (GM_ADDR)biasDevice, vectorGroup,
        (GM_ADDR)outputDevice, dataGroup, 1, stream, kEpsilon);
    printf("SYNC_POINT run_kernel_internal_aclrtSynchronizeStream return_value=not_exposed_by_task_wrapper\n");
    syncStatus = aclrtSynchronizeStream(stream);
    printf("SYNC_POINT probe_aclrtSynchronizeStream code=%d\n", static_cast<int>(syncStatus));
    if (ReportAcl(syncStatus, "aclrtSynchronizeStream(after run_kernel)") != 0) {
        exitCode = 3;
        goto cleanup;
    }
    if (ReportAcl(aclrtMemcpy(outputHost, bytes, outputDevice, bytes,
            ACL_MEMCPY_DEVICE_TO_HOST), "aclrtMemcpy(output,D2H)") != 0) {
        exitCode = 3;
        goto cleanup;
    }

    {
        double sumSquares = 0.0;
        for (uint64_t i = 0; i < kWidth; ++i) {
            const double combined = static_cast<double>(xHost[i]) + residualHost[i];
            sumSquares += combined * combined;
        }
        const double inverseRms = 1.0 / sqrt(sumSquares / static_cast<double>(kWidth) + kEpsilon);
        uint64_t matched = 0;
        float maxAbsError = 0.0f;
        for (uint64_t i = 0; i < kWidth; ++i) {
            const float expected = static_cast<float>(
                (static_cast<double>(xHost[i]) + residualHost[i]) * inverseRms * gammaHost[i]
                + biasHost[i]);
            const float absError = fabsf(outputHost[i] - expected);
            if (absError > maxAbsError) {
                maxAbsError = absError;
            }
            if (absError <= 1.0e-5f + 1.0e-3f * fabsf(expected)) {
                ++matched;
            }
        }
        const double matchedRatio = static_cast<double>(matched) / static_cast<double>(kWidth);
        const int pass = matchedRatio >= 0.99 && maxAbsError <= 1.0e-2f;
        printf("CPU_REFERENCE matched=%llu/%llu matched_ratio=%.8f max_abs_error=%.9g status=%s\n",
            static_cast<unsigned long long>(matched),
            static_cast<unsigned long long>(kWidth), matchedRatio,
            static_cast<double>(maxAbsError), pass ? "PASS" : "FAIL");
        exitCode = pass ? 0 : 1;
    }

cleanup:
    if (xDevice != nullptr) ReportAcl(aclrtFree(xDevice), "aclrtFree(x)");
    if (residualDevice != nullptr) ReportAcl(aclrtFree(residualDevice), "aclrtFree(residual)");
    if (gammaDevice != nullptr) ReportAcl(aclrtFree(gammaDevice), "aclrtFree(gamma)");
    if (biasDevice != nullptr) ReportAcl(aclrtFree(biasDevice), "aclrtFree(bias)");
    if (outputDevice != nullptr) ReportAcl(aclrtFree(outputDevice), "aclrtFree(output)");
    if (stream != nullptr) ReportAcl(aclrtDestroyStream(stream), "aclrtDestroyStream");
    if (deviceSelected) ReportAcl(aclrtResetDevice(0), "aclrtResetDevice(0)");
    if (aclInitialized) ReportAcl(aclFinalize(), "aclFinalize");
    printf("EXIT_SUMMARY code=%d\n", exitCode);
    return exitCode;
}
