#include "local_abi_shim.h"

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

#ifndef EPI_PARENT_SOURCE_SHA256
#error EPI_PARENT_SOURCE_SHA256 is required
#endif
#ifndef EPI_CANDIDATE_SOURCE_SHA256
#error EPI_CANDIDATE_SOURCE_SHA256 is required
#endif

extern "C" void epi_run_kernel_parent(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);
extern "C" void epi_run_kernel_candidate(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);

namespace {

constexpr int kDeviceId = 7;
constexpr int32_t kFp32 = 0;
constexpr int64_t kRows = 2;
constexpr int64_t kAvailableCoreNum = 1;
constexpr float kEpsilon = 1.0e-5f;
constexpr double kAtol = 1.0 / 65536.0;
constexpr double kRtol = 1.0 / 1024.0;
constexpr double kRequiredMatchedRatio = 0.99;
constexpr double kMaxAbsErrorLimit = 1.0e-2;

struct KernelEntry {
    const char* side;
    const char* sourceSha256;
    void (*call)(GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
                 GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
                 GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);
};

const KernelEntry kParent{"parent", EPI_PARENT_SOURCE_SHA256, epi_run_kernel_parent};
const KernelEntry kCandidate{"candidate", EPI_CANDIDATE_SOURCE_SHA256, epi_run_kernel_candidate};

bool CheckAcl(aclError status, const char* operation)
{
    if (status == ACL_SUCCESS) return true;
    std::fprintf(stderr, "%s failed: ACL status %d\n", operation, static_cast<int>(status));
    return false;
}

bool ParseInteger(const char* text, long long minimum, long long maximum, long long& value)
{
    errno = 0;
    char* end = nullptr;
    const long long parsed = std::strtoll(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || parsed < minimum || parsed > maximum) return false;
    value = parsed;
    return true;
}

bool SupportedWidth(int64_t width)
{
    return width == 8192 || width == 8193 || width == 12288 || width == 16384 ||
           width == 18416 || width == 18417 || width == 32768;
}

int32_t SourceDerivedBatchRows(int64_t width)
{
    constexpr int64_t kWideThreshold = 8192;
    constexpr int64_t kYBudgetBytes = 176 * 1024;
    constexpr int64_t kMaxRows = 8;
    constexpr int64_t kTileElems = 4096;
    constexpr int64_t kReduceStride = 16;
    constexpr int64_t kIoTiles = 2;
    if (width <= kWideThreshold) return 0;

    const int64_t yBytesPerRow = width * static_cast<int64_t>(sizeof(float));
    const int64_t ioBytes = kIoTiles * kTileElems * static_cast<int64_t>(sizeof(float));
    int64_t configuredRows = 1;
    for (int64_t rows = kMaxRows; rows > 1; --rows) {
        const int64_t reduceBytes = rows * kReduceStride * static_cast<int64_t>(sizeof(float));
        if (yBytesPerRow * rows + ioBytes + reduceBytes <= kYBudgetBytes) {
            configuredRows = rows;
            break;
        }
    }
    const int64_t localRows = kRows / kAvailableCoreNum;
    return static_cast<int32_t>(std::min(localRows, configuredRows));
}

void FillInputs(int64_t width, std::vector<float>& x, std::vector<float>& residual,
                std::vector<float>& gamma, std::vector<float>& bias)
{
    for (int64_t row = 0; row < kRows; ++row) {
        for (int64_t col = 0; col < width; ++col) {
            const size_t index = static_cast<size_t>(row * width + col);
            const int64_t xCode = (index * 17 + row * 101) % 2001 - 1000;
            const int64_t rCode = (index * 29 + row * 73) % 2001 - 1000;
            x[index] = static_cast<float>(xCode) / 200.0f;
            residual[index] = static_cast<float>(rCode) / 250.0f;
        }
    }
    for (int64_t col = 0; col < width; ++col) {
        const int64_t gammaCode = (col * 37) % 1001;
        const int64_t biasCode = (col * 43) % 2001 - 1000;
        gamma[static_cast<size_t>(col)] = 0.5f + static_cast<float>(gammaCode) / 1000.0f;
        bias[static_cast<size_t>(col)] = static_cast<float>(biasCode) / 1000.0f;
    }
}

std::vector<float> MakeGolden(int64_t width, const std::vector<float>& x,
                              const std::vector<float>& residual,
                              const std::vector<float>& gamma,
                              const std::vector<float>& bias)
{
    std::vector<float> golden(static_cast<size_t>(kRows * width));
    for (int64_t row = 0; row < kRows; ++row) {
        double squareSum = 0.0;
        for (int64_t col = 0; col < width; ++col) {
            const size_t index = static_cast<size_t>(row * width + col);
            const float value = x[index] + residual[index];
            squareSum += static_cast<double>(value) * static_cast<double>(value);
        }
        const float meanSquare = static_cast<float>(squareSum / static_cast<double>(width));
        const float invRms = 1.0f / std::sqrt(meanSquare + kEpsilon);
        for (int64_t col = 0; col < width; ++col) {
            const size_t index = static_cast<size_t>(row * width + col);
            const float value = x[index] + residual[index];
            const float normalized = value * invRms;
            const float scaled = normalized * gamma[static_cast<size_t>(col)];
            golden[index] = scaled + bias[static_cast<size_t>(col)];
        }
    }
    return golden;
}

bool WriteResult(const char* path, const KernelEntry& kernel, int64_t width,
                 const std::vector<float>& golden, const std::vector<float>& actual)
{
    const int fd = open(path, O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (fd < 0) {
        std::fprintf(stderr, "cannot create new result file %s: %s\n", path, std::strerror(errno));
        return false;
    }
    FILE* output = fdopen(fd, "w");
    if (output == nullptr) {
        std::fprintf(stderr, "fdopen failed: %s\n", std::strerror(errno));
        close(fd);
        return false;
    }

    double maxAbsError = 0.0;
    size_t matched = 0;
    size_t nonfinite = 0;
    const size_t count = golden.size();
    for (size_t i = 0; i < count; ++i) {
        if (!std::isfinite(actual[i]) || !std::isfinite(golden[i])) {
            ++nonfinite;
            continue;
        }
        const double error = std::abs(static_cast<double>(actual[i]) - golden[i]);
        maxAbsError = std::max(maxAbsError, error);
        if (error <= kAtol + kRtol * std::abs(static_cast<double>(golden[i]))) ++matched;
    }
    const double matchedRatio = static_cast<double>(matched) / static_cast<double>(count);
    const bool passed = nonfinite == 0 && matchedRatio >= kRequiredMatchedRatio &&
                        maxAbsError <= kMaxAbsErrorLimit;
    const int writeRc = std::fprintf(output,
        "side\tsource_sha256\tdevice\trows\twidth\tdtype\tblock_count\tsource_derived_batch_rows\tmatched_ratio\tmax_abs_error\tatol\trtol\tmax_abs_error_limit\tnonfinite\tstatus\n"
        "%s\t%s\t%d\t%lld\t%lld\tFP32\t1\t%d\t%.9g\t%.9g\t%.9g\t%.9g\t%.9g\t%zu\t%s\n",
        kernel.side, kernel.sourceSha256, kDeviceId, static_cast<long long>(kRows),
        static_cast<long long>(width), SourceDerivedBatchRows(width), matchedRatio, maxAbsError, kAtol, kRtol,
        kMaxAbsErrorLimit, nonfinite, passed ? "PASS" : "FAIL");
    const bool closeOk = std::fclose(output) == 0;
    std::printf("CORRECTNESS side=%s source_sha256=%s device=%d rows=%lld width=%lld dtype=FP32 block_count=1 source_derived_batch_rows=%d matched_ratio=%.9g max_abs_error=%.9g nonfinite=%zu status=%s\n",
        kernel.side, kernel.sourceSha256, kDeviceId, static_cast<long long>(kRows),
        static_cast<long long>(width), SourceDerivedBatchRows(width), matchedRatio, maxAbsError, nonfinite,
        passed ? "PASS" : "FAIL");
    return writeRc >= 0 && closeOk && passed;
}

}  // namespace

int main(int argc, char** argv)
{
    if (argc != 5) {
        std::fprintf(stderr, "Usage: %s DEVICE SIDE WIDTH OUTPUT.tsv\n", argv[0]);
        return 2;
    }
    long long deviceArg = -1;
    long long widthArg = -1;
    if (!ParseInteger(argv[1], 0, 7, deviceArg) || !ParseInteger(argv[3], 1, 32768, widthArg) ||
        deviceArg != kDeviceId || !SupportedWidth(static_cast<int64_t>(widthArg)) ||
        (std::strcmp(argv[2], "parent") != 0 && std::strcmp(argv[2], "candidate") != 0)) {
        std::fprintf(stderr, "unsupported device, side, or width; device 7 and the declared matrix are required\n");
        return 2;
    }

    const int64_t width = static_cast<int64_t>(widthArg);
    const KernelEntry& kernel = std::strcmp(argv[2], "parent") == 0 ? kParent : kCandidate;
    std::vector<float> x(static_cast<size_t>(kRows * width));
    std::vector<float> residual(x.size());
    std::vector<float> output(x.size());
    std::vector<float> gamma(static_cast<size_t>(width));
    std::vector<float> bias(static_cast<size_t>(width));
    FillInputs(width, x, residual, gamma, bias);
    const std::vector<float> golden = MakeGolden(width, x, residual, gamma, bias);

    const size_t inputBytes = x.size() * sizeof(float);
    const size_t vectorBytes = gamma.size() * sizeof(float);
    void *deviceX = nullptr, *deviceResidual = nullptr, *deviceGamma = nullptr;
    void *deviceBias = nullptr, *deviceOutput = nullptr;
    aclrtStream stream = nullptr;
    bool initialized = false;
    bool ok = CheckAcl(aclInit(nullptr), "aclInit");
    initialized = ok;
    if (ok) {
        ok = CheckAcl(aclrtSetDevice(kDeviceId), "aclrtSetDevice");
    }
    if (ok) ok = CheckAcl(aclrtCreateStream(&stream), "aclrtCreateStream");
    if (ok) ok = CheckAcl(aclrtMalloc(&deviceX, inputBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(x)");
    if (ok) ok = CheckAcl(aclrtMalloc(&deviceResidual, inputBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(residual)");
    if (ok) ok = CheckAcl(aclrtMalloc(&deviceGamma, vectorBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(gamma)");
    if (ok) ok = CheckAcl(aclrtMalloc(&deviceBias, vectorBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(bias)");
    if (ok) ok = CheckAcl(aclrtMalloc(&deviceOutput, inputBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(output)");
    if (ok) ok = CheckAcl(aclrtMemcpy(deviceX, inputBytes, x.data(), inputBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy x");
    if (ok) ok = CheckAcl(aclrtMemcpy(deviceResidual, inputBytes, residual.data(), inputBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy residual");
    if (ok) ok = CheckAcl(aclrtMemcpy(deviceGamma, vectorBytes, gamma.data(), vectorBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma");
    if (ok) ok = CheckAcl(aclrtMemcpy(deviceBias, vectorBytes, bias.data(), vectorBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy bias");
    if (ok) ok = CheckAcl(aclrtMemset(deviceOutput, inputBytes, 0xff, inputBytes), "initialize output");

    const int64_t inputShape[] = {kRows, width};
    const int64_t vectorShape[] = {width};
    const TensorInfo inputInfo{inputShape, 2, kFp32};
    const TensorInfo vectorInfo{vectorShape, 1, kFp32};
    const TensorGroupInfo inputGroup{&inputInfo, 1};
    const TensorGroupInfo vectorGroup{&vectorInfo, 1};
    if (ok) {
        kernel.call(deviceX, inputGroup, deviceResidual, inputGroup,
                    deviceGamma, vectorGroup, deviceBias, vectorGroup,
                    deviceOutput, inputGroup, kAvailableCoreNum, stream, kEpsilon);
        ok = CheckAcl(aclrtSynchronizeStream(stream), "aclrtSynchronizeStream(correctness)") &&
             CheckAcl(aclrtMemcpy(output.data(), inputBytes, deviceOutput, inputBytes,
                                 ACL_MEMCPY_DEVICE_TO_HOST), "copy output");
    }
    if (ok) ok = WriteResult(argv[4], kernel, width, golden, output);

    if (stream != nullptr) {
        const aclError drain = aclrtSynchronizeStream(stream);
        if (drain != ACL_SUCCESS) {
            std::fprintf(stderr, "cleanup stream synchronization failed: %d\n", static_cast<int>(drain));
            ok = false;
        }
    }
    const auto release = [&ok](aclError status, const char* name) {
        if (status != ACL_SUCCESS) {
            std::fprintf(stderr, "cleanup %s failed: %d\n", name, static_cast<int>(status));
            ok = false;
        }
    };
    if (deviceX != nullptr) release(aclrtFree(deviceX), "x");
    if (deviceResidual != nullptr) release(aclrtFree(deviceResidual), "residual");
    if (deviceGamma != nullptr) release(aclrtFree(deviceGamma), "gamma");
    if (deviceBias != nullptr) release(aclrtFree(deviceBias), "bias");
    if (deviceOutput != nullptr) release(aclrtFree(deviceOutput), "output");
    if (stream != nullptr) release(aclrtDestroyStream(stream), "stream");
    // Let process teardown release this context; never reset the assigned device.
    if (initialized) release(aclFinalize(), "aclFinalize");
    return ok ? 0 : 1;
}
