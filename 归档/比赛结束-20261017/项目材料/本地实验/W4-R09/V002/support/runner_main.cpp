#include <acl/acl.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

#include "runner_abi.h"

using GM_ADDR = void*;

extern "C" void run_kernel_parent(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);
extern "C" void run_kernel_candidate(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);

namespace {

constexpr int32_t kFp16 = 1;
constexpr float kEpsilon = 1.0e-5f;
constexpr float kAtol = 1.0e-3f;
constexpr float kRtol = 1.0e-3f;
using KernelCall = void (*)(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);

struct Options {
    std::string mode;
    std::string dtype;
    std::string output;
    int device = -1;
    int64_t rows = 0;
    int64_t width = 0;
    uint32_t blocks = 0;
    int warmups = 5;
    int repeats = 21;
};

struct Runtime {
    bool initialized = false;
    aclrtStream stream = nullptr;
    aclrtEvent startEvent = nullptr;
    aclrtEvent stopEvent = nullptr;
    void* x = nullptr;
    void* residual = nullptr;
    void* gamma = nullptr;
    void* bias = nullptr;
    void* parentOutput = nullptr;
    void* candidateOutput = nullptr;

    ~Runtime()
    {
        if (x != nullptr) aclrtFree(x);
        if (residual != nullptr) aclrtFree(residual);
        if (gamma != nullptr) aclrtFree(gamma);
        if (bias != nullptr) aclrtFree(bias);
        if (parentOutput != nullptr) aclrtFree(parentOutput);
        if (candidateOutput != nullptr) aclrtFree(candidateOutput);
        if (startEvent != nullptr) aclrtDestroyEvent(startEvent);
        if (stopEvent != nullptr) aclrtDestroyEvent(stopEvent);
        if (stream != nullptr) aclrtDestroyStream(stream);
        if (initialized) aclFinalize();
    }
};

struct Sample {
    char order[3]{};
    double parentDeviceUs = 0.0;
    double candidateDeviceUs = 0.0;
    double parentWallUs = 0.0;
    double candidateWallUs = 0.0;
};

struct TensorGroups {
    TensorGroupInfo x;
    TensorGroupInfo residual;
    TensorGroupInfo gamma;
    TensorGroupInfo bias;
    TensorGroupInfo output;
};

bool CheckAcl(aclError status, const char* operation)
{
    if (status == ACL_SUCCESS) return true;
    std::fprintf(stderr, "%s failed: ACL status %d\n", operation, static_cast<int>(status));
    return false;
}

bool ParseInt64(const char* text, int64_t minimum, int64_t maximum, int64_t& value)
{
    errno = 0;
    char* end = nullptr;
    const long long parsed = std::strtoll(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || parsed < minimum || parsed > maximum) return false;
    value = static_cast<int64_t>(parsed);
    return true;
}

void PrintUsage(const char* program)
{
    std::printf("Usage: %s --mode correctness|local|same --device ID --rows M --width D --blocks N --dtype fp16 [options]\n",
                program);
    std::printf("Required dimensions are explicit local-proxy inputs; width must select narrow-mid (128 < D <= 4096).\n");
    std::printf("Options: --warmups N (default 5), --repeats N (default 21), --output FILE\n");
    std::printf("Correctness requires bitwise Parent/Candidate agreement and independent FP64 CPU reference agreement.\n");
    std::printf("Local alternates Parent/Candidate; same uses the Parent function for both timed sides.\n");
}

bool ParseOptions(int argc, char** argv, Options& options)
{
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--mode") == 0 && i + 1 < argc) {
            options.mode = argv[++i];
        } else if (std::strcmp(argv[i], "--dtype") == 0 && i + 1 < argc) {
            options.dtype = argv[++i];
        } else if (std::strcmp(argv[i], "--output") == 0 && i + 1 < argc) {
            options.output = argv[++i];
        } else if (std::strcmp(argv[i], "--device") == 0 && i + 1 < argc) {
            int64_t parsed = 0;
            if (!ParseInt64(argv[++i], 0, 255, parsed)) return false;
            options.device = static_cast<int>(parsed);
        } else if (std::strcmp(argv[i], "--rows") == 0 && i + 1 < argc) {
            if (!ParseInt64(argv[++i], 1, std::numeric_limits<int64_t>::max(), options.rows)) return false;
        } else if (std::strcmp(argv[i], "--width") == 0 && i + 1 < argc) {
            if (!ParseInt64(argv[++i], 129, 4096, options.width)) return false;
        } else if (std::strcmp(argv[i], "--blocks") == 0 && i + 1 < argc) {
            int64_t parsed = 0;
            if (!ParseInt64(argv[++i], 1, std::numeric_limits<uint32_t>::max(), parsed)) return false;
            options.blocks = static_cast<uint32_t>(parsed);
        } else if (std::strcmp(argv[i], "--warmups") == 0 && i + 1 < argc) {
            int64_t parsed = 0;
            if (!ParseInt64(argv[++i], 0, 100000, parsed)) return false;
            options.warmups = static_cast<int>(parsed);
        } else if (std::strcmp(argv[i], "--repeats") == 0 && i + 1 < argc) {
            int64_t parsed = 0;
            if (!ParseInt64(argv[++i], 1, 100000, parsed)) return false;
            options.repeats = static_cast<int>(parsed);
        } else {
            return false;
        }
    }
    return options.mode == "correctness" || options.mode == "local" || options.mode == "same";
}

uint16_t ToFp16(float value)
{
    const aclFloat16 halfValue = aclFloatToFloat16(value);
    uint16_t bits = 0;
    std::memcpy(&bits, &halfValue, sizeof(bits));
    return bits;
}

float FromFp16(uint16_t bits)
{
    aclFloat16 halfValue;
    std::memcpy(&halfValue, &bits, sizeof(bits));
    return aclFloat16ToFloat(halfValue);
}

float InputValue(uint64_t index, uint32_t multiplier, uint32_t offset)
{
    const uint64_t residue = ((index % 2047U) * multiplier + offset) % 2047U;
    return static_cast<float>(static_cast<int64_t>(residue) - 1023) / 1024.0f;
}

void CallKernel(KernelCall kernel, const Runtime& runtime,
                const TensorGroups& groups, const Options& options, void* output)
{
    kernel(runtime.x, groups.x, runtime.residual, groups.residual,
           runtime.gamma, groups.gamma, runtime.bias, groups.bias,
           output, groups.output, static_cast<int64_t>(options.blocks), runtime.stream, kEpsilon);
}

bool SynchronizeKernel(KernelCall kernel, const Runtime& runtime,
                       const TensorGroups& groups, const Options& options, void* output)
{
    CallKernel(kernel, runtime, groups, options, output);
    return CheckAcl(aclrtSynchronizeStream(runtime.stream), "aclrtSynchronizeStream");
}

bool CompareOutputs(const Runtime& runtime, const Options& options,
                    const std::vector<uint16_t>& parent, const std::vector<uint16_t>& candidate)
{
    const size_t elements = parent.size();
    size_t bitDifferences = 0;
    size_t toleranceFailures = 0;
    size_t nonfinite = 0;
    float maxAbs = 0.0f;
    for (size_t i = 0; i < elements; ++i) {
        if (parent[i] != candidate[i]) ++bitDifferences;
        const float expected = FromFp16(parent[i]);
        const float actual = FromFp16(candidate[i]);
        if (!std::isfinite(expected) || !std::isfinite(actual)) {
            ++nonfinite;
            ++toleranceFailures;
            continue;
        }
        const float error = std::fabs(actual - expected);
        maxAbs = std::max(maxAbs, error);
        if (error > kAtol + kRtol * std::fabs(expected)) ++toleranceFailures;
    }
    const bool passed = toleranceFailures == 0 && bitDifferences == 0;
    std::printf("CORRECTNESS proxy_only=1 parent=W4-R09-V001 candidate=W4-R09-V002 input=deterministic_fp16_pattern_v1 epsilon=%.8g device=%d rows=%lld width=%lld dtype=fp16 blocks=%u bit_differences=%zu max_abs=%.9g tolerance_failures=%zu nonfinite=%zu result=%s\n",
                kEpsilon,
                options.device, static_cast<long long>(options.rows),
                static_cast<long long>(options.width), options.blocks,
                bitDifferences, maxAbs, toleranceFailures, nonfinite, passed ? "PASS" : "FAIL");
    return passed;
}

std::vector<float> Reference(const Options& options,
                             const std::vector<uint16_t>& x,
                             const std::vector<uint16_t>& residual,
                             const std::vector<uint16_t>& gamma,
                             const std::vector<uint16_t>& bias)
{
    std::vector<float> expected(x.size());
    for (int64_t row = 0; row < options.rows; ++row) {
        double squareSum = 0.0;
        for (int64_t col = 0; col < options.width; ++col) {
            const size_t index = static_cast<size_t>(row * options.width + col);
            const double value = static_cast<double>(FromFp16(x[index])) + FromFp16(residual[index]);
            squareSum += value * value;
        }
        const double invRms = 1.0 / std::sqrt(squareSum / options.width + kEpsilon);
        for (int64_t col = 0; col < options.width; ++col) {
            const size_t index = static_cast<size_t>(row * options.width + col);
            const double value = static_cast<double>(FromFp16(x[index])) + FromFp16(residual[index]);
            const double affine = value * invRms * FromFp16(gamma[static_cast<size_t>(col)]) +
                                  FromFp16(bias[static_cast<size_t>(col)]);
            expected[index] = FromFp16(ToFp16(static_cast<float>(affine)));
        }
    }
    return expected;
}

bool CompareReference(const char* side, const Options& options,
                      const std::vector<float>& reference,
                      const std::vector<uint16_t>& actual)
{
    constexpr float atol = 1.0f / 512.0f;
    constexpr float rtol = 1.0f / 512.0f;
    size_t failures = 0, nonfinite = 0;
    float maxAbs = 0.0f;
    for (size_t i = 0; i < reference.size(); ++i) {
        const float want = reference[i], got = FromFp16(actual[i]);
        if (!std::isfinite(want) || !std::isfinite(got)) {
            ++failures;
            ++nonfinite;
            continue;
        }
        const float error = std::fabs(got - want);
        maxAbs = std::max(maxAbs, error);
        if (error > atol + rtol * std::fabs(want)) ++failures;
    }
    const bool passed = failures == 0 && maxAbs <= 0.1f;
    std::printf("REFERENCE side=%s reference=fp64_cpu_final_fp16 rows=%lld width=%lld dtype=fp16 max_abs=%.9g atol=%.9g rtol=%.9g failures=%zu elements=%zu nonfinite=%zu result=%s\n",
                side, static_cast<long long>(options.rows), static_cast<long long>(options.width),
                maxAbs, atol, rtol, failures, reference.size(), nonfinite, passed ? "PASS" : "FAIL");
    return passed;
}

bool CopyOutputs(const Runtime& runtime, size_t bytes,
                 std::vector<uint16_t>& parent, std::vector<uint16_t>& candidate)
{
    if (!CheckAcl(aclrtMemcpy(parent.data(), bytes, runtime.parentOutput, bytes,
                              ACL_MEMCPY_DEVICE_TO_HOST), "copy Parent output")) return false;
    return CheckAcl(aclrtMemcpy(candidate.data(), bytes, runtime.candidateOutput, bytes,
                                ACL_MEMCPY_DEVICE_TO_HOST), "copy Candidate output");
}

bool RunCorrectness(const Runtime& runtime, const Options& options,
                    const TensorGroups& groups, size_t outputBytes,
                    std::vector<uint16_t>& parent, std::vector<uint16_t>& candidate,
                    int warmups)
{
    for (int i = 0; i < warmups; ++i) {
        if (!SynchronizeKernel(run_kernel_parent, runtime, groups, options, runtime.parentOutput) ||
            !SynchronizeKernel(run_kernel_candidate, runtime, groups, options, runtime.candidateOutput)) return false;
    }
    if (!SynchronizeKernel(run_kernel_parent, runtime, groups, options, runtime.parentOutput) ||
        !SynchronizeKernel(run_kernel_candidate, runtime, groups, options, runtime.candidateOutput) ||
        !CopyOutputs(runtime, outputBytes, parent, candidate)) return false;
    return CompareOutputs(runtime, options, parent, candidate);
}

bool Measure(KernelCall kernel, const Runtime& runtime,
             const TensorGroups& groups, const Options& options, void* output,
             double& deviceUs, double& wallUs)
{
    const auto wallStart = std::chrono::steady_clock::now();
    if (!CheckAcl(aclrtRecordEvent(runtime.startEvent, runtime.stream), "record start event")) return false;
    CallKernel(kernel, runtime, groups, options, output);
    if (!CheckAcl(aclrtRecordEvent(runtime.stopEvent, runtime.stream), "record stop event") ||
        !CheckAcl(aclrtSynchronizeEvent(runtime.stopEvent), "synchronize stop event")) return false;
    const auto wallStop = std::chrono::steady_clock::now();
    float elapsedMs = 0.0f;
    if (!CheckAcl(aclrtEventElapsedTime(&elapsedMs, runtime.startEvent, runtime.stopEvent),
                  "read device-event elapsed time")) return false;
    deviceUs = static_cast<double>(elapsedMs) * 1000.0;
    wallUs = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(
                   wallStop - wallStart).count()) / 1000.0;
    return true;
}

double Quantile(std::vector<double> values, double q)
{
    std::sort(values.begin(), values.end());
    const size_t index = static_cast<size_t>(std::ceil(q * values.size())) - 1;
    return values[std::min(index, values.size() - 1)];
}

double Median(std::vector<double> values)
{
    std::sort(values.begin(), values.end());
    const size_t middle = values.size() / 2;
    return values.size() % 2 == 0 ? (values[middle - 1] + values[middle]) / 2.0 : values[middle];
}

bool RunLocal(const Runtime& runtime, const Options& options,
              const TensorGroups& groups)
{
    const KernelCall measuredCandidate = options.mode == "same" ? run_kernel_parent : run_kernel_candidate;
    const char* measuredName = options.mode == "same" ? "W4-R09-V001_same_function" : "W4-R09-V002";
    std::vector<Sample> samples(static_cast<size_t>(options.repeats));
    for (int i = 0; i < options.warmups; ++i) {
        const bool parentFirst = (i & 1) == 0;
        const KernelCall first = parentFirst ? run_kernel_parent : measuredCandidate;
        const KernelCall second = parentFirst ? measuredCandidate : run_kernel_parent;
        void* firstOutput = parentFirst ? runtime.parentOutput : runtime.candidateOutput;
        void* secondOutput = parentFirst ? runtime.candidateOutput : runtime.parentOutput;
        if (!SynchronizeKernel(first, runtime, groups, options, firstOutput) ||
            !SynchronizeKernel(second, runtime, groups, options, secondOutput)) return false;
    }

    for (int i = 0; i < options.repeats; ++i) {
        Sample& sample = samples[static_cast<size_t>(i)];
        const bool parentFirst = (i & 1) == 0;
        std::strcpy(sample.order, parentFirst ? "PC" : "CP");
        if (parentFirst) {
            if (!Measure(run_kernel_parent, runtime, groups, options, runtime.parentOutput,
                         sample.parentDeviceUs, sample.parentWallUs) ||
                !Measure(measuredCandidate, runtime, groups, options, runtime.candidateOutput,
                         sample.candidateDeviceUs, sample.candidateWallUs)) return false;
        } else {
            if (!Measure(measuredCandidate, runtime, groups, options, runtime.candidateOutput,
                         sample.candidateDeviceUs, sample.candidateWallUs) ||
                !Measure(run_kernel_parent, runtime, groups, options, runtime.parentOutput,
                         sample.parentDeviceUs, sample.parentWallUs)) return false;
        }
    }

    std::ofstream file;
    std::ostream* output = &std::cout;
    if (!options.output.empty()) {
        file.open(options.output);
        if (!file) {
            std::fprintf(stderr, "cannot open output file: %s\n", options.output.c_str());
            return false;
        }
        output = &file;
    }
    *output << "# LOCAL_PROXY_ONLY parent=W4-R09-V001 candidate=" << measuredName << " input=deterministic_fp16_pattern_v1"
            << " epsilon=" << kEpsilon << " order=alternating_PC_CP device_load=EXTERNAL_NOT_CAPTURED device=" << options.device
            << " rows=" << options.rows << " width=" << options.width << " dtype=fp16"
            << " requested_blocks=" << options.blocks << " effective_blocks="
            << std::min<uint64_t>(options.blocks, static_cast<uint64_t>(options.rows))
            << " warmups=" << options.warmups << " paired_samples=" << options.repeats << '\n';
    *output << "sample\torder\tparent_device_us\tcandidate_device_us\tdelta_device_us"
               "\tparent_wall_us\tcandidate_wall_us\tdelta_wall_us\n";
    *output << std::fixed << std::setprecision(6);

    std::vector<double> parentDevice, candidateDevice, deltaDevice;
    std::vector<double> parentWall, candidateWall, deltaWall, percentDelta;
    parentDevice.reserve(samples.size());
    candidateDevice.reserve(samples.size());
    deltaDevice.reserve(samples.size());
    parentWall.reserve(samples.size());
    candidateWall.reserve(samples.size());
    deltaWall.reserve(samples.size());
    percentDelta.reserve(samples.size());
    for (size_t i = 0; i < samples.size(); ++i) {
        const Sample& sample = samples[i];
        const double deviceDelta = sample.candidateDeviceUs - sample.parentDeviceUs;
        const double wallDelta = sample.candidateWallUs - sample.parentWallUs;
        *output << i << '\t' << sample.order << '\t' << sample.parentDeviceUs << '\t'
                << sample.candidateDeviceUs << '\t' << deviceDelta << '\t'
                << sample.parentWallUs << '\t' << sample.candidateWallUs << '\t' << wallDelta << '\n';
        parentDevice.push_back(sample.parentDeviceUs);
        candidateDevice.push_back(sample.candidateDeviceUs);
        deltaDevice.push_back(deviceDelta);
        parentWall.push_back(sample.parentWallUs);
        candidateWall.push_back(sample.candidateWallUs);
        deltaWall.push_back(wallDelta);
        percentDelta.push_back(sample.parentDeviceUs > 0.0
            ? deviceDelta * 100.0 / sample.parentDeviceUs : 0.0);
    }
    output->flush();
    if (!*output) {
        std::fprintf(stderr, "failed while writing local samples\n");
        return false;
    }

    const uint64_t effectiveBlocks = std::min<uint64_t>(options.blocks, static_cast<uint64_t>(options.rows));
    std::printf("LOCAL_PROXY_ONLY parent=W4-R09-V001 candidate=%s input=deterministic_fp16_pattern_v1 epsilon=%.8g order=alternating_PC_CP device_load=EXTERNAL_NOT_CAPTURED device=%d rows=%lld width=%lld dtype=fp16 requested_blocks=%u effective_blocks=%llu min_rows_per_block=%lld max_rows_per_block=%lld warmups=%d paired_samples=%d\n",
                measuredName, kEpsilon,
                options.device, static_cast<long long>(options.rows),
                static_cast<long long>(options.width), options.blocks,
                static_cast<unsigned long long>(effectiveBlocks),
                static_cast<long long>(options.rows / static_cast<int64_t>(effectiveBlocks)),
                static_cast<long long>((options.rows + static_cast<int64_t>(effectiveBlocks) - 1) /
                                       static_cast<int64_t>(effectiveBlocks)),
                options.warmups, options.repeats);
    const double deviceP05Parent = Quantile(parentDevice, 0.05);
    const double deviceP95Parent = Quantile(parentDevice, 0.95);
    const double deviceP05Candidate = Quantile(candidateDevice, 0.05);
    const double deviceP95Candidate = Quantile(candidateDevice, 0.95);
    const double wallP05Parent = Quantile(parentWall, 0.05);
    const double wallP95Parent = Quantile(parentWall, 0.95);
    const double wallP05Candidate = Quantile(candidateWall, 0.05);
    const double wallP95Candidate = Quantile(candidateWall, 0.95);
    std::printf("DEVICE_US median_parent=%.6f median_candidate=%.6f median_paired_delta=%.6f median_delta_pct=%.4f p05_parent=%.6f p95_parent=%.6f spread90_parent=%.6f p05_candidate=%.6f p95_candidate=%.6f spread90_candidate=%.6f\n",
                Median(parentDevice), Median(candidateDevice), Median(deltaDevice), Median(percentDelta),
                deviceP05Parent, deviceP95Parent, deviceP95Parent - deviceP05Parent,
                deviceP05Candidate, deviceP95Candidate, deviceP95Candidate - deviceP05Candidate);
    std::printf("WALL_US median_parent=%.6f median_candidate=%.6f median_paired_delta=%.6f p05_parent=%.6f p95_parent=%.6f spread90_parent=%.6f p05_candidate=%.6f p95_candidate=%.6f spread90_candidate=%.6f\n",
                Median(parentWall), Median(candidateWall), Median(deltaWall),
                wallP05Parent, wallP95Parent, wallP95Parent - wallP05Parent,
                wallP05Candidate, wallP95Candidate, wallP95Candidate - wallP05Candidate);
    return true;
}

}  // namespace

int main(int argc, char** argv)
{
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--help") == 0 || std::strcmp(argv[i], "-h") == 0) {
            PrintUsage(argv[0]);
            return 0;
        }
    }

    Options options;
    if (!ParseOptions(argc, argv, options) || options.device < 0 || options.rows <= 0 ||
        options.width <= 128 || options.width > 4096 || options.blocks == 0 || options.dtype != "fp16") {
        PrintUsage(argv[0]);
        return 2;
    }
    if (options.rows > std::numeric_limits<int64_t>::max() / options.width) {
        std::fprintf(stderr, "rows*width exceeds the supported element count\n");
        return 2;
    }
    const uint64_t elements64 = static_cast<uint64_t>(options.rows) * static_cast<uint64_t>(options.width);
    if (elements64 > std::numeric_limits<size_t>::max() / sizeof(uint16_t)) {
        std::fprintf(stderr, "requested tensor does not fit host address space\n");
        return 2;
    }
    const size_t elements = static_cast<size_t>(elements64);
    const size_t dataBytes = elements * sizeof(uint16_t);
    const size_t paramBytes = static_cast<size_t>(options.width) * sizeof(uint16_t);

    Runtime runtime;
    if (!CheckAcl(aclInit(nullptr), "aclInit")) return 1;
    runtime.initialized = true;
    if (!CheckAcl(aclrtSetDevice(options.device), "aclrtSetDevice") ||
        !CheckAcl(aclrtCreateStream(&runtime.stream), "aclrtCreateStream") ||
        !CheckAcl(aclrtCreateEvent(&runtime.startEvent), "aclrtCreateEvent(start)") ||
        !CheckAcl(aclrtCreateEvent(&runtime.stopEvent), "aclrtCreateEvent(stop)")) return 1;

    if (!CheckAcl(aclrtMalloc(&runtime.x, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "allocate x") ||
        !CheckAcl(aclrtMalloc(&runtime.residual, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "allocate residual") ||
        !CheckAcl(aclrtMalloc(&runtime.gamma, paramBytes, ACL_MEM_MALLOC_HUGE_FIRST), "allocate gamma") ||
        !CheckAcl(aclrtMalloc(&runtime.bias, paramBytes, ACL_MEM_MALLOC_HUGE_FIRST), "allocate bias") ||
        !CheckAcl(aclrtMalloc(&runtime.parentOutput, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "allocate Parent output") ||
        !CheckAcl(aclrtMalloc(&runtime.candidateOutput, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "allocate Candidate output")) return 1;

    std::vector<uint16_t> hostX(elements), hostResidual(elements), hostGamma(static_cast<size_t>(options.width));
    std::vector<uint16_t> hostBias(static_cast<size_t>(options.width));
    std::vector<uint16_t> parentOutput(elements), candidateOutput(elements);
    for (size_t i = 0; i < elements; ++i) {
        hostX[i] = ToFp16(InputValue(i, 37, 11));
        hostResidual[i] = ToFp16(InputValue(i, 17, 3));
    }
    for (int64_t i = 0; i < options.width; ++i) {
        hostGamma[static_cast<size_t>(i)] = ToFp16(0.75f + static_cast<float>((i * 13) % 100) / 200.0f);
        hostBias[static_cast<size_t>(i)] = ToFp16(static_cast<float>((i * 7) % 100) / 400.0f - 0.125f);
    }
    if (!CheckAcl(aclrtMemcpy(runtime.x, dataBytes, hostX.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy x") ||
        !CheckAcl(aclrtMemcpy(runtime.residual, dataBytes, hostResidual.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy residual") ||
        !CheckAcl(aclrtMemcpy(runtime.gamma, paramBytes, hostGamma.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma") ||
        !CheckAcl(aclrtMemcpy(runtime.bias, paramBytes, hostBias.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy bias")) return 1;

    int64_t inputShape[] = {options.rows, options.width};
    int64_t vectorShape[] = {options.width};
    TensorInfo xInfo[] = {{inputShape, 2, kFp16}};
    TensorInfo residualInfo[] = {{inputShape, 2, kFp16}};
    TensorInfo gammaInfo[] = {{vectorShape, 1, kFp16}};
    TensorInfo biasInfo[] = {{vectorShape, 1, kFp16}};
    TensorInfo outputInfo[] = {{inputShape, 2, kFp16}};
    TensorGroups groups{
        {xInfo, 1}, {residualInfo, 1}, {gammaInfo, 1}, {biasInfo, 1}, {outputInfo, 1}
    };

    const bool correctnessOk = RunCorrectness(
        runtime, options, groups, dataBytes,
        parentOutput, candidateOutput, options.mode == "correctness" ? 2 : 1);
    if (!correctnessOk) return 3;
    const std::vector<float> reference = Reference(options, hostX, hostResidual, hostGamma, hostBias);
    const bool parentReferenceOk = CompareReference("parent", options, reference, parentOutput);
    const bool candidateReferenceOk = CompareReference("candidate", options, reference, candidateOutput);
    if (!parentReferenceOk || !candidateReferenceOk) return 3;
    if (options.mode == "correctness") return 0;
    if (!RunLocal(runtime, options, groups)) return 1;
    return 0;
}
