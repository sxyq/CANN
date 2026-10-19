#include "local_abi_shim.h"

#include <acl/acl.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <limits>
#include <numeric>
#include <string>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

#ifndef RUNNER_PARENT_SOURCE_PATH
#error RUNNER_PARENT_SOURCE_PATH is required
#endif
#ifndef RUNNER_CANDIDATE_SOURCE_PATH
#error RUNNER_CANDIDATE_SOURCE_PATH is required
#endif
#ifndef RUNNER_PARENT_SOURCE_SHA256
#error RUNNER_PARENT_SOURCE_SHA256 is required
#endif
#ifndef RUNNER_CANDIDATE_SOURCE_SHA256
#error RUNNER_CANDIDATE_SOURCE_SHA256 is required
#endif

extern "C" void run_kernel_parent(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);
extern "C" void run_kernel_candidate(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);

namespace {

constexpr int32_t kFp32 = 0;
constexpr int32_t kFp16 = 1;
constexpr int32_t kBf16 = 2;
constexpr int kMinimumWarmups = 45;
constexpr int kMinimumSamples = 21;
constexpr int kMinimumPairedBlocks = 4;

struct Kernel {
    const char* name;
    const char* sourcePath;
    const char* sourceSha256;
    void (*call)(GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
                 GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
                 GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);
};

const Kernel kParent{"parent", RUNNER_PARENT_SOURCE_PATH, RUNNER_PARENT_SOURCE_SHA256, run_kernel_parent};
const Kernel kCandidate{"candidate", RUNNER_CANDIDATE_SOURCE_PATH, RUNNER_CANDIDATE_SOURCE_SHA256, run_kernel_candidate};

bool CheckAcl(aclError status, const char* operation)
{
    if (status == ACL_SUCCESS) return true;
    std::fprintf(stderr, "%s failed: ACL status %d\n", operation, static_cast<int>(status));
    return false;
}

const char* DTypeName(int32_t dtype)
{
    if (dtype == kFp32) return "fp32";
    if (dtype == kFp16) return "fp16";
    return "bf16";
}

size_t ElementSize(int32_t dtype)
{
    return dtype == kFp32 ? sizeof(float) : sizeof(uint16_t);
}

uint16_t FloatToBf16(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    bits += 0x7fffU + ((bits >> 16) & 1U);
    return static_cast<uint16_t>(bits >> 16);
}

uint16_t FloatToFp16(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    const uint16_t sign = static_cast<uint16_t>((bits >> 16) & 0x8000U);
    const uint32_t exponent = (bits >> 23) & 0xffU;
    uint32_t mantissa = bits & 0x7fffffU;
    if (exponent == 0xffU) {
        return static_cast<uint16_t>(sign | 0x7c00U | (mantissa == 0 ? 0 : 0x0200U));
    }
    const int32_t halfExponent = static_cast<int32_t>(exponent) - 127 + 15;
    if (halfExponent >= 31) return static_cast<uint16_t>(sign | 0x7c00U);
    if (halfExponent <= 0) {
        if (halfExponent < -10) return sign;
        mantissa |= 0x800000U;
        const uint32_t shift = static_cast<uint32_t>(14 - halfExponent);
        uint32_t result = mantissa >> shift;
        const uint32_t remainder = mantissa & ((1U << shift) - 1U);
        const uint32_t halfway = 1U << (shift - 1U);
        if (remainder > halfway || (remainder == halfway && (result & 1U))) ++result;
        return static_cast<uint16_t>(sign | result);
    }
    uint32_t result = (static_cast<uint32_t>(halfExponent) << 10) | (mantissa >> 13);
    const uint32_t remainder = mantissa & 0x1fffU;
    if (remainder > 0x1000U || (remainder == 0x1000U && (result & 1U))) ++result;
    return static_cast<uint16_t>(sign | result);
}

void Store(std::vector<uint8_t>& data, size_t index, int32_t dtype, float value)
{
    if (dtype == kFp32) {
        std::memcpy(data.data() + index * sizeof(value), &value, sizeof(value));
        return;
    }
    const uint16_t packed = dtype == kFp16 ? FloatToFp16(value) : FloatToBf16(value);
    std::memcpy(data.data() + index * sizeof(packed), &packed, sizeof(packed));
}

float Load(const std::vector<uint8_t>& data, size_t index, int32_t dtype)
{
    if (dtype == kFp32) {
        float value = 0.0f;
        std::memcpy(&value, data.data() + index * sizeof(value), sizeof(value));
        return value;
    }
    uint16_t packed = 0;
    std::memcpy(&packed, data.data() + index * sizeof(packed), sizeof(packed));
    if (dtype == kBf16) {
        const uint32_t bits = static_cast<uint32_t>(packed) << 16;
        float value = 0.0f;
        std::memcpy(&value, &bits, sizeof(value));
        return value;
    }
    const int exponent = (packed >> 10) & 0x1f;
    const int mantissa = packed & 0x03ff;
    float value = exponent == 0 ? std::ldexp(static_cast<float>(mantissa), -24) :
                  exponent == 31 ? (mantissa == 0 ? std::numeric_limits<float>::infinity() :
                                    std::numeric_limits<float>::quiet_NaN()) :
                  std::ldexp(static_cast<float>(1024 + mantissa), exponent - 25);
    return (packed & 0x8000U) != 0 ? -value : value;
}

bool CompareOutput(FILE* output, const Kernel& kernel, int device, int32_t dtype,
                   int64_t rows, int64_t width, double epsilon, double atol, double rtol,
                   const std::vector<uint8_t>& x,
                   const std::vector<uint8_t>& residual, const std::vector<uint8_t>& gamma,
                   const std::vector<uint8_t>& bias, const std::vector<uint8_t>& actual)
{
    std::vector<uint8_t> expected(ElementSize(dtype));
    double maxError = 0.0, maxRelativeError = 0.0;
    size_t mismatches = 0, nonfinite = 0;
    for (int64_t row = 0; row < rows; ++row) {
        double squareSum = 0.0;
        for (int64_t j = 0; j < width; ++j) {
            const size_t index = static_cast<size_t>(row * width + j);
            const float value = Load(x, index, dtype) + Load(residual, index, dtype);
            squareSum += static_cast<double>(value) * value;
        }
        const float invRms = 1.0f / std::sqrt(static_cast<float>(squareSum / width + epsilon));
        for (int64_t j = 0; j < width; ++j) {
            const size_t index = static_cast<size_t>(row * width + j);
            const float value = (Load(x, index, dtype) + Load(residual, index, dtype)) * invRms *
                                Load(gamma, static_cast<size_t>(j), dtype) + Load(bias, static_cast<size_t>(j), dtype);
            Store(expected, 0, dtype, value);
            const float got = Load(actual, index, dtype), want = Load(expected, 0, dtype);
            if (!std::isfinite(got) || !std::isfinite(want)) {
                ++nonfinite;
                ++mismatches;
                continue;
            }
            const double error = std::abs(static_cast<double>(got) - static_cast<double>(want));
            const double relativeError = error / std::max(1.0e-30, std::abs(static_cast<double>(want)));
            maxError = std::max(maxError, error);
            maxRelativeError = std::max(maxRelativeError, relativeError);
            if (error > atol + rtol * std::abs(static_cast<double>(want))) ++mismatches;
        }
    }
    const bool written = std::fprintf(output, "%s\t%s\t%d\t%lld\t%lld\t%s\t%.9g\t%.9g\t%.9g\t%.9g\t%zu\t%zu\t%s\n",
        kernel.name, kernel.sourceSha256, device, static_cast<long long>(rows),
        static_cast<long long>(width), DTypeName(dtype), maxError, maxRelativeError,
        atol, rtol, mismatches, nonfinite, mismatches == 0 ? "PASS" : "FAIL") >= 0;
    std::printf("CORRECTNESS side=%s rows=%lld width=%lld dtype=%s max_abs_error=%.9g max_relative_error=%.9g mismatches=%zu nonfinite=%zu\n",
        kernel.name, static_cast<long long>(rows), static_cast<long long>(width),
        DTypeName(dtype), maxError, maxRelativeError, mismatches, nonfinite);
    return written && mismatches == 0;
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

bool ParseDouble(const char* text, double minimum, double maximum, double& value)
{
    errno = 0;
    char* end = nullptr;
    const double parsed = std::strtod(text, &end);
    if (errno != 0 || end == text || *end != '\0' || !std::isfinite(parsed) ||
        parsed < minimum || parsed > maximum) return false;
    value = parsed;
    return true;
}

uint64_t Mix64(uint64_t value)
{
    value += 0x9e3779b97f4a7c15ULL;
    value = (value ^ (value >> 30)) * 0xbf58476d1ce4e5b9ULL;
    value = (value ^ (value >> 27)) * 0x94d049bb133111ebULL;
    return value ^ (value >> 31);
}

float UnitSample(uint64_t seed, uint64_t index, uint64_t stream)
{
    const uint64_t bits = Mix64(seed ^ (index * 0xd6e8feb86659fd93ULL) ^
                                (stream * 0xa0761d6478bd642fULL));
    return static_cast<float>((bits >> 40) & 0xffffffU) / 16777216.0f;
}

struct DeviceMetadata {
    long long hbmFree = -1;
    long long hbmTotal = -1;
    long long cubeUtilization = -1;
    long long vectorUtilization = -1;
    long long aiCpuUtilization = -1;
    long long memoryUtilization = -1;
};

DeviceMetadata QueryDeviceMetadata(int device)
{
    DeviceMetadata result;
    size_t freeBytes = 0, totalBytes = 0;
    if (aclrtGetMemInfo(ACL_HBM_MEM, &freeBytes, &totalBytes) == ACL_SUCCESS) {
        result.hbmFree = static_cast<long long>(freeBytes);
        result.hbmTotal = static_cast<long long>(totalBytes);
    }
    aclrtUtilizationInfo utilization{};
    if (aclrtGetDeviceUtilizationRate(device, &utilization) == ACL_SUCCESS) {
        result.cubeUtilization = utilization.cubeUtilization;
        result.vectorUtilization = utilization.vectorUtilization;
        result.aiCpuUtilization = utilization.aicpuUtilization;
        result.memoryUtilization = utilization.memoryUtilization;
    }
    return result;
}

void WriteDeviceMetadata(FILE* output, const char* label, const DeviceMetadata& data)
{
    if (output == nullptr) return;
    std::fprintf(output, "# %s_HBM_FREE_BYTES=%lld\n", label, data.hbmFree);
    std::fprintf(output, "# %s_HBM_TOTAL_BYTES=%lld\n", label, data.hbmTotal);
    std::fprintf(output, "# %s_CUBE_UTILIZATION_PERCENT=%lld\n", label, data.cubeUtilization);
    std::fprintf(output, "# %s_VECTOR_UTILIZATION_PERCENT=%lld\n", label, data.vectorUtilization);
    std::fprintf(output, "# %s_AICPU_UTILIZATION_PERCENT=%lld\n", label, data.aiCpuUtilization);
    std::fprintf(output, "# %s_MEMORY_UTILIZATION_PERCENT=%lld\n", label, data.memoryUtilization);
}

bool WritePreamble(FILE* output, const char* kind, const char* mode,
                   const char* dtypeName, int32_t dtypeCode,
                   int device, int64_t rows, int64_t width, uint64_t seed,
                   double epsilon, double atol, double rtol,
                   int64_t cores, int warmups, int samples, int blocks)
{
    if (output == nullptr) return true;
    const size_t elementSize = dtypeCode == kFp32 ? sizeof(float) : sizeof(uint16_t);
    const long double bytes = (3.0L * rows * width + 2.0L * width) * elementSize;
    return std::fprintf(output,
        "# FORMAT=exact-source-pair-runner-v1\n"
        "# ARTIFACT=%s\n# MODE=%s\n"
        "# PARENT_SOURCE_PATH=%s\n# PARENT_SOURCE_SHA256=%s\n"
        "# CANDIDATE_SOURCE_PATH=%s\n# CANDIDATE_SOURCE_SHA256=%s\n"
        "# DEVICE=%d\n# ROWS=%lld\n# WIDTH=%lld\n# DTYPE=%s\n# DTYPE_CODE=%d\n"
        "# INPUT_SEED=%llu\n# INPUT_GENERATOR=splitmix64_uniform_v1\n"
        "# EPSILON=%.17g\n# ATOL=%.17g\n# RTOL=%.17g\n# AVAILABLE_VECTOR_CORES=%lld\n"
        "# WARMUPS_PER_SIDE=%d\n# SAMPLES_PER_BLOCK=%d\n# PAIRED_BLOCKS=%d\n"
        "# NOMINAL_HBM_BYTES_PER_INVOCATION=%.0Lf\n",
        kind, mode, kParent.sourcePath, kParent.sourceSha256,
        kCandidate.sourcePath, kCandidate.sourceSha256, device,
        static_cast<long long>(rows), static_cast<long long>(width), dtypeName,
        dtypeCode, static_cast<unsigned long long>(seed), epsilon, atol, rtol,
        static_cast<long long>(cores), warmups, samples, blocks, bytes) >= 0;
}

bool WriteSample(FILE* output, const Kernel& kernel, int block, int sample,
                 const char* order, int device, int64_t rows, int64_t width,
                 const char* dtypeName, int32_t dtypeCode, long double nominalBytes,
                 double deviceUs, double wallUs)
{
    const double throughput = deviceUs > 0.0 ? static_cast<double>(nominalBytes / deviceUs / 1000.0L) : 0.0;
    return std::fprintf(output,
        "%d\t%d\t%s\t%s\t%s\t%d\t%lld\t%lld\t%s\t%d\t%.6f\t%.3f\t%.0Lf\t%.6f\n",
        block, sample, order, kernel.name, kernel.sourceSha256, device,
        static_cast<long long>(rows), static_cast<long long>(width), dtypeName,
        dtypeCode, deviceUs, wallUs, nominalBytes, throughput) >= 0;
}

struct SampleStats {
    double median = 0.0;
    double mean = 0.0;
    double stdev = 0.0;
    double cv = 0.0;
    double min = 0.0;
    double max = 0.0;
    double maxOverMin = 0.0;
    double mad = 0.0;
    double p10 = 0.0;
    double p90 = 0.0;
};

double Quantile(const std::vector<double>& sorted, double fraction)
{
    if (sorted.empty()) return 0.0;
    const double position = fraction * static_cast<double>(sorted.size() - 1);
    const size_t lower = static_cast<size_t>(position);
    const size_t upper = std::min(sorted.size() - 1, lower + 1);
    const double weight = position - static_cast<double>(lower);
    return sorted[lower] + (sorted[upper] - sorted[lower]) * weight;
}

SampleStats Summarize(const std::vector<double>& values)
{
    SampleStats result;
    if (values.empty()) return result;
    std::vector<double> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    result.min = sorted.front();
    result.max = sorted.back();
    result.median = Quantile(sorted, 0.5);
    result.p10 = Quantile(sorted, 0.1);
    result.p90 = Quantile(sorted, 0.9);
    result.mean = std::accumulate(values.begin(), values.end(), 0.0) / values.size();
    double variance = 0.0;
    for (double value : values) variance += (value - result.mean) * (value - result.mean);
    result.stdev = std::sqrt(variance / values.size());
    result.cv = result.mean > 0.0 ? result.stdev / result.mean : 0.0;
    result.maxOverMin = result.min > 0.0 ? result.max / result.min : 0.0;
    std::vector<double> deviations;
    deviations.reserve(values.size());
    for (double value : values) deviations.push_back(std::abs(value - result.median));
    std::sort(deviations.begin(), deviations.end());
    result.mad = Quantile(deviations, 0.5);
    return result;
}

bool WriteTimingSummary(FILE* output, const char* side, const std::vector<double>& values,
                        long double nominalBytes)
{
    if (output == nullptr) return true;
    const SampleStats stats = Summarize(values);
    const double throughput = stats.median > 0.0
        ? static_cast<double>(nominalBytes / stats.median / 1000.0L) : 0.0;
    return std::fprintf(output,
        "%s\t%zu\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\n",
        side, values.size(), stats.median, stats.mean, stats.stdev, stats.cv,
        stats.min, stats.max, stats.maxOverMin, stats.mad, stats.p10, stats.p90,
        stats.max - stats.min, throughput) >= 0;
}

bool WritePairSummary(FILE* output, const std::vector<double>& deltas)
{
    if (output == nullptr) return true;
    const SampleStats stats = Summarize(deltas);
    return std::fprintf(output,
        "paired_candidate_minus_parent_us\t%zu\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t\n",
        deltas.size(), stats.median, stats.mean, stats.stdev, stats.cv,
        stats.min, stats.max, stats.maxOverMin, stats.mad, stats.p10,
        stats.p90, stats.max - stats.min) >= 0;
}

bool LaunchAndMeasure(const Kernel& kernel, void* deviceX, void* deviceResidual,
                      void* deviceGamma, void* deviceBias, void* deviceOutput,
                      const TensorGroupInfo& inputGroup, const TensorGroupInfo& vectorGroup,
                      int64_t cores, float epsilon, aclrtStream stream,
                      aclrtEvent startEvent, aclrtEvent stopEvent,
                      double& deviceUs, double& wallUs)
{
    const auto wallStart = std::chrono::steady_clock::now();
    if (!CheckAcl(aclrtRecordEvent(startEvent, stream), "aclrtRecordEvent(start)")) return false;
    kernel.call(deviceX, inputGroup, deviceResidual, inputGroup,
                deviceGamma, vectorGroup, deviceBias, vectorGroup,
                deviceOutput, inputGroup, cores, stream, epsilon);
    if (!CheckAcl(aclrtRecordEvent(stopEvent, stream), "aclrtRecordEvent(stop)")) return false;
    if (!CheckAcl(aclrtSynchronizeEvent(stopEvent), "aclrtSynchronizeEvent")) return false;
    const auto wallStop = std::chrono::steady_clock::now();
    float elapsedMs = 0.0f;
    if (!CheckAcl(aclrtEventElapsedTime(&elapsedMs, startEvent, stopEvent), "aclrtEventElapsedTime")) return false;
    deviceUs = static_cast<double>(elapsedMs) * 1000.0;
    wallUs = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(wallStop - wallStart).count()) / 1000.0;
    return true;
}

bool Warmup(const Kernel& kernel, int count, void* deviceX, void* deviceResidual,
            void* deviceGamma, void* deviceBias, void* deviceOutput,
            const TensorGroupInfo& inputGroup, const TensorGroupInfo& vectorGroup,
            int64_t cores, float epsilon, aclrtStream stream)
{
    for (int i = 0; i < count; ++i) {
        kernel.call(deviceX, inputGroup, deviceResidual, inputGroup,
                    deviceGamma, vectorGroup, deviceBias, vectorGroup,
                    deviceOutput, inputGroup, cores, stream, epsilon);
        if (!CheckAcl(aclrtSynchronizeStream(stream), "aclrtSynchronizeStream(warmup)")) return false;
    }
    return true;
}

void PrintUsage(const char* program)
{
    std::printf("Usage: %s --device ID --rows M --width D --dtype fp32|fp16|bf16 --dtype-code N \\\n+ --epsilon E --atol A --rtol R --seed N --output-prefix PATH [--cores N] \\\n+ [--mode correctness|local] [--warmups N --samples N --blocks N]\n", program);
    std::printf("local mode runs CPU-golden correctness first; on pass it performs adjacent, alternating P/C ");
    std::printf("device-event timing. Outputs are created exclusively and never overwrite existing evidence.\n");
}

}  // namespace

int main(int argc, char** argv)
{
    long long deviceValue = -1, rowsValue = -1, widthValue = -1, dtypeCodeValue = -1;
    long long coresValue = 0, seedValue = -1, blocksValue = 4, warmupsValue = 45, samplesValue = 21;
    double epsilonValue = -1.0, atolValue = -1.0, rtolValue = -1.0;
    std::string dtypeValue, modeValue = "local", outputPrefix;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            PrintUsage(argv[0]);
            return 0;
        }
        if (i + 1 >= argc) { PrintUsage(argv[0]); return 2; }
        const char* option = argv[i++];
        const char* value = argv[i];
        long long* numeric = nullptr;
        double* real = nullptr;
        if (std::strcmp(option, "--device") == 0) numeric = &deviceValue;
        else if (std::strcmp(option, "--rows") == 0) numeric = &rowsValue;
        else if (std::strcmp(option, "--width") == 0) numeric = &widthValue;
        else if (std::strcmp(option, "--dtype-code") == 0) numeric = &dtypeCodeValue;
        else if (std::strcmp(option, "--cores") == 0) numeric = &coresValue;
        else if (std::strcmp(option, "--seed") == 0) numeric = &seedValue;
        else if (std::strcmp(option, "--blocks") == 0) numeric = &blocksValue;
        else if (std::strcmp(option, "--warmups") == 0) numeric = &warmupsValue;
        else if (std::strcmp(option, "--samples") == 0) numeric = &samplesValue;
        else if (std::strcmp(option, "--epsilon") == 0) real = &epsilonValue;
        else if (std::strcmp(option, "--atol") == 0) real = &atolValue;
        else if (std::strcmp(option, "--rtol") == 0) real = &rtolValue;
        else if (std::strcmp(option, "--dtype") == 0) dtypeValue = value;
        else if (std::strcmp(option, "--mode") == 0) modeValue = value;
        else if (std::strcmp(option, "--output-prefix") == 0) outputPrefix = value;
        else { std::fprintf(stderr, "unknown option: %s\n", option); return 2; }
        if (numeric != nullptr && !ParseInteger(value, 0, std::numeric_limits<int32_t>::max(), *numeric)) {
            std::fprintf(stderr, "invalid integer for %s\n", option); return 2;
        }
        if (real != nullptr && !ParseDouble(value, 0.0, 1.0e6, *real)) {
            std::fprintf(stderr, "invalid real value for %s\n", option); return 2;
        }
    }

    if (deviceValue < 0 || rowsValue < 1 || widthValue < 1 || dtypeCodeValue < 0 ||
        seedValue < 0 || epsilonValue < 0 || atolValue < 0 || rtolValue < 0 ||
        outputPrefix.empty() || (modeValue != "correctness" && modeValue != "local") ||
        rowsValue > std::numeric_limits<int64_t>::max() / widthValue ||
        (dtypeValue != "fp32" && dtypeValue != "fp16" && dtypeValue != "bf16") ||
        blocksValue < 1 || warmupsValue < 0 || samplesValue < 0 || coresValue < 0) {
        PrintUsage(argv[0]); return 2;
    }
    const bool correctnessOnly = modeValue == "correctness";
    if (!correctnessOnly && (warmupsValue < kMinimumWarmups || samplesValue < kMinimumSamples ||
                             blocksValue < kMinimumPairedBlocks)) {
        std::fprintf(stderr, "local sampling requires at least %d warmups, %d samples, and %d paired blocks\n",
                     kMinimumWarmups, kMinimumSamples, kMinimumPairedBlocks);
        return 2;
    }
    const int32_t storageType = dtypeValue == "fp32" ? kFp32 : dtypeValue == "fp16" ? kFp16 : kBf16;
    const int32_t dtypeCode = static_cast<int32_t>(dtypeCodeValue);
    const int device = static_cast<int>(deviceValue);
    const int64_t rows = static_cast<int64_t>(rowsValue);
    const int64_t width = static_cast<int64_t>(widthValue);
    const size_t elementSize = ElementSize(storageType);
    const uint64_t inputElements64 = static_cast<uint64_t>(rows) * static_cast<uint64_t>(width);
    if (inputElements64 > std::numeric_limits<size_t>::max() / elementSize ||
        static_cast<uint64_t>(width) > std::numeric_limits<size_t>::max() / elementSize) {
        std::fprintf(stderr, "tensor byte size overflows host size_t\n"); return 2;
    }
    const size_t inputElements = static_cast<size_t>(inputElements64);
    const size_t vectorElements = static_cast<size_t>(width);
    const size_t inputBytes = inputElements * elementSize;
    const size_t vectorBytes = vectorElements * elementSize;
    const long double nominalBytes = (3.0L * rows * width + 2.0L * width) * elementSize;
    if (nominalBytes > static_cast<long double>(std::numeric_limits<uint64_t>::max())) {
        std::fprintf(stderr, "nominal traffic byte count overflows uint64_t\n"); return 2;
    }

    std::vector<uint8_t> x(inputBytes), residual(inputBytes), gamma(vectorBytes), bias(vectorBytes);
    const uint64_t seed = static_cast<uint64_t>(seedValue);
    for (size_t i = 0; i < inputElements; ++i) {
        Store(x, i, storageType, (UnitSample(seed, i, 0) - 0.5f) * 0.1f);
        Store(residual, i, storageType, (UnitSample(seed, i, 1) - 0.5f) * 0.1f);
    }
    for (size_t i = 0; i < vectorElements; ++i) {
        Store(gamma, i, storageType, 0.8f + UnitSample(seed, i, 2) * 0.4f);
        Store(bias, i, storageType, (UnitSample(seed, i, 3) - 0.5f) * 0.02f);
    }

    const std::string correctnessPath = outputPrefix + ".correctness.tsv";
    const int correctnessFd = open(correctnessPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (correctnessFd < 0) {
        std::fprintf(stderr, "cannot create new output file %s: %s\n", correctnessPath.c_str(), std::strerror(errno));
        return 1;
    }
    FILE* correctnessOutput = fdopen(correctnessFd, "w");
    if (correctnessOutput == nullptr) { close(correctnessFd); return 1; }
    FILE* rawOutput = nullptr;
    FILE* summaryOutput = nullptr;
    if (!correctnessOnly) {
        const std::string rawPath = outputPrefix + ".raw.tsv";
        const int rawFd = open(rawPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
        if (rawFd < 0) {
            std::fprintf(stderr, "cannot create new output file %s: %s\n", rawPath.c_str(), std::strerror(errno));
            std::fclose(correctnessOutput);
            return 1;
        }
        rawOutput = fdopen(rawFd, "w");
        if (rawOutput == nullptr) { close(rawFd); std::fclose(correctnessOutput); return 1; }
        const std::string summaryPath = outputPrefix + ".summary.tsv";
        const int summaryFd = open(summaryPath.c_str(), O_WRONLY | O_CREAT | O_EXCL, 0644);
        if (summaryFd < 0) {
            std::fprintf(stderr, "cannot create new output file %s: %s\n", summaryPath.c_str(), std::strerror(errno));
            std::fclose(rawOutput);
            std::fclose(correctnessOutput);
            return 1;
        }
        summaryOutput = fdopen(summaryFd, "w");
        if (summaryOutput == nullptr) {
            close(summaryFd);
            std::fclose(rawOutput);
            std::fclose(correctnessOutput);
            return 1;
        }
    }

    bool runtimeInitialized = false;
    bool deviceSelected = false;
    bool resetDevice = false;
    bool passed = CheckAcl(aclInit(nullptr), "aclInit");
    runtimeInitialized = passed;
    uint32_t deviceCount = 0;
    if (passed) passed = CheckAcl(aclrtGetDeviceCount(&deviceCount), "aclrtGetDeviceCount") &&
                         static_cast<uint32_t>(device) < deviceCount;
    if (passed) {
        if (static_cast<uint32_t>(device) >= deviceCount) std::fprintf(stderr, "device id exceeds runtime device count\n");
        passed = CheckAcl(aclrtSetDevice(device), "aclrtSetDevice");
        deviceSelected = passed;
        resetDevice = passed;
    }
    int64_t availableCores = 0;
    if (passed) passed = CheckAcl(aclrtGetDeviceInfo(static_cast<uint32_t>(device), ACL_DEV_ATTR_VECTOR_CORE_NUM,
                                                     &availableCores), "aclrtGetDeviceInfo(vector cores)");
    if (passed && availableCores < 1) { std::fprintf(stderr, "runtime reported no vector cores\n"); passed = false; }
    const int64_t cores = coresValue > 0 ? coresValue : availableCores;

    aclrtStream stream = nullptr;
    aclrtEvent startEvent = nullptr, stopEvent = nullptr;
    void *deviceX = nullptr, *deviceResidual = nullptr, *deviceGamma = nullptr;
    void *deviceBias = nullptr, *deviceOutput = nullptr;
    int exitCode = 0;
    const auto release = [&exitCode](aclError status, const char* operation) {
        if (status == ACL_SUCCESS) return;
        std::fprintf(stderr, "cleanup failed: %s returned %d\n", operation, static_cast<int>(status));
        exitCode = 1;
    };
    const auto cleanup = [&]() {
        if (stream != nullptr && aclrtSynchronizeStream(stream) != ACL_SUCCESS) {
            std::fprintf(stderr, "cleanup failed: stream synchronization failed; remaining runtime resources left intact\n");
            exitCode = 1;
            return;
        }
        if (startEvent != nullptr) release(aclrtDestroyEvent(startEvent), "aclrtDestroyEvent(start)");
        if (stopEvent != nullptr) release(aclrtDestroyEvent(stopEvent), "aclrtDestroyEvent(stop)");
        if (deviceX != nullptr) release(aclrtFree(deviceX), "aclrtFree(x)");
        if (deviceResidual != nullptr) release(aclrtFree(deviceResidual), "aclrtFree(residual)");
        if (deviceGamma != nullptr) release(aclrtFree(deviceGamma), "aclrtFree(gamma)");
        if (deviceBias != nullptr) release(aclrtFree(deviceBias), "aclrtFree(bias)");
        if (deviceOutput != nullptr) release(aclrtFree(deviceOutput), "aclrtFree(output)");
        if (stream != nullptr) release(aclrtDestroyStream(stream), "aclrtDestroyStream");
        if (resetDevice) release(aclrtResetDevice(device), "aclrtResetDevice");
        if (runtimeInitialized) release(aclFinalize(), "aclFinalize");
    };

    if (passed) passed = CheckAcl(aclrtCreateStream(&stream), "aclrtCreateStream");
    if (passed && !correctnessOnly) passed = CheckAcl(aclrtCreateEvent(&startEvent), "aclrtCreateEvent(start)") &&
                                              CheckAcl(aclrtCreateEvent(&stopEvent), "aclrtCreateEvent(stop)");
    if (passed) passed = CheckAcl(aclrtMalloc(&deviceX, inputBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(x)");
    if (passed) passed = CheckAcl(aclrtMalloc(&deviceResidual, inputBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(residual)");
    if (passed) passed = CheckAcl(aclrtMalloc(&deviceGamma, vectorBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(gamma)");
    if (passed) passed = CheckAcl(aclrtMalloc(&deviceBias, vectorBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(bias)");
    if (passed) passed = CheckAcl(aclrtMalloc(&deviceOutput, inputBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(output)");
    if (passed) passed = CheckAcl(aclrtMemcpy(deviceX, inputBytes, x.data(), inputBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy x");
    if (passed) passed = CheckAcl(aclrtMemcpy(deviceResidual, inputBytes, residual.data(), inputBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy residual");
    if (passed) passed = CheckAcl(aclrtMemcpy(deviceGamma, vectorBytes, gamma.data(), vectorBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma");
    if (passed) passed = CheckAcl(aclrtMemcpy(deviceBias, vectorBytes, bias.data(), vectorBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy bias");

    const int64_t inputShape[] = {rows, width};
    const int64_t vectorShape[] = {width};
    const TensorInfo inputInfo{inputShape, 2, dtypeCode};
    const TensorInfo vectorInfo{vectorShape, 1, dtypeCode};
    const TensorGroupInfo inputGroup{&inputInfo, 1};
    const TensorGroupInfo vectorGroup{&vectorInfo, 1};
    const int warmups = correctnessOnly ? 0 : static_cast<int>(warmupsValue);
    const int samples = correctnessOnly ? 0 : static_cast<int>(samplesValue);
    const int blocks = correctnessOnly ? 0 : static_cast<int>(blocksValue);
    const char* dtypeName = DTypeName(storageType);

    if (passed) {
        passed = WritePreamble(correctnessOutput, "correctness", modeValue.c_str(), dtypeName,
                               dtypeCode, device, rows, width, seed, epsilonValue, atolValue,
                               rtolValue, cores, warmups, samples, blocks) &&
                 std::fprintf(correctnessOutput,
                    "side\tsource_sha256\tdevice\trows\twidth\tdtype\tdtype_code\tmax_abs_error\tmax_relative_error\tatol\trtol\tmismatches\tnonfinite\tstatus\n") >= 0 &&
                 WritePreamble(rawOutput, "raw_timing", modeValue.c_str(), dtypeName,
                               dtypeCode, device, rows, width, seed, epsilonValue, atolValue,
                               rtolValue, cores, warmups, samples, blocks) &&
                 WritePreamble(summaryOutput, "timing_summary", modeValue.c_str(), dtypeName,
                               dtypeCode, device, rows, width, seed, epsilonValue, atolValue,
                               rtolValue, cores, warmups, samples, blocks);
        if (rawOutput != nullptr && passed) {
            passed = std::fprintf(rawOutput,
                "block\tsample\torder\tside\tsource_sha256\tdevice\trows\twidth\tdtype\tdtype_code\tdevice_event_us\twall_us\tnominal_hbm_bytes\teffective_gb_per_s\n") >= 0;
        }
        if (summaryOutput != nullptr && passed) {
            passed = std::fprintf(summaryOutput,
                "record\tcount\tmedian_us\tmean_us\tstdev_us\tcv\tmin_us\tmax_us\tmax_over_min\tmad_us\tp10_us\tp90_us\tspan_us\teffective_gb_per_s\n") >= 0;
        }
    }

    DeviceMetadata beforeMetadata;
    if (passed) {
        beforeMetadata = QueryDeviceMetadata(device);
        WriteDeviceMetadata(correctnessOutput, "BEFORE", beforeMetadata);
        WriteDeviceMetadata(rawOutput, "BEFORE", beforeMetadata);
        WriteDeviceMetadata(summaryOutput, "BEFORE", beforeMetadata);
        passed = std::fflush(correctnessOutput) == 0 && (rawOutput == nullptr || std::fflush(rawOutput) == 0) &&
                 (summaryOutput == nullptr || std::fflush(summaryOutput) == 0);
    }

    bool correctnessPassed = false;
    if (passed) {
        std::vector<uint8_t> actual(inputBytes);
        bool allCorrect = true;
        for (const Kernel* kernel : {&kParent, &kCandidate}) {
            bool callOk = CheckAcl(aclrtMemset(deviceOutput, inputBytes, 0xff, inputBytes), "initialize output to NaN");
            if (callOk) {
                kernel->call(deviceX, inputGroup, deviceResidual, inputGroup,
                             deviceGamma, vectorGroup, deviceBias, vectorGroup,
                             deviceOutput, inputGroup, cores, stream, static_cast<float>(epsilonValue));
                callOk = CheckAcl(aclrtSynchronizeStream(stream), "aclrtSynchronizeStream(correctness)") &&
                         CheckAcl(aclrtMemcpy(actual.data(), inputBytes, deviceOutput, inputBytes,
                                             ACL_MEMCPY_DEVICE_TO_HOST), "copy correctness output");
            }
            if (!callOk || !CompareOutput(correctnessOutput, *kernel, device, storageType,
                                         rows, width, epsilonValue, atolValue, rtolValue,
                                         x, residual, gamma, bias, actual)) {
                allCorrect = false;
                if (!callOk) break;
            }
        }
        correctnessPassed = allCorrect;
        passed = std::fflush(correctnessOutput) == 0;
        if (!correctnessPassed) {
            std::fprintf(stderr, "CPU-golden correctness failed; timing was not run\n");
            if (passed) exitCode = 3;
        }
    }

    if (passed && correctnessPassed && !correctnessOnly) {
        passed = Warmup(kParent, warmups, deviceX, deviceResidual, deviceGamma,
                        deviceBias, deviceOutput, inputGroup, vectorGroup, cores,
                        static_cast<float>(epsilonValue), stream) &&
                 Warmup(kCandidate, warmups, deviceX, deviceResidual, deviceGamma,
                        deviceBias, deviceOutput, inputGroup, vectorGroup, cores,
                        static_cast<float>(epsilonValue), stream);
    }

    std::vector<double> parentSamples;
    std::vector<double> candidateSamples;
    std::vector<double> pairedDeltas;
    for (int block = 0; passed && correctnessPassed && !correctnessOnly && block < blocks; ++block) {
        for (int sample = 0; passed && sample < samples; ++sample) {
            const bool candidateFirst = ((block + sample) % 2) != 0;
            const Kernel& first = candidateFirst ? kCandidate : kParent;
            const Kernel& second = candidateFirst ? kParent : kCandidate;
            const char* firstOrder = candidateFirst ? "CP" : "PC";
            double firstDeviceUs = 0.0, firstWallUs = 0.0;
            passed = LaunchAndMeasure(first, deviceX, deviceResidual, deviceGamma, deviceBias,
                                      deviceOutput, inputGroup, vectorGroup, cores,
                                      static_cast<float>(epsilonValue), stream, startEvent, stopEvent,
                                      firstDeviceUs, firstWallUs) &&
                     WriteSample(rawOutput, first, block + 1, sample + 1, firstOrder,
                                 device, rows, width, dtypeName, dtypeCode, nominalBytes,
                                 firstDeviceUs, firstWallUs);
            if (passed) {
                (candidateFirst ? candidateSamples : parentSamples).push_back(firstDeviceUs);
            }
            double secondDeviceUs = 0.0, secondWallUs = 0.0;
            if (passed) {
                passed = LaunchAndMeasure(second, deviceX, deviceResidual, deviceGamma, deviceBias,
                                          deviceOutput, inputGroup, vectorGroup, cores,
                                          static_cast<float>(epsilonValue), stream, startEvent, stopEvent,
                                          secondDeviceUs, secondWallUs) &&
                         WriteSample(rawOutput, second, block + 1, sample + 1, firstOrder,
                                     device, rows, width, dtypeName, dtypeCode, nominalBytes,
                                     secondDeviceUs, secondWallUs);
                if (passed) {
                    (candidateFirst ? parentSamples : candidateSamples).push_back(secondDeviceUs);
                    pairedDeltas.push_back(candidateFirst ? firstDeviceUs - secondDeviceUs
                                                           : secondDeviceUs - firstDeviceUs);
                }
            }
        }
        if (passed) passed = std::fflush(rawOutput) == 0;
    }

    if (passed && correctnessPassed && !correctnessOnly) {
        passed = WriteTimingSummary(summaryOutput, "parent", parentSamples, nominalBytes) &&
                 WriteTimingSummary(summaryOutput, "candidate", candidateSamples, nominalBytes) &&
                 WritePairSummary(summaryOutput, pairedDeltas) && std::fflush(summaryOutput) == 0;
    }

    if (deviceSelected && passed) {
        const DeviceMetadata afterMetadata = QueryDeviceMetadata(device);
        WriteDeviceMetadata(correctnessOutput, "AFTER", afterMetadata);
        WriteDeviceMetadata(rawOutput, "AFTER", afterMetadata);
        WriteDeviceMetadata(summaryOutput, "AFTER", afterMetadata);
        passed = std::fflush(correctnessOutput) == 0 && (rawOutput == nullptr || std::fflush(rawOutput) == 0) &&
                 (summaryOutput == nullptr || std::fflush(summaryOutput) == 0);
    }

    cleanup();
    if (std::fclose(correctnessOutput) != 0) exitCode = 1;
    if (rawOutput != nullptr && std::fclose(rawOutput) != 0) exitCode = 1;
    if (summaryOutput != nullptr && std::fclose(summaryOutput) != 0) exitCode = 1;
    if (!passed && exitCode == 0) exitCode = 1;
    if (correctnessOnly && !correctnessPassed && exitCode == 0) exitCode = 3;
    return exitCode;
}
