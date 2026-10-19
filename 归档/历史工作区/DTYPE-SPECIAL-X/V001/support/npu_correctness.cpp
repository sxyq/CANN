#include <acl/acl.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
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
    const int device = argc == 2 ? std::atoi(argv[1]) : 4;
    const std::vector<int64_t> widths = {
        1, 7, 8, 9, 63, 64, 65, 127, 128, 129, 1024, 4096, 8192
    };
    const std::vector<std::vector<int64_t>> prefixes = {
        {12}, {3, 4}, {1, 2, 6}
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

    const float epsilon = 1.0e-5f;
    size_t failures = 0;
    float overallMaxAbs = 0.0f;
    size_t caseCount = 0;
    for (size_t rank = 0; rank < prefixes.size(); ++rank) {
        for (int64_t width : widths) {
            const int64_t rows = 12;
            const size_t elements = static_cast<size_t>(rows * width);
            const size_t dataBytes = elements * sizeof(float);
            const size_t paramBytes = static_cast<size_t>(width) * sizeof(float);
            CheckAcl(aclrtMemcpy(dx, dataBytes, x.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy x");
            CheckAcl(aclrtMemcpy(dr, dataBytes, residual.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy residual");
            CheckAcl(aclrtMemcpy(dg, paramBytes, gamma.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma");
            CheckAcl(aclrtMemcpy(db, paramBytes, bias.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy bias");

            std::vector<int64_t> shape = prefixes[rank];
            shape.push_back(width);
            const TensorInfo dataInfo{shape.data(), static_cast<int64_t>(shape.size()), 0};
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
            ++caseCount;
            std::printf("rank=%zu width=%lld device=%d correctness=%s max_abs=%.8g\n",
                        rank + 2, static_cast<long long>(width), device,
                        caseFailures == 0 ? "PASS" : "FAIL", maxAbs);
        }
    }

    CheckAcl(aclrtFree(dout), "free output");
    CheckAcl(aclrtFree(db), "free bias");
    CheckAcl(aclrtFree(dg), "free gamma");
    CheckAcl(aclrtFree(dr), "free residual");
    CheckAcl(aclrtFree(dx), "free x");
    CheckAcl(aclrtDestroyStream(stream), "destroy stream");
    CheckAcl(aclrtResetDevice(device), "aclrtResetDevice");
    CheckAcl(aclFinalize(), "aclFinalize");
    std::printf("cases=%zu failures=%zu max_abs=%.8g correctness=%s timing=NOT_RUN\n",
                caseCount, failures, overallMaxAbs, failures == 0 ? "PASS" : "FAIL");
    return failures == 0 ? 0 : 1;
}
