#include <acl/acl.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstdio>
#include <fstream>
#include <array>

#include "local_types.h"
#include MIX_A_SUBMISSION

#define ACL_CHECK(expr) do { \
    const aclError result = (expr); \
    if (result != ACL_SUCCESS) { \
        std::fprintf(stderr, "ACL failure %s=%d at %s:%d\n", #expr, result, __FILE__, __LINE__); \
        return 1; \
    } \
} while (0)

namespace {
constexpr int64_t kRows = 1;
constexpr int64_t kWidth = 64;
constexpr int kWarmups = 2;
constexpr int kRepeats = 9;
constexpr float kEpsilon = 1.0e-5f;

float InputValue(int64_t index, int multiplier, int offset)
{
    return static_cast<float>((index * multiplier + offset) % 997) / 498.0f - 1.0f;
}

bool WriteFloats(const char* path, const float* values, size_t count)
{
    std::ofstream out(path, std::ios::binary);
    out.write(reinterpret_cast<const char*>(values), count * sizeof(float));
    return out.good();
}
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        std::fprintf(stderr, "usage: %s device_id result_prefix\n", argv[0]);
        return 2;
    }
    const int deviceId = std::atoi(argv[1]);
    const char* prefix = argv[2];
    const size_t elements = static_cast<size_t>(kRows * kWidth);
    const size_t dataBytes = elements * sizeof(float);
    const size_t paramBytes = static_cast<size_t>(kWidth) * sizeof(float);
    const float epsilon = kEpsilon;
    int64_t availableCoreNum = 0;
    aclrtStream stream = nullptr;

    ACL_CHECK(aclInit(nullptr));
    ACL_CHECK(aclrtSetDevice(deviceId));
    ACL_CHECK(aclrtGetDeviceInfo(deviceId, ACL_DEV_ATTR_VECTOR_CORE_NUM, &availableCoreNum));
    ACL_CHECK(aclrtCreateStream(&stream));

    int64_t xyShape[] = {kRows, kWidth};
    int64_t paramShape[] = {kWidth};
    TensorInfo xTensor[] = {{xyShape, 2, 0}};
    TensorInfo rTensor[] = {{xyShape, 2, 0}};
    TensorInfo gTensor[] = {{paramShape, 1, 0}};
    TensorInfo bTensor[] = {{paramShape, 1, 0}};
    TensorInfo oTensor[] = {{xyShape, 2, 0}};
    TensorGroupInfo infoX = {xTensor, 1}, infoR = {rTensor, 1};
    TensorGroupInfo infoG = {gTensor, 1}, infoB = {bTensor, 1}, infoO = {oTensor, 1};

    float *hostX = nullptr, *hostR = nullptr, *hostG = nullptr, *hostB = nullptr, *hostOut = nullptr;
    float *deviceX = nullptr, *deviceR = nullptr, *deviceG = nullptr, *deviceB = nullptr, *deviceOut = nullptr;
    ACL_CHECK(aclrtMallocHost(reinterpret_cast<void**>(&hostX), dataBytes));
    ACL_CHECK(aclrtMallocHost(reinterpret_cast<void**>(&hostR), dataBytes));
    ACL_CHECK(aclrtMallocHost(reinterpret_cast<void**>(&hostG), paramBytes));
    ACL_CHECK(aclrtMallocHost(reinterpret_cast<void**>(&hostB), paramBytes));
    ACL_CHECK(aclrtMallocHost(reinterpret_cast<void**>(&hostOut), dataBytes));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceX), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceR), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceG), paramBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceB), paramBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceOut), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));

    for (int64_t i = 0; i < kRows * kWidth; ++i) {
        hostX[i] = InputValue(i, 37, 11);
        hostR[i] = InputValue(i, 17, 3);
    }
    for (int64_t i = 0; i < kWidth; ++i) {
        hostG[i] = 0.75f + static_cast<float>((i * 13) % 100) / 200.0f;
        hostB[i] = static_cast<float>((i * 7) % 100) / 400.0f - 0.125f;
    }
    ACL_CHECK(aclrtMemcpy(deviceX, dataBytes, hostX, dataBytes, ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceR, dataBytes, hostR, dataBytes, ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceG, paramBytes, hostG, paramBytes, ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceB, paramBytes, hostB, paramBytes, ACL_MEMCPY_HOST_TO_DEVICE));

    auto launch = [&]() {
        run_kernel((GM_ADDR)deviceX, infoX,
                   (GM_ADDR)deviceR, infoR,
                   (GM_ADDR)deviceG, infoG,
                   (GM_ADDR)deviceB, infoB,
                   (GM_ADDR)deviceOut, infoO,
                   availableCoreNum, stream, epsilon);
    };
    for (int i = 0; i < kWarmups; ++i) {
        launch();
        ACL_CHECK(aclrtSynchronizeStream(stream));
    }
    std::array<double, kRepeats> samples{};
    for (int i = 0; i < kRepeats; ++i) {
        const auto start = std::chrono::steady_clock::now();
        launch();
        ACL_CHECK(aclrtSynchronizeStream(stream));
        const auto stop = std::chrono::steady_clock::now();
        samples[static_cast<size_t>(i)] =
            std::chrono::duration<double, std::micro>(stop - start).count();
    }
    std::sort(samples.begin(), samples.end());
    ACL_CHECK(aclrtMemcpy(hostOut, dataBytes, deviceOut, dataBytes, ACL_MEMCPY_DEVICE_TO_HOST));

    float maxAbs = 0.0f;
    int bad = 0;
    for (int64_t row = 0; row < kRows; ++row) {
        float sum = 0.0f;
        for (int64_t col = 0; col < kWidth; ++col) {
            const int64_t i = row * kWidth + col;
            const float u = hostX[i] + hostR[i];
            sum += u * u;
        }
        const float invRms = 1.0f / std::sqrt(sum / static_cast<float>(kWidth) + epsilon);
        for (int64_t col = 0; col < kWidth; ++col) {
            const int64_t i = row * kWidth + col;
            const float u = hostX[i] + hostR[i];
            const float expected = u * invRms * hostG[col] + hostB[col];
            const float error = std::fabs(hostOut[i] - expected);
            maxAbs = std::max(maxAbs, error);
            if (error > 1.0e-4f + 1.0e-4f * std::fabs(expected)) {
                ++bad;
            }
        }
    }
    char outputPath[512];
    char timingPath[512];
    std::snprintf(outputPath, sizeof(outputPath), "%s-output.bin", prefix);
    std::snprintf(timingPath, sizeof(timingPath), "%s.tsv", prefix);
    if (!WriteFloats(outputPath, hostOut, elements)) {
        std::fprintf(stderr, "cannot write %s\n", outputPath);
        return 1;
    }
    std::ofstream timing(timingPath);
    timing << "rows\twidth\twarmups\trepeats\tmedian_us\tmax_abs\tbad\n"
           << kRows << '\t' << kWidth << '\t' << kWarmups << '\t' << kRepeats << '\t'
           << samples[samples.size() / 2] << '\t' << maxAbs << '\t' << bad << '\n';
    std::printf("device=%d rows=%lld width=%lld median_us=%.3f max_abs=%.7g bad=%d\n",
                deviceId, static_cast<long long>(kRows), static_cast<long long>(kWidth),
                samples[samples.size() / 2], maxAbs, bad);

    ACL_CHECK(aclrtFree(deviceX)); ACL_CHECK(aclrtFree(deviceR));
    ACL_CHECK(aclrtFree(deviceG)); ACL_CHECK(aclrtFree(deviceB)); ACL_CHECK(aclrtFree(deviceOut));
    ACL_CHECK(aclrtFreeHost(hostX)); ACL_CHECK(aclrtFreeHost(hostR));
    ACL_CHECK(aclrtFreeHost(hostG)); ACL_CHECK(aclrtFreeHost(hostB)); ACL_CHECK(aclrtFreeHost(hostOut));
    ACL_CHECK(aclrtDestroyStream(stream));
    ACL_CHECK(aclFinalize());
    return bad == 0 ? 0 : 3;
}
