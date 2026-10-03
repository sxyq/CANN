#include <acl/acl.h>
#include <algorithm>
#include <cerrno>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fcntl.h>
#include <string>
#include <unistd.h>
#include <vector>

#include "local_types.h"
#include REPEATABILITY_V001_SOURCE

#define ACL_CHECK(expr) do { \
    const aclError result = (expr); \
    if (result != ACL_SUCCESS) { \
        std::fprintf(stderr, "ACL failure %s=%d at %s:%d\n", #expr, result, __FILE__, __LINE__); \
        return 1; \
    } \
} while (0)

namespace {
constexpr int64_t kRows = 2;
constexpr int64_t kWidth = 16384;
constexpr int kDtype = 0;
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
#if defined(REPEATABILITY_V001_CANDIDATE)
    return dtype == 0 && ((rows == 8 && width == 16384) ||
                          (rows == 1 && (width == 32768 || width == 16384)));
#else
    (void)rows;
    (void)width;
    (void)dtype;
    return false;
#endif
}

bool WriteNewFile(const std::string& path, const void* data, size_t bytes)
{
    const int fd = ::open(path.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd < 0) {
        std::fprintf(stderr, "cannot create new artifact %s: %s\n",
                     path.c_str(), std::strerror(errno));
        return false;
    }

    const auto* current = static_cast<const unsigned char*>(data);
    size_t offset = 0;
    while (offset < bytes) {
        const ssize_t count = ::write(fd, current + offset, bytes - offset);
        if (count < 0 && errno == EINTR) {
            continue;
        }
        if (count <= 0) {
            std::fprintf(stderr, "cannot write artifact %s: %s\n",
                         path.c_str(), std::strerror(errno));
            ::close(fd);
            return false;
        }
        offset += static_cast<size_t>(count);
    }
    if (::close(fd) != 0) {
        std::fprintf(stderr, "cannot close artifact %s: %s\n",
                     path.c_str(), std::strerror(errno));
        return false;
    }
    return true;
}

bool SaveFloats(const std::string& path, const std::vector<float>& values)
{
    static_assert(sizeof(float) == 4, "diagnostic expects FP32 buffers");
    return WriteNewFile(path, values.data(), values.size() * sizeof(float));
}

struct GoldenMetrics {
    double maxAbs = 0.0;
    size_t bad = 0;
    double matchedRatio = 0.0;
    bool pass = false;
};

GoldenMetrics EvaluateGolden(int64_t rows, int64_t width,
                            const std::vector<float>& x,
                            const std::vector<float>& residual,
                            const std::vector<float>& gamma,
                            const std::vector<float>& bias,
                            const std::vector<float>& output)
{
    GoldenMetrics metrics;
    const size_t elements = static_cast<size_t>(rows * width);
    for (int64_t row = 0; row < rows; ++row) {
        double squareSum = 0.0;
        for (int64_t col = 0; col < width; ++col) {
            const size_t index = static_cast<size_t>(row * width + col);
            const double value = static_cast<double>(x[index]) + residual[index];
            squareSum += value * value;
        }
        const double invRms = 1.0 / std::sqrt(
            squareSum / static_cast<double>(width) + static_cast<double>(kEpsilon));
        for (int64_t col = 0; col < width; ++col) {
            const size_t index = static_cast<size_t>(row * width + col);
            const double value = static_cast<double>(x[index]) + residual[index];
            const double expected = value * invRms * gamma[static_cast<size_t>(col)] +
                                    bias[static_cast<size_t>(col)];
            const double error = std::fabs(static_cast<double>(output[index]) - expected);
            metrics.maxAbs = std::max(metrics.maxAbs, error);
            if (!std::isfinite(output[index]) ||
                error > kAtol + kRtol * std::fabs(expected)) {
                ++metrics.bad;
            }
        }
    }
    metrics.matchedRatio = 1.0 - static_cast<double>(metrics.bad) / elements;
    metrics.pass = metrics.matchedRatio >= 0.99 && metrics.maxAbs <= kMaxAbsLimit;
    return metrics;
}

bool EqualBytes(const std::vector<float>& left, const std::vector<float>& right)
{
    const size_t bytes = left.size() * sizeof(float);
    return left.size() == right.size() && std::memcmp(left.data(), right.data(), bytes) == 0;
}
}

int main(int argc, char** argv)
{
    if (argc != 3) {
        std::fprintf(stderr, "usage: %s device_id new_invocation_dir\n", argv[0]);
        return 2;
    }
    const int deviceId = std::atoi(argv[1]);
    const std::string outputDir(argv[2]);
    if (outputDir.empty()) {
        std::fprintf(stderr, "empty invocation directory\n");
        return 2;
    }

    const bool donor = SelectsDonor(kRows, kWidth, kDtype);
    const int32_t tileWidth = kWidth > 8192 ? 4096 : 0;
    const int32_t tileCount = tileWidth == 0 ? 0 :
        static_cast<int32_t>((kWidth + tileWidth - 1) / tileWidth);
    const bool mergeRowRuns = donor && tileCount >= 4 && (kWidth % 8) == 0;
    std::printf("variant=%s rank=2 rows=%lld width=%lld dtype=FP32 path=%s "
                "tileWidth=%d tileCount=%d mergeRowRuns=%s\n",
                REPEATABILITY_V001_VARIANT, static_cast<long long>(kRows),
                static_cast<long long>(kWidth), donor ? "STORE_V003" : "R31B_V011",
                tileWidth, tileCount,
                mergeRowRuns ? "true" : "false");

    const size_t elements = static_cast<size_t>(kRows * kWidth);
    const size_t dataBytes = elements * sizeof(float);
    const size_t paramBytes = static_cast<size_t>(kWidth) * sizeof(float);
    std::vector<float> hostX(elements), hostResidual(elements), hostGamma(kWidth),
                       hostBias(kWidth), deviceXReadback(elements),
                       deviceResidualReadback(elements), deviceGammaReadback(kWidth),
                       deviceBiasReadback(kWidth), outputFirst(elements), outputSecond(elements);

    for (size_t i = 0; i < elements; ++i) {
        hostX[i] = InputValue(static_cast<int64_t>(i), 37, 11);
        hostResidual[i] = InputValue(static_cast<int64_t>(i), 17, 3);
    }
    for (int64_t i = 0; i < kWidth; ++i) {
        hostGamma[static_cast<size_t>(i)] =
            0.75f + static_cast<float>((i * 13) % 100) / 200.0f;
        hostBias[static_cast<size_t>(i)] =
            static_cast<float>((i * 7) % 100) / 400.0f - 0.125f;
    }

    if (!SaveFloats(outputDir + "/host_pre_h2d_x.bin", hostX) ||
        !SaveFloats(outputDir + "/host_pre_h2d_residual.bin", hostResidual) ||
        !SaveFloats(outputDir + "/host_pre_h2d_gamma.bin", hostGamma) ||
        !SaveFloats(outputDir + "/host_pre_h2d_bias.bin", hostBias)) {
        return 1;
    }
    std::printf("host_pre_h2d_snapshots=written bytes_x=%zu bytes_residual=%zu "
                "bytes_gamma=%zu bytes_bias=%zu\n",
                dataBytes, dataBytes, paramBytes, paramBytes);

    int64_t availableCoreNum = 0;
    aclrtStream stream = nullptr;
    ACL_CHECK(aclInit(nullptr));
    ACL_CHECK(aclrtSetDevice(deviceId));
    ACL_CHECK(aclrtGetDeviceInfo(deviceId, ACL_DEV_ATTR_VECTOR_CORE_NUM,
                                 &availableCoreNum));
    ACL_CHECK(aclrtCreateStream(&stream));

    int64_t xyShape[] = {kRows, kWidth};
    int64_t paramShape[] = {kWidth};
    TensorInfo xTensor[] = {{xyShape, 2, kDtype}};
    TensorInfo residualTensor[] = {{xyShape, 2, kDtype}};
    TensorInfo gammaTensor[] = {{paramShape, 1, kDtype}};
    TensorInfo biasTensor[] = {{paramShape, 1, kDtype}};
    TensorInfo outputTensor[] = {{xyShape, 2, kDtype}};
    TensorGroupInfo infoX = {xTensor, 1}, infoResidual = {residualTensor, 1};
    TensorGroupInfo infoGamma = {gammaTensor, 1}, infoBias = {biasTensor, 1};
    TensorGroupInfo infoOutput = {outputTensor, 1};

    void *deviceX = nullptr, *deviceResidual = nullptr, *deviceGamma = nullptr,
         *deviceBias = nullptr, *deviceOutput = nullptr;
    ACL_CHECK(aclrtMalloc(&deviceX, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(&deviceResidual, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(&deviceGamma, paramBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(&deviceBias, paramBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(&deviceOutput, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMemset(deviceOutput, dataBytes, 0, dataBytes));

    ACL_CHECK(aclrtMemcpy(deviceX, dataBytes, hostX.data(), dataBytes,
                          ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceResidual, dataBytes, hostResidual.data(), dataBytes,
                          ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceGamma, paramBytes, hostGamma.data(), paramBytes,
                          ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceBias, paramBytes, hostBias.data(), paramBytes,
                          ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtSynchronizeStream(stream));

    ACL_CHECK(aclrtMemcpy(deviceXReadback.data(), dataBytes, deviceX, dataBytes,
                          ACL_MEMCPY_DEVICE_TO_HOST));
    ACL_CHECK(aclrtMemcpy(deviceResidualReadback.data(), dataBytes, deviceResidual,
                          dataBytes, ACL_MEMCPY_DEVICE_TO_HOST));
    ACL_CHECK(aclrtMemcpy(deviceGammaReadback.data(), paramBytes, deviceGamma,
                          paramBytes, ACL_MEMCPY_DEVICE_TO_HOST));
    ACL_CHECK(aclrtMemcpy(deviceBiasReadback.data(), paramBytes, deviceBias,
                          paramBytes, ACL_MEMCPY_DEVICE_TO_HOST));
    if (!SaveFloats(outputDir + "/device_after_h2d_x.bin", deviceXReadback) ||
        !SaveFloats(outputDir + "/device_after_h2d_residual.bin", deviceResidualReadback) ||
        !SaveFloats(outputDir + "/device_after_h2d_gamma.bin", deviceGammaReadback) ||
        !SaveFloats(outputDir + "/device_after_h2d_bias.bin", deviceBiasReadback)) {
        return 1;
    }
    const bool xCopyEqual = EqualBytes(hostX, deviceXReadback);
    const bool residualCopyEqual = EqualBytes(hostResidual, deviceResidualReadback);
    const bool gammaCopyEqual = EqualBytes(hostGamma, deviceGammaReadback);
    const bool biasCopyEqual = EqualBytes(hostBias, deviceBiasReadback);
    std::printf("h2d_readback_equal x=%s residual=%s gamma=%s bias=%s\n",
                xCopyEqual ? "yes" : "no", residualCopyEqual ? "yes" : "no",
                gammaCopyEqual ? "yes" : "no", biasCopyEqual ? "yes" : "no");

    run_kernel((GM_ADDR)deviceX, infoX, (GM_ADDR)deviceResidual, infoResidual,
               (GM_ADDR)deviceGamma, infoGamma, (GM_ADDR)deviceBias, infoBias,
               (GM_ADDR)deviceOutput, infoOutput, availableCoreNum, stream, kEpsilon);
    ACL_CHECK(aclrtSynchronizeStream(stream));
    ACL_CHECK(aclrtMemcpy(outputFirst.data(), dataBytes, deviceOutput, dataBytes,
                          ACL_MEMCPY_DEVICE_TO_HOST));
    ACL_CHECK(aclrtMemcpy(outputSecond.data(), dataBytes, deviceOutput, dataBytes,
                          ACL_MEMCPY_DEVICE_TO_HOST));
    if (!SaveFloats(outputDir + "/output_d2h_first.bin", outputFirst) ||
        !SaveFloats(outputDir + "/output_d2h_second.bin", outputSecond)) {
        return 1;
    }
    const bool outputCopiesEqual = EqualBytes(outputFirst, outputSecond);
    const GoldenMetrics firstMetrics = EvaluateGolden(
        kRows, kWidth, hostX, hostResidual, hostGamma, hostBias, outputFirst);
    const GoldenMetrics secondMetrics = EvaluateGolden(
        kRows, kWidth, hostX, hostResidual, hostGamma, hostBias, outputSecond);

    char metricsText[1024];
    const int metricsLength = std::snprintf(
        metricsText, sizeof(metricsText),
        "variant\trows\twidth\tdtype\th2d_x_equal\th2d_residual_equal\t"
        "h2d_gamma_equal\th2d_bias_equal\td2h_copies_equal\t"
        "first_max_abs\tfirst_bad\tfirst_matched_ratio\tfirst_golden\t"
        "second_max_abs\tsecond_bad\tsecond_matched_ratio\tsecond_golden\n"
        "%s\t%lld\t%lld\tFP32\t%s\t%s\t%s\t%s\t%s\t%.12g\t%zu\t%.9f\t%s\t"
        "%.12g\t%zu\t%.9f\t%s\n",
        REPEATABILITY_V001_VARIANT, static_cast<long long>(kRows),
        static_cast<long long>(kWidth), xCopyEqual ? "yes" : "no",
        residualCopyEqual ? "yes" : "no", gammaCopyEqual ? "yes" : "no",
        biasCopyEqual ? "yes" : "no", outputCopiesEqual ? "yes" : "no",
        firstMetrics.maxAbs, firstMetrics.bad, firstMetrics.matchedRatio,
        firstMetrics.pass ? "PASS" : "FAIL", secondMetrics.maxAbs,
        secondMetrics.bad, secondMetrics.matchedRatio,
        secondMetrics.pass ? "PASS" : "FAIL");
    if (metricsLength < 0 || static_cast<size_t>(metricsLength) >= sizeof(metricsText) ||
        !WriteNewFile(outputDir + "/metrics.tsv", metricsText,
                      static_cast<size_t>(metricsLength))) {
        return 1;
    }

    std::printf("d2h_copy_equal=%s\n", outputCopiesEqual ? "yes" : "no");
    std::printf("golden_first max_abs=%.12g bad=%zu matched_ratio=%.9f status=%s\n",
                firstMetrics.maxAbs, firstMetrics.bad, firstMetrics.matchedRatio,
                firstMetrics.pass ? "PASS" : "FAIL");
    std::printf("golden_second max_abs=%.12g bad=%zu matched_ratio=%.9f status=%s\n",
                secondMetrics.maxAbs, secondMetrics.bad, secondMetrics.matchedRatio,
                secondMetrics.pass ? "PASS" : "FAIL");
    std::printf("capture_status=COMPLETE\n");

    ACL_CHECK(aclrtFree(deviceX));
    ACL_CHECK(aclrtFree(deviceResidual));
    ACL_CHECK(aclrtFree(deviceGamma));
    ACL_CHECK(aclrtFree(deviceBias));
    ACL_CHECK(aclrtFree(deviceOutput));
    ACL_CHECK(aclrtDestroyStream(stream));
    ACL_CHECK(aclFinalize());
    return 0;
}
