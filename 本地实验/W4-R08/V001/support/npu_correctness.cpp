#include <acl/acl.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

struct TensorInfo {
    const int64_t* shape;
    int64_t numDims;
    int32_t dtype;
};

struct TensorGroupInfo {
    const TensorInfo* tensors;
    int64_t numTensors;
};

#include "submission.asc"

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
    const int device = argc == 2 ? std::atoi(argv[1]) : 7;
    const float epsilon = 1.0e-5f;
    // Shape set is taken from retained evidence:
    //   本地实验/UB-LIVENESS-X/V001/local-result.json correctness.shapes
    //   本地实验/ARCHIVED ROWGROUP probes (rows=3/9 width=65/257)
    //   本地实验/DTYPE-SPECIAL-X/V001 measured matrix (rows=12, width<=8192)
    const std::vector<std::vector<int64_t>> cases = {
        {8, 4096},    // ProcessNarrowMidOverlap is bypassed; regression guard
        {3, 65},      // generic tiny path, cross-row prefetch inactive
        {9, 257},     // mid-width non-aligned
        {12, 129},    // lower bound of narrow-mid range
        {12, 1024},   // ProcessNarrowMidOverlap, aligned
        {12, 1025},   // ProcessNarrowMidOverlap, unaligned tail
        {12, 4095},   // ProcessNarrowMidOverlap, last aligned width
        {12, 4096},   // boundary: ProcessNarrowMidOverlap upper bound
        {12, 4097},   // boundary just above: kSmallFp32Batched bucket
        {12, 8192},   // boundary: ProcessFp32FullRowOutputPipelined
        {5, 2048},    // odd row count, mid path
        {2, 4096},    // minimum localRows>1, mid path
    };
    const int64_t maxRows = 12;
    const int64_t maxWidth = 8192;
    const size_t maxElements = static_cast<size_t>(maxRows * maxWidth);
    std::vector<float> x(maxElements), residual(maxElements), output(maxElements);
    std::vector<float> gamma(maxWidth), bias(maxWidth), expected(maxElements);

    for (size_t i = 0; i < maxElements; ++i) {
        x[i] = 0.19f * std::sin(static_cast<float>((i * 17) % 997) * 0.013f);
        residual[i] = 0.11f * std::cos(static_cast<float>((i * 29) % 991) * 0.017f);
    }
    for (int64_t col = 0; col < maxWidth; ++col) {
        gamma[col] = 0.85f + static_cast<float>(col % 23) * 0.002f;
        bias[col] = -0.025f + static_cast<float>(col % 19) * 0.001f;
    }

    CheckAcl(aclInit(nullptr), "aclInit");
    CheckAcl(aclrtSetDevice(device), "aclrtSetDevice");
    aclrtStream stream = nullptr;
    CheckAcl(aclrtCreateStream(&stream), "aclrtCreateStream");

    void* dx = nullptr;
    void* dr = nullptr;
    void* dg = nullptr;
    void* db = nullptr;
    void* dout = nullptr;
    CheckAcl(aclrtMalloc(&dx, maxElements * sizeof(float), ACL_MEM_MALLOC_HUGE_FIRST), "malloc x");
    CheckAcl(aclrtMalloc(&dr, maxElements * sizeof(float), ACL_MEM_MALLOC_HUGE_FIRST), "malloc residual");
    CheckAcl(aclrtMalloc(&dg, maxWidth * sizeof(float), ACL_MEM_MALLOC_HUGE_FIRST), "malloc gamma");
    CheckAcl(aclrtMalloc(&db, maxWidth * sizeof(float), ACL_MEM_MALLOC_HUGE_FIRST), "malloc bias");
    CheckAcl(aclrtMalloc(&dout, maxElements * sizeof(float), ACL_MEM_MALLOC_HUGE_FIRST), "malloc output");

    size_t failures = 0;
    float overallMaxAbs = 0.0f;
    for (size_t caseIndex = 0; caseIndex < cases.size(); ++caseIndex) {
        const int64_t rows = cases[caseIndex][0];
        const int64_t width = cases[caseIndex][1];
        const size_t elements = static_cast<size_t>(rows * width);
        const size_t dataBytes = elements * sizeof(float);
        const size_t paramBytes = static_cast<size_t>(width) * sizeof(float);
        CheckAcl(aclrtMemcpy(dx, dataBytes, x.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy x");
        CheckAcl(aclrtMemcpy(dr, dataBytes, residual.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy residual");
        CheckAcl(aclrtMemcpy(dg, paramBytes, gamma.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma");
        CheckAcl(aclrtMemcpy(db, paramBytes, bias.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy bias");

        const int64_t shape[2] = {rows, width};
        const TensorInfo dataInfo{shape, 2, 0};
        const TensorInfo paramInfo{&width, 1, 0};
        const TensorGroupInfo dataGroup{&dataInfo, 1};
        const TensorGroupInfo paramGroup{&paramInfo, 1};
        run_kernel((GM_ADDR)dx, dataGroup,
                   (GM_ADDR)dr, dataGroup,
                   (GM_ADDR)dg, paramGroup,
                   (GM_ADDR)db, paramGroup,
                   (GM_ADDR)dout, dataGroup,
                   8, stream, epsilon);
        CheckAcl(aclrtSynchronizeStream(stream), "kernel synchronize");
        CheckAcl(aclrtMemcpy(output.data(), dataBytes, dout, dataBytes,
                             ACL_MEMCPY_DEVICE_TO_HOST), "copy output");

        float maxAbs = 0.0f;
        size_t caseFailures = 0;
        for (int64_t row = 0; row < rows; ++row) {
            float squareSum = 0.0f;
            for (int64_t col = 0; col < width; ++col) {
                const size_t index = static_cast<size_t>(row * width + col);
                expected[index] = x[index] + residual[index];
                squareSum += expected[index] * expected[index];
            }
            const float invRms = 1.0f / std::sqrt(
                squareSum / static_cast<float>(width) + epsilon);
            for (int64_t col = 0; col < width; ++col) {
                const size_t index = static_cast<size_t>(row * width + col);
                const float want = expected[index] * invRms * gamma[col] + bias[col];
                const float error = std::fabs(output[index] - want);
                maxAbs = std::max(maxAbs, error);
                if (error > 1.0e-4f + 1.0e-4f * std::fabs(want)) {
                    ++failures;
                    ++caseFailures;
                }
            }
        }
        overallMaxAbs = std::max(overallMaxAbs, maxAbs);
        std::printf("case=%zu rows=%lld width=%lld device=%d correctness=%s max_abs=%.8g failures=%zu\n",
                    caseIndex, static_cast<long long>(rows), static_cast<long long>(width), device,
                    caseFailures == 0 ? "PASS" : "FAIL", maxAbs, caseFailures);
        std::fflush(stdout);
    }

    CheckAcl(aclrtFree(dout), "free output");
    CheckAcl(aclrtFree(db), "free bias");
    CheckAcl(aclrtFree(dg), "free gamma");
    CheckAcl(aclrtFree(dr), "free residual");
    CheckAcl(aclrtFree(dx), "free x");
    CheckAcl(aclrtDestroyStream(stream), "destroy stream");
    CheckAcl(aclrtResetDevice(device), "reset device");
    CheckAcl(aclFinalize(), "aclFinalize");
    std::printf("cases=%zu failures=%zu max_abs=%.8g correctness=%s timing=NOT_RUN\n",
                cases.size(), failures, overallMaxAbs, failures == 0 ? "PASS" : "FAIL");
    return failures == 0 ? 0 : 1;
}
