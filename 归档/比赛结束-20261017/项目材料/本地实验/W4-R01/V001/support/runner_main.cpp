#include <acl/acl.h>
#include <dlfcn.h>
#include <unistd.h>

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
constexpr double kAllowedMismatchFraction = 0.001;
using KernelCall = void (*)(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);

struct Options {
    std::string mode;
    std::string dtype;
    std::string output;
    std::string comparison = "parent-candidate";
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
    char previousSlot = '?';
    int cycle = -1;
    int position = 0;
    bool swapped = false;
    double parentDeviceUs = std::numeric_limits<double>::quiet_NaN();
    double candidateDeviceUs = std::numeric_limits<double>::quiet_NaN();
    double parentWallUs = std::numeric_limits<double>::quiet_NaN();
    double candidateWallUs = std::numeric_limits<double>::quiet_NaN();
    bool parentCompleted = false;
    bool candidateCompleted = false;
};

struct TensorGroups {
    TensorGroupInfo x;
    TensorGroupInfo residual;
    TensorGroupInfo gamma;
    TensorGroupInfo bias;
    TensorGroupInfo output;
};

struct CallMetadata {
    const char* phase = "reference";
    const char* comparison = "PC";
    int phaseIndex = 0;
    int group = 0;
    int cycle = -1;
    int pair = -1;
    int sequenceIndex = -1;
    int position = 0;
    char slot = '?';
    bool swapped = false;
};

struct ActualCall {
    CallMetadata metadata;
    KernelCall function;
    KernelCall previousFunction;
    void* output;
    void* previousOutput;
    void* x;
    void* residual;
    void* gamma;
    void* bias;
    char previousSlot;
};

struct CallTrace;
CallTrace* activeTrace = nullptr;

struct CallTrace {
    Options options;
    std::vector<ActualCall> calls;
    CallMetadata current;
    bool enabled;
    bool saved = false;

    explicit CallTrace(const Options& source) : options(source), enabled(source.mode == "task-compare")
    {
        if (enabled) {
            calls.reserve(1300);
            activeTrace = this;
        }
    }

    bool Save()
    {
        if (!enabled || saved) return true;
        const std::string path = options.output + "-r16-d" + std::to_string(options.width) + "-calls.tsv";
        std::ofstream out(path);
        out << "ordinal\tpid\trows\twidth\tdevice\tphase\tcomparison\tphase_index\tgroup\tcycle\tpair"
               "\tsequence_index\tposition\tslot\tprevious_slot\tswapped\tfunction_address"
               "\tprevious_function_address\toutput_address\tprevious_output_address\tx\tresidual\tgamma\tbias\n";
        for (size_t i = 0; i < calls.size(); ++i) {
            const ActualCall& call = calls[i];
            const CallMetadata& meta = call.metadata;
            out << i + 1 << '\t' << getpid() << '\t' << options.rows << '\t' << options.width
                << '\t' << options.device << '\t' << meta.phase << '\t' << meta.comparison
                << '\t' << meta.phaseIndex << '\t' << meta.group << '\t' << meta.cycle << '\t' << meta.pair
                << '\t' << meta.sequenceIndex << '\t' << meta.position << '\t' << meta.slot
                << '\t' << call.previousSlot << '\t' << meta.swapped
                << '\t' << reinterpret_cast<void*>(call.function)
                << '\t' << reinterpret_cast<void*>(call.previousFunction) << '\t' << call.output
                << '\t' << call.previousOutput << '\t' << call.x << '\t' << call.residual
                << '\t' << call.gamma << '\t' << call.bias << '\n';
        }
        out.flush();
        saved = static_cast<bool>(out);
        std::printf("CALL_TRACE rows=%lld width=%lld calls=%zu file=%s result=%s\n",
                    static_cast<long long>(options.rows), static_cast<long long>(options.width),
                    calls.size(), path.c_str(), saved ? "PASS" : "FAIL");
        return saved;
    }

    ~CallTrace()
    {
        if (enabled) {
            Save();
            activeTrace = nullptr;
        }
    }
};

void RecordCall(KernelCall kernel, const Runtime& runtime, void* output)
{
    if (activeTrace == nullptr) return;
    CallMetadata metadata = activeTrace->current;
    if (metadata.slot == '?') metadata.slot = output == runtime.parentOutput ? 'P' : 'C';
    const ActualCall* previous = activeTrace->calls.empty() ? nullptr : &activeTrace->calls.back();
    activeTrace->calls.push_back({metadata, kernel, previous == nullptr ? nullptr : previous->function,
        output, previous == nullptr ? nullptr : previous->output,
        runtime.x, runtime.residual, runtime.gamma, runtime.bias,
        previous == nullptr ? '?' : previous->metadata.slot});
}

KernelCall SecondKernel(const Options& options)
{
    return options.comparison == "parent-parent" ? run_kernel_parent : run_kernel_candidate;
}

const char* SecondLabel(const Options& options)
{
    return options.comparison == "parent-parent" ? "R31B_V011-same-function" : "W4-R01-V001";
}

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
    std::printf("Usage: %s --mode correctness|local|parent-diagnostic|task-study|task-compare --device ID --rows M --width D --blocks N --dtype fp16 [options]\n",
                program);
    std::printf("Required dimensions are explicit local-proxy inputs; width must select the wide path (D > 8192).\n");
    std::printf("Options: --warmups N (default 5), --repeats N (default 21), --output FILE\n");
    std::printf("--comparison parent-candidate (default) or parent-parent (one Parent function, two outputs).\n");
    std::printf("parent-diagnostic requires parent-parent and records reference failures without timing Candidate.\n");
    std::printf("task-study uses Parent only: 16x16384 then 16x32768, blocks=8, warmups=45, repeats=42, three groups each, one process.\n");
    std::printf("task-study requires an output prefix and preserves both output addresses within each input.\n");
    std::printf("task-compare uses the declared device0 PP/PC design: two inputs, four groups, 45 warmup pairs, 32 matched pairs per phase.\n");
    std::printf("Correctness compares both outputs with an independent CPU reference before Local,\n");
    std::printf("using atol=rtol=1e-3 and the existing template's 0.1%% mismatch allowance; strict counts remain visible.\n");
    std::printf("then alternates Parent/Candidate order and reports raw device-event and wall-time samples.\n");
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
        } else if (std::strcmp(argv[i], "--comparison") == 0 && i + 1 < argc) {
            options.comparison = argv[++i];
        } else if (std::strcmp(argv[i], "--device") == 0 && i + 1 < argc) {
            int64_t parsed = 0;
            if (!ParseInt64(argv[++i], 0, 255, parsed)) return false;
            options.device = static_cast<int>(parsed);
        } else if (std::strcmp(argv[i], "--rows") == 0 && i + 1 < argc) {
            if (!ParseInt64(argv[++i], 1, std::numeric_limits<int64_t>::max(), options.rows)) return false;
        } else if (std::strcmp(argv[i], "--width") == 0 && i + 1 < argc) {
            if (!ParseInt64(argv[++i], 8193, std::numeric_limits<int32_t>::max(), options.width)) return false;
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
    if (options.mode == "parent-diagnostic" || options.mode == "task-study") {
        return options.comparison == "parent-parent";
    }
    if (options.mode == "task-compare") return options.comparison == "parent-candidate";
    return (options.mode == "correctness" || options.mode == "local") &&
           (options.comparison == "parent-candidate" || options.comparison == "parent-parent");
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

std::vector<float> CpuReference(const Options& options,
                              const std::vector<uint16_t>& x,
                              const std::vector<uint16_t>& residual,
                              const std::vector<uint16_t>& gamma,
                              const std::vector<uint16_t>& bias)
{
    // Evaluate the problem formula independently in FP64, rounding only the output to FP16.
    std::vector<float> expected(x.size());
    const size_t width = static_cast<size_t>(options.width);
    for (size_t row = 0; row < static_cast<size_t>(options.rows); ++row) {
        double squareSum = 0.0;
        for (size_t col = 0; col < width; ++col) {
            const size_t index = row * width + col;
            const double value = static_cast<double>(FromFp16(x[index])) + FromFp16(residual[index]);
            squareSum += value * value;
        }
        const double rms = std::sqrt(squareSum / static_cast<double>(width) + kEpsilon);
        for (size_t col = 0; col < width; ++col) {
            const size_t index = row * width + col;
            const double value = static_cast<double>(FromFp16(x[index])) + FromFp16(residual[index]);
            const double result = value / rms * FromFp16(gamma[col]) + FromFp16(bias[col]);
            expected[index] = FromFp16(ToFp16(static_cast<float>(result)));
        }
    }
    return expected;
}

bool CompareReference(const char* label, const Options& options,
                      const std::vector<uint16_t>& actual, const std::vector<float>& expected)
{
    size_t failures = 0;
    size_t nonfinite = 0;
    float maxAbs = 0.0f;
    for (size_t index = 0; index < expected.size(); ++index) {
        const float value = FromFp16(actual[index]);
        const float error = std::fabs(value - expected[index]);
        maxAbs = std::max(maxAbs, error);
        const bool finite = std::isfinite(value) && std::isfinite(expected[index]);
        if (!finite) ++nonfinite;
        if (!finite || error > kAtol + kRtol * std::fabs(expected[index])) {
            if (failures < 8) {
                std::printf("REFERENCE_FAILURE label=%s index=%zu actual=%.9g expected=%.9g abs_error=%.9g\n",
                            label, index, value, expected[index], error);
            }
            ++failures;
        }
    }
    const double mismatchFraction = static_cast<double>(failures) / expected.size();
    const bool passed = nonfinite == 0 && mismatchFraction <= kAllowedMismatchFraction;
    std::printf("REFERENCE label=%s implementation=cpu_fp64_formula_final_fp16 device=%d rows=%lld width=%lld dtype=fp16 blocks=%u atol=%.9g rtol=%.9g elements=%zu tolerance_failures=%zu nonfinite=%zu max_abs=%.9g mismatch_fraction=%.12g allowed_mismatch_fraction=%.12g strict_all_elements=%s result=%s\n",
                label, options.device, static_cast<long long>(options.rows),
                static_cast<long long>(options.width), options.blocks, kAtol, kRtol,
                expected.size(), failures, nonfinite, maxAbs, mismatchFraction,
                kAllowedMismatchFraction, failures == 0 ? "PASS" : "FAIL", passed ? "PASS" : "FAIL");
    return passed;
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
    RecordCall(kernel, runtime, output);
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
    const bool passed = toleranceFailures == 0;
    std::printf("CORRECTNESS proxy_only=1 parent=R31B_V011 candidate=%s input=deterministic_fp16_pattern_v1 epsilon=%.8g device=%d rows=%lld width=%lld dtype=fp16 blocks=%u bit_differences=%zu max_abs=%.9g tolerance_failures=%zu nonfinite=%zu result=%s\n",
                SecondLabel(options), kEpsilon,
                options.device, static_cast<long long>(options.rows),
                static_cast<long long>(options.width), options.blocks,
                bitDifferences, maxAbs, toleranceFailures, nonfinite, passed ? "PASS" : "FAIL");
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
            !SynchronizeKernel(SecondKernel(options), runtime, groups, options, runtime.candidateOutput)) return false;
    }
    if (!SynchronizeKernel(run_kernel_parent, runtime, groups, options, runtime.parentOutput) ||
        !SynchronizeKernel(SecondKernel(options), runtime, groups, options, runtime.candidateOutput) ||
        !CopyOutputs(runtime, outputBytes, parent, candidate)) return false;
    return CompareOutputs(runtime, options, parent, candidate);
}

bool Measure(KernelCall kernel, const Runtime& runtime,
             const TensorGroups& groups, const Options& options, void* output,
             double& deviceUs, double& wallUs)
{
    RecordCall(kernel, runtime, output);
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

bool RunBalancedCalls(const Runtime& runtime, const Options& options,
                      const TensorGroups& groups, std::vector<Sample>& samples)
{
    auto kernelFor = [&](char slot) { return slot == 'P' ? run_kernel_parent : SecondKernel(options); };
    auto outputFor = [&](char slot, bool swapped) {
        return ((slot == 'P') != swapped) ? runtime.parentOutput : runtime.candidateOutput;
    };
    CallMetadata& meta = activeTrace->current;
    for (int i = 0; i < options.warmups; ++i) {
        const char* order = (i & 1) == 0 ? "PC" : "CP";
        for (int position = 0; position < 2; ++position) {
            meta.phase = "warmup";
            meta.pair = i;
            meta.cycle = -1;
            meta.sequenceIndex = i * 2 + position;
            meta.position = position + 1;
            meta.slot = order[position];
            meta.swapped = false;
            if (!SynchronizeKernel(kernelFor(meta.slot), runtime, groups, options, outputFor(meta.slot, false))) {
                samples.clear();
                return false;
            }
        }
    }
    for (int cycle = 0; cycle < 8; ++cycle) {
        const char* sequence = (cycle & 1) == 0 ? "PPPCCPCC" : "CCCPPCPP";
        const bool swapped = ((cycle / 2) & 1) != 0;
        meta.phase = "prime";
        meta.cycle = cycle;
        meta.pair = -1;
        meta.sequenceIndex = -1;
        meta.position = 0;
        meta.slot = sequence[7];
        meta.swapped = swapped;
        const KernelCall previousCycleKernel = kernelFor(meta.slot);
        void* previousCycleOutput = outputFor(meta.slot, swapped);
        if (!SynchronizeKernel(kernelFor(meta.slot), runtime, groups, options, outputFor(meta.slot, swapped))) {
            samples.resize(static_cast<size_t>(cycle * 4));
            return false;
        }
        for (int index = 0; index < 8; ++index) {
            const char slot = sequence[index];
            const char previousSlot = sequence[(index + 7) % 8];
            const int position = index % 2 + 1;
            const int pair = cycle * 4 + (previousSlot == 'P' ? 0 : 2) + position - 1;
            Sample& sample = samples[static_cast<size_t>(pair)];
            if (!sample.parentCompleted && !sample.candidateCompleted) {
                std::strcpy(sample.order, previousSlot == 'P' ? "P" : "C");
                sample.previousSlot = previousSlot;
                sample.cycle = cycle;
                sample.position = position;
                sample.swapped = swapped;
            }
            meta.phase = "timed";
            meta.pair = pair;
            meta.sequenceIndex = index;
            meta.position = position;
            meta.slot = slot;
            meta.comparison = options.comparison == "parent-parent" ? "PP" : "PC";
            double& deviceUs = slot == 'P' ? sample.parentDeviceUs : sample.candidateDeviceUs;
            double& wallUs = slot == 'P' ? sample.parentWallUs : sample.candidateWallUs;
            bool& completed = slot == 'P' ? sample.parentCompleted : sample.candidateCompleted;
            const KernelCall currentKernel = kernelFor(slot);
            void* currentOutput = outputFor(slot, swapped);
            completed = Measure(currentKernel, runtime, groups, options, currentOutput, deviceUs, wallUs);
            if (activeTrace != nullptr && !activeTrace->calls.empty()) {
                ActualCall& record = activeTrace->calls.back();
                record.metadata.phaseIndex = activeTrace->current.phaseIndex;
                record.metadata.comparison = meta.comparison;
                record.previousFunction = index == 0 ? previousCycleKernel : kernelFor(previousSlot);
                record.previousOutput = index == 0 ? previousCycleOutput : outputFor(previousSlot, swapped);
                record.previousSlot = previousSlot;
                record.metadata.cycle = cycle;
                record.metadata.group = activeTrace->current.group;
            }
            if (!completed) {
                samples.resize(static_cast<size_t>((cycle + 1) * 4));
                return false;
            }
        }
    }
    return true;
}

bool RunLocal(const Runtime& runtime, const Options& options,
              const TensorGroups& groups)
{
    std::vector<Sample> samples(static_cast<size_t>(options.repeats));
    bool completed = true;
    if (options.mode == "task-compare") {
        completed = RunBalancedCalls(runtime, options, groups, samples);
    } else {
    for (int i = 0; i < options.warmups; ++i) {
        const bool parentFirst = (i & 1) == 0;
        const KernelCall first = parentFirst ? run_kernel_parent : SecondKernel(options);
        const KernelCall second = parentFirst ? SecondKernel(options) : run_kernel_parent;
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
            sample.parentCompleted = Measure(run_kernel_parent, runtime, groups, options, runtime.parentOutput,
                                             sample.parentDeviceUs, sample.parentWallUs);
            if (sample.parentCompleted) {
                sample.candidateCompleted = Measure(SecondKernel(options), runtime, groups, options,
                                                    runtime.candidateOutput, sample.candidateDeviceUs,
                                                    sample.candidateWallUs);
            }
        } else {
            sample.candidateCompleted = Measure(SecondKernel(options), runtime, groups, options,
                                                runtime.candidateOutput, sample.candidateDeviceUs,
                                                sample.candidateWallUs);
            if (sample.candidateCompleted) {
                sample.parentCompleted = Measure(run_kernel_parent, runtime, groups, options, runtime.parentOutput,
                                                 sample.parentDeviceUs, sample.parentWallUs);
            }
        }
        if (!sample.parentCompleted || !sample.candidateCompleted) {
            samples.resize(static_cast<size_t>(i + 1));
            completed = false;
            break;
        }
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
    *output << "# LOCAL_PROXY_ONLY parent=R31B_V011 candidate=" << SecondLabel(options)
            << " comparison=" << options.comparison << " input=deterministic_fp16_pattern_v1"
            << " epsilon=" << kEpsilon << " order="
            << (options.mode == "task-compare" ? "matched_previous_and_position" : "alternating_PC_CP")
            << " device_load=EXTERNAL_NOT_CAPTURED device=" << options.device
            << " rows=" << options.rows << " width=" << options.width << " dtype=fp16"
            << " requested_blocks=" << options.blocks << " effective_blocks="
            << std::min<uint64_t>(options.blocks, static_cast<uint64_t>(options.rows))
            << " warmups=" << options.warmups << " paired_samples=" << options.repeats << '\n';
    if (options.mode == "task-compare") {
        *output << "sample\tprevious_slot\tcycle\tposition\tswapped\tparent_device_us"
                   "\tcandidate_device_us\tdelta_device_us\tparent_wall_us\tcandidate_wall_us"
                   "\tdelta_wall_us\tparent_completed\tcandidate_completed\n";
    } else {
        *output << "sample\torder\tparent_device_us\tcandidate_device_us\tdelta_device_us"
                   "\tparent_wall_us\tcandidate_wall_us\tdelta_wall_us\tparent_completed\tcandidate_completed\n";
    }
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
        *output << i << '\t';
        if (options.mode == "task-compare") {
            *output << sample.previousSlot << '\t' << sample.cycle << '\t' << sample.position
                    << '\t' << sample.swapped << '\t';
        } else {
            *output << sample.order << '\t';
        }
        *output << sample.parentDeviceUs << '\t' << sample.candidateDeviceUs << '\t' << deviceDelta << '\t'
                << sample.parentWallUs << '\t' << sample.candidateWallUs << '\t' << wallDelta << '\t'
                << sample.parentCompleted << '\t' << sample.candidateCompleted << '\n';
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
    if (!completed) {
        std::fprintf(stderr, "TIMING_FAILURE partial_samples_saved=%zu output=%s\n",
                     samples.size(), options.output.c_str());
        return false;
    }

    const uint64_t effectiveBlocks = std::min<uint64_t>(options.blocks, static_cast<uint64_t>(options.rows));
    std::printf("LOCAL_PROXY_ONLY parent=R31B_V011 candidate=%s comparison=%s input=deterministic_fp16_pattern_v1 epsilon=%.8g order=%s device_load=EXTERNAL_NOT_CAPTURED device=%d rows=%lld width=%lld dtype=fp16 requested_blocks=%u effective_blocks=%llu min_rows_per_block=%lld max_rows_per_block=%lld warmups=%d paired_samples=%d\n",
                SecondLabel(options), options.comparison.c_str(), kEpsilon,
                options.mode == "task-compare" ? "matched_previous_and_position" : "alternating_PC_CP",
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

bool StudyMemory(const Options& options, const char* stage)
{
    FILE* pipe = popen("npu-smi info -t usages -i 0", "r");
    if (pipe == nullptr) return false;
    char line[512];
    int capacity = -1;
    int used = -1;
    while (std::fgets(line, sizeof(line), pipe) != nullptr) {
        std::fputs(line, stdout);
        if (std::strstr(line, "HBM Capacity(MB)") != nullptr) std::sscanf(std::strchr(line, ':') + 1, "%d", &capacity);
        if (std::strstr(line, "HBM Usage Rate(%)") != nullptr) std::sscanf(std::strchr(line, ':') + 1, "%d", &used);
    }
    const int status = pclose(pipe);
    const int freeMb = capacity >= 0 && used >= 0 ? capacity * (100 - used) / 100 : -1;
    std::printf("RESOURCE_BEFORE_%s rows=%lld width=%lld device=0 FREE_HBM_MB=%d command_status=%d\n",
                stage, static_cast<long long>(options.rows), static_cast<long long>(options.width), freeMb, status);
    return status == 0 && freeMb >= 100;
}

}  // namespace

int RunCase(const Options& options)
{
    const uint64_t effectiveBlocks = std::min<uint64_t>(
        options.blocks, static_cast<uint64_t>(options.rows));
    if (options.mode != "correctness" &&
        static_cast<uint64_t>(options.rows) / effectiveBlocks < 2) {
        std::fprintf(stderr,
                     "local proxy requires at least two rows per effective block; rows=%lld effective_blocks=%llu\n",
                     static_cast<long long>(options.rows),
                     static_cast<unsigned long long>(effectiveBlocks));
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

    CallTrace trace(options);
    if (options.mode == "task-study" || options.mode == "task-compare") {
        std::printf("TIMING_CONTEXT pid=%ld rows=%lld width=%lld device=%d blocks=%u comparison=%s x=%p residual=%p gamma=%p bias=%p output_P=%p output_C=%p stream=%p start_event=%p stop_event=%p parent_function=%p second_function=%p\n",
                    static_cast<long>(getpid()), static_cast<long long>(options.rows),
                    static_cast<long long>(options.width), options.device, options.blocks,
                    options.comparison.c_str(), runtime.x, runtime.residual, runtime.gamma, runtime.bias,
                    runtime.parentOutput, runtime.candidateOutput, runtime.stream,
                    runtime.startEvent, runtime.stopEvent, reinterpret_cast<void*>(run_kernel_parent),
                    reinterpret_cast<void*>(SecondKernel(options)));
        std::printf("REFERENCE_PHASE=PRE rows=%lld width=%lld\n",
                    static_cast<long long>(options.rows), static_cast<long long>(options.width));
    }
    if (options.mode == "task-compare" && !StudyMemory(options, "CORRECTNESS")) return 1;
    if (options.mode == "task-compare") {
        trace.current.phase = "correctness";
        trace.current.comparison = "CORRECTNESS";
    }
    const bool correctnessOk = RunCorrectness(
        runtime, options, groups, dataBytes,
        parentOutput, candidateOutput, options.mode == "correctness" ? 2 : 1);
    const std::vector<float> reference = CpuReference(options, hostX, hostResidual, hostGamma, hostBias);
    const bool parentReferenceOk = CompareReference("R31B_V011", options, parentOutput, reference);
    const bool candidateReferenceOk = CompareReference(SecondLabel(options), options, candidateOutput, reference);
    if (!correctnessOk) return 3;
    if (options.mode == "parent-diagnostic") {
        std::printf("PARENT_DIAGNOSTIC_ONLY=1 CANDIDATE_DISPATCHED=0 REFERENCE_ACCEPTED=%s\n",
                    parentReferenceOk && candidateReferenceOk ? "YES" : "NO");
    } else if (!parentReferenceOk || !candidateReferenceOk) {
        return 3;
    }
    if (options.mode == "correctness") return 0;
    if (options.mode == "task-compare") {
        trace.current.phase = "timed";
        trace.current.comparison = "PC";
    }
    if (options.mode == "task-compare" && !StudyMemory(options, "LOCAL")) return 1;
    const int groupCount = options.mode == "task-compare" ? 4 : (options.mode == "task-study" ? 3 : 1);
    for (int group = 1; group <= groupCount; ++group) {
        if (options.mode == "task-compare") {
            for (int phase = 0; phase < 2; ++phase) {
                Options phaseOptions = options;
                const bool ppFirst = group == 1 || group == 4;
                const bool pp = (phase == 0) == ppFirst;
                phaseOptions.comparison = pp ? "parent-parent" : "parent-candidate";
                phaseOptions.output += "-r16-d" + std::to_string(options.width) + "-g" +
                                       std::to_string(group) + (pp ? "-pp.tsv" : "-pc.tsv");
                trace.current.group = group;
                trace.current.phaseIndex = (group - 1) * 2 + phase + 1;
                trace.current.comparison = pp ? "PP" : "PC";
                std::printf("TIMING_GROUP rows=16 width=%lld group=%d phase_index=%d comparison=%s warmup_calls=90 priming_calls=8 timed_calls=64 output=%s\n",
                            static_cast<long long>(options.width), group, trace.current.phaseIndex,
                            trace.current.comparison, phaseOptions.output.c_str());
                if (!RunLocal(runtime, phaseOptions, groups)) return 1;
            }
            continue;
        }
        Options sampleOptions = options;
        if (options.mode == "task-study") {
            sampleOptions.output += "-r" + std::to_string(options.rows) + "-d" +
                                    std::to_string(options.width) + "-b" + std::to_string(group) + ".tsv";
            std::printf("TIMING_GROUP rows=%lld width=%lld group=%d warmup_calls=%d timed_calls=%d output=%s\n",
                        static_cast<long long>(options.rows), static_cast<long long>(options.width), group,
                        options.warmups * 2, options.repeats * 2, sampleOptions.output.c_str());
        }
        if (!RunLocal(runtime, sampleOptions, groups)) return 1;
    }
    if (options.mode == "task-study" || options.mode == "task-compare") {
        std::printf("REFERENCE_PHASE=POST rows=%lld width=%lld additional_launches=0\n",
                    static_cast<long long>(options.rows), static_cast<long long>(options.width));
        if (options.mode == "task-compare") {
            if (!CheckAcl(aclrtMemcpy(parentOutput.data(), dataBytes, runtime.candidateOutput, dataBytes,
                                      ACL_MEMCPY_DEVICE_TO_HOST), "copy mapped Parent output") ||
                !CheckAcl(aclrtMemcpy(candidateOutput.data(), dataBytes, runtime.parentOutput, dataBytes,
                                      ACL_MEMCPY_DEVICE_TO_HOST), "copy mapped Candidate output")) return 1;
            std::printf("POST_OUTPUT_MAPPING parent_address=%p candidate_address=%p\n",
                        runtime.candidateOutput, runtime.parentOutput);
        } else if (!CopyOutputs(runtime, dataBytes, parentOutput, candidateOutput)) {
            return 1;
        }
        const bool pairOk = CompareOutputs(runtime, options, parentOutput, candidateOutput);
        const bool firstOk = CompareReference("R31B_V011", options, parentOutput, reference);
        const bool secondOk = CompareReference(SecondLabel(options), options, candidateOutput, reference);
        if (!pairOk || !firstOk || !secondOk) return 3;
    }
    return trace.Save() ? 0 : 1;
}

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
        options.width <= 8192 || options.blocks == 0 || options.dtype != "fp16") {
        PrintUsage(argv[0]);
        return 2;
    }
    if (options.mode == "task-study" &&
        (options.rows != 16 || options.width != 16384 || options.blocks != 8 ||
         options.warmups != 45 || options.repeats != 42 || options.output.empty())) {
        std::fprintf(stderr, "task-study requires the declared R01 dimensions, counts and output prefix\n");
        return 2;
    }
    if (options.mode == "task-compare" &&
        (options.device != 0 || options.rows != 16 || options.width != 16384 || options.blocks != 8 ||
         options.warmups != 45 || options.repeats != 32 || options.output.empty())) {
        std::fprintf(stderr, "task-compare requires the declared device, dimensions, counts and output prefix\n");
        return 2;
    }
    Runtime session;
    if (!CheckAcl(aclInit(nullptr), "aclInit")) return 1;
    session.initialized = true;
    if (options.mode == "task-study" || options.mode == "task-compare") {
        Dl_info parentInfo{}, candidateInfo{};
        dladdr(reinterpret_cast<void*>(run_kernel_parent), &parentInfo);
        dladdr(reinterpret_cast<void*>(run_kernel_candidate), &candidateInfo);
        std::printf("STUDY_SESSION pid=%ld parent_library=%s candidate_library=%s candidate_dispatched=%d\n",
                    static_cast<long>(getpid()), parentInfo.dli_fname, candidateInfo.dli_fname,
                    options.mode == "task-compare" ? 1 : 0);
        std::ifstream maps("/proc/self/maps");
        std::string line;
        while (std::getline(maps, line)) {
            if (line.find("libw4r01_v001_") != std::string::npos) {
                std::printf("LIBRARY_MAP %s\n", line.c_str());
            }
        }
    }
    const int firstStatus = RunCase(options);
    if (firstStatus != 0 || (options.mode != "task-study" && options.mode != "task-compare")) return firstStatus;
    options.width = 32768;
    return RunCase(options);
}
