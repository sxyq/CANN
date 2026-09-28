#include <acl/acl.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

#ifndef TENSOR_GROUP_INFO_DEFINED
#define TENSOR_GROUP_INFO_DEFINED
struct TensorInfo { const int64_t* shape; int64_t numDims; int32_t dtype; };
struct TensorGroupInfo { const TensorInfo* tensors; int64_t numTensors; };
#endif

#if defined(R31B_PROBE_PARENT)
#include "R31B-V011-LP-ROW-PIPELINE_kernel.asc"
#else
#include "R31B-V016-WIDE-TILE-SEED_kernel.asc"
#endif

static void CheckAcl(aclError status, const char* operation)
{
    if (status != ACL_SUCCESS) {
        std::fprintf(stderr, "%s failed: %d %s\n", operation, status,
                     aclGetRecentErrMsg() ? aclGetRecentErrMsg() : "");
        std::exit(2);
    }
}

int main(int argc, char** argv)
{
    if (argc != 4) {
        std::fprintf(stderr, "usage: probe_fp32 rows width device\n");
        return 2;
    }
    const int64_t rows = std::atoll(argv[1]);
    const int64_t width = std::atoll(argv[2]);
    const int device = std::atoi(argv[3]);
    if (rows <= 0 || width < 8192) {
        std::fprintf(stderr, "invalid probe arguments\n");
        return 2;
    }

    CheckAcl(aclInit(nullptr), "aclInit");
    CheckAcl(aclrtSetDevice(device), "aclrtSetDevice");
    aclrtStream stream = nullptr;
    CheckAcl(aclrtCreateStream(&stream), "aclrtCreateStream");

    const size_t elementCount = static_cast<size_t>(rows * width);
    const size_t dataBytes = elementCount * sizeof(float);
    const size_t paramBytes = static_cast<size_t>(width) * sizeof(float);
    std::vector<float> x(elementCount), residual(elementCount), gamma(width), bias(width);
    std::vector<float> output(elementCount), expected(elementCount);
    for (size_t i = 0; i < elementCount; ++i) {
        x[i] = 0.19f * std::sin(static_cast<float>((i * 17) % 997) * 0.013f);
        residual[i] = 0.11f * std::cos(static_cast<float>((i * 29) % 991) * 0.017f);
    }
    for (int64_t col = 0; col < width; ++col) {
        gamma[col] = 0.85f + static_cast<float>(col % 23) * 0.002f;
        bias[col] = -0.025f + static_cast<float>(col % 19) * 0.001f;
    }

    void* dx = nullptr;
    void* dr = nullptr;
    void* dg = nullptr;
    void* db = nullptr;
    void* dout = nullptr;
    CheckAcl(aclrtMalloc(&dx, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc x");
    CheckAcl(aclrtMalloc(&dr, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc residual");
    CheckAcl(aclrtMalloc(&dg, paramBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc gamma");
    CheckAcl(aclrtMalloc(&db, paramBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc bias");
    CheckAcl(aclrtMalloc(&dout, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc output");
    CheckAcl(aclrtMemcpy(dx, dataBytes, x.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy x");
    CheckAcl(aclrtMemcpy(dr, dataBytes, residual.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy residual");
    CheckAcl(aclrtMemcpy(dg, paramBytes, gamma.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma");
    CheckAcl(aclrtMemcpy(db, paramBytes, bias.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy bias");

    const int64_t xShape[2] = {rows, width};
    const int64_t paramShape[1] = {width};
    const TensorInfo dataInfo{xShape, 2, 0};
    const TensorInfo paramInfo{paramShape, 1, 0};
    const TensorGroupInfo dataGroup{&dataInfo, 1};
    const TensorGroupInfo paramGroup{&paramInfo, 1};
    run_kernel((GM_ADDR)dx, dataGroup, (GM_ADDR)dr, dataGroup,
               (GM_ADDR)dg, paramGroup, (GM_ADDR)db, paramGroup,
               (GM_ADDR)dout, dataGroup, 8, stream, 1.0e-5f);
    CheckAcl(aclrtSynchronizeStream(stream), "kernel synchronize");
    CheckAcl(aclrtMemcpy(output.data(), dataBytes, dout, dataBytes, ACL_MEMCPY_DEVICE_TO_HOST),
             "copy output");

    float maxAbs = 0.0f;
    float maxRel = 0.0f;
    size_t failures = 0;
    for (int64_t row = 0; row < rows; ++row) {
        float squareSum = 0.0f;
        for (int64_t col = 0; col < width; ++col) {
            const size_t index = static_cast<size_t>(row * width + col);
            expected[index] = x[index] + residual[index];
            squareSum += expected[index] * expected[index];
        }
        const float invRms = 1.0f / std::sqrt(squareSum / static_cast<float>(width) + 1.0e-5f);
        for (int64_t col = 0; col < width; ++col) {
            const size_t index = static_cast<size_t>(row * width + col);
            const float want = expected[index] * invRms * gamma[col] + bias[col];
            const float absError = std::fabs(output[index] - want);
            const float relError = absError / std::max(std::fabs(want), 1.0e-6f);
            maxAbs = std::max(maxAbs, absError);
            maxRel = std::max(maxRel, relError);
            if (absError > 1.0e-4f + 1.0e-4f * std::fabs(want)) {
                ++failures;
            }
        }
    }
    std::printf("dtype=fp32 rows=%lld width=%lld device=%d correctness=%s failures=%zu max_abs=%.8g max_rel=%.8g\n",
                static_cast<long long>(rows), static_cast<long long>(width), device,
                failures == 0 ? "PASS" : "FAIL", failures, maxAbs, maxRel);

    CheckAcl(aclrtFree(dout), "free output");
    CheckAcl(aclrtFree(db), "free bias");
    CheckAcl(aclrtFree(dg), "free gamma");
    CheckAcl(aclrtFree(dr), "free residual");
    CheckAcl(aclrtFree(dx), "free x");
    CheckAcl(aclrtDestroyStream(stream), "destroy stream");
    CheckAcl(aclrtResetDevice(device), "reset device");
    CheckAcl(aclFinalize(), "aclFinalize");
    return failures == 0 ? 0 : 1;
}
