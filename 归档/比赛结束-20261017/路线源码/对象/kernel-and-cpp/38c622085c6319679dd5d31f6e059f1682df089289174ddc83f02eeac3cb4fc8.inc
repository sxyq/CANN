#include <acl/acl.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>

#include "local_types.h"
#include SELECTIVE_V001_SOURCE

#define ACL_CHECK(expr) do { \
    const aclError result = (expr); \
    if (result != ACL_SUCCESS) { \
        std::fprintf(stderr, "ACL failure %s=%d at %s:%d\n", #expr, result, __FILE__, __LINE__); \
        return 1; \
    } \
} while (0)

namespace {
constexpr float kEpsilon = 1.0e-5f;
constexpr double kAtol = 1.0 / 65536.0;
constexpr double kRtol = 1.0 / 1024.0;
constexpr double kMaxAbsLimit = 1.0e-2;

float InputValue(int64_t index, int multiplier, int offset)
{
    return static_cast<float>((index * multiplier + offset) % 997) / 498.0f - 1.0f;
}

bool SelectsDonor(int64_t rows, int64_t width, int dtype)
{
#if defined(SELECTIVE_V001_CANDIDATE)
    return dtype == 0 && ((rows == 8 && width == 16384) ||
                          (rows == 1 && (width == 32768 || width == 16384)));
#else
    (void)rows;
    (void)width;
    (void)dtype;
    return false;
#endif
}
}

int main(int argc, char** argv)
{
    if (argc != 7) {
        std::fprintf(stderr,
                     "usage: %s device_id rows width dtype output_path result_path\n",
                     argv[0]);
        return 2;
    }

    const int deviceId = std::atoi(argv[1]);
    const int64_t rows = std::strtoll(argv[2], nullptr, 10);
    const int64_t width = std::strtoll(argv[3], nullptr, 10);
    const int dtype = std::atoi(argv[4]);
    const char* outputPath = argv[5];
    const char* resultPath = argv[6];
    if (rows < 1 || width < 64 || width > 32768 || dtype != 0) {
        std::fprintf(stderr, "unsupported probe rows=%lld width=%lld dtype=%d\n",
                     static_cast<long long>(rows), static_cast<long long>(width), dtype);
        return 2;
    }

    const bool donor = SelectsDonor(rows, width, dtype);
    const int32_t tileWidth = width > 8192 ? 4096 : 0;
    const int32_t tileCount = tileWidth == 0 ? 0 :
        static_cast<int32_t>((width + tileWidth - 1) / tileWidth);
    const bool mergeRowRuns = donor && tileCount >= 4 && (width % 8) == 0;
    std::printf("variant=%s rank=2 rows=%lld width=%lld dtype=FP32 path=%s "
                "tileWidth=%d tileCount=%d mergeRowRuns=%s\n",
                SELECTIVE_V001_VARIANT, static_cast<long long>(rows),
                static_cast<long long>(width), donor ? "STORE_V003" : "R31B_V011",
                tileWidth, tileCount, mergeRowRuns ? "true" : "false");

    const size_t elements = static_cast<size_t>(rows * width);
    const size_t dataBytes = elements * sizeof(float);
    const size_t paramBytes = static_cast<size_t>(width) * sizeof(float);
    int64_t availableCoreNum = 0;
    aclrtStream stream = nullptr;
    ACL_CHECK(aclInit(nullptr));
    ACL_CHECK(aclrtSetDevice(deviceId));
    ACL_CHECK(aclrtGetDeviceInfo(deviceId, ACL_DEV_ATTR_VECTOR_CORE_NUM, &availableCoreNum));
    ACL_CHECK(aclrtCreateStream(&stream));

    int64_t xyShape[] = {rows, width};
    int64_t paramShape[] = {width};
    TensorInfo xTensor[] = {{xyShape, 2, dtype}};
    TensorInfo rTensor[] = {{xyShape, 2, dtype}};
    TensorInfo gTensor[] = {{paramShape, 1, dtype}};
    TensorInfo bTensor[] = {{paramShape, 1, dtype}};
    TensorInfo oTensor[] = {{xyShape, 2, dtype}};
    TensorGroupInfo infoX = {xTensor, 1}, infoR = {rTensor, 1};
    TensorGroupInfo infoG = {gTensor, 1}, infoB = {bTensor, 1}, infoO = {oTensor, 1};

    std::vector<float> hostX(elements), hostR(elements), hostG(width),
                       hostB(width), hostOut(elements);
    void *deviceX = nullptr, *deviceR = nullptr, *deviceG = nullptr,
         *deviceB = nullptr, *deviceOut = nullptr;
    ACL_CHECK(aclrtMalloc(&deviceX, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(&deviceR, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(&deviceG, paramBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(&deviceB, paramBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(&deviceOut, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMemset(deviceOut, dataBytes, 0, dataBytes));

    for (size_t i = 0; i < elements; ++i) {
        hostX[i] = InputValue(static_cast<int64_t>(i), 37, 11);
        hostR[i] = InputValue(static_cast<int64_t>(i), 17, 3);
    }
    for (int64_t i = 0; i < width; ++i) {
        hostG[static_cast<size_t>(i)] =
            0.75f + static_cast<float>((i * 13) % 100) / 200.0f;
        hostB[static_cast<size_t>(i)] =
            static_cast<float>((i * 7) % 100) / 400.0f - 0.125f;
    }
    ACL_CHECK(aclrtMemcpy(deviceX, dataBytes, hostX.data(), dataBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceR, dataBytes, hostR.data(), dataBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceG, paramBytes, hostG.data(), paramBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceB, paramBytes, hostB.data(), paramBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE));

    run_kernel((GM_ADDR)deviceX, infoX, (GM_ADDR)deviceR, infoR,
               (GM_ADDR)deviceG, infoG, (GM_ADDR)deviceB, infoB,
               (GM_ADDR)deviceOut, infoO, availableCoreNum, stream, kEpsilon);
    ACL_CHECK(aclrtSynchronizeStream(stream));
    ACL_CHECK(aclrtMemcpy(hostOut.data(), dataBytes, deviceOut, dataBytes,
                         ACL_MEMCPY_DEVICE_TO_HOST));

    double maxAbs = 0.0;
    size_t bad = 0;
    for (int64_t row = 0; row < rows; ++row) {
        double sum = 0.0;
        for (int64_t col = 0; col < width; ++col) {
            const size_t index = static_cast<size_t>(row * width + col);
            const double value = static_cast<double>(hostX[index]) + hostR[index];
            sum += value * value;
        }
        const double invRms = 1.0 / std::sqrt(sum / static_cast<double>(width) + kEpsilon);
        for (int64_t col = 0; col < width; ++col) {
            const size_t index = static_cast<size_t>(row * width + col);
            const double value = static_cast<double>(hostX[index]) + hostR[index];
            const double expected = value * invRms * hostG[static_cast<size_t>(col)] +
                                    hostB[static_cast<size_t>(col)];
            const double error = std::fabs(static_cast<double>(hostOut[index]) - expected);
            maxAbs = std::max(maxAbs, error);
            if (!std::isfinite(hostOut[index]) || error > kAtol + kRtol * std::fabs(expected)) {
                ++bad;
            }
        }
    }

    std::ofstream output(outputPath, std::ios::binary);
    output.write(reinterpret_cast<const char*>(hostOut.data()), dataBytes);
    output.close();
    if (!output.good()) {
        std::fprintf(stderr, "cannot write %s\n", outputPath);
        return 1;
    }
    const double matchedRatio = 1.0 - static_cast<double>(bad) / elements;
    const bool passed = matchedRatio >= 0.99 && maxAbs <= kMaxAbsLimit;
    std::ofstream result(resultPath);
    result << "variant\tdevice\trank\trows\twidth\tdtype\tmax_abs\tbad\tmatched_ratio\n"
           << SELECTIVE_V001_VARIANT << '\t' << deviceId << "\t2\t" << rows << '\t'
           << width << "\tFP32\t" << maxAbs << '\t' << bad << '\t' << matchedRatio << '\n';
    result.close();
    if (!result.good()) {
        std::fprintf(stderr, "cannot write %s\n", resultPath);
        return 1;
    }

    std::printf("golden_max_abs=%.12g golden_bad=%zu matched_ratio=%.9f "
                "max_abs_limit=%.9g status=%s\n",
                maxAbs, bad, matchedRatio, kMaxAbsLimit, passed ? "PASS" : "FAIL");

    ACL_CHECK(aclrtFree(deviceX)); ACL_CHECK(aclrtFree(deviceR));
    ACL_CHECK(aclrtFree(deviceG)); ACL_CHECK(aclrtFree(deviceB));
    ACL_CHECK(aclrtFree(deviceOut));
    ACL_CHECK(aclrtDestroyStream(stream));
    ACL_CHECK(aclFinalize());
    return passed ? 0 : 3;
}
