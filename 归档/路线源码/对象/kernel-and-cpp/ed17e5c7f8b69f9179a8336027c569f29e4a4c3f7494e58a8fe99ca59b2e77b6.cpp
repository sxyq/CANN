#include "paired_runner_abi.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <numeric>
#include <string>
#include <vector>

namespace {

using RunKernel = decltype(&tiny_parent::run_kernel_parent);

constexpr int kCoreCount = 8;
constexpr int kWarmup = 60;
constexpr int kQualificationBlocks = 2;
constexpr int kQualificationSamples = 31;
constexpr int kPairs = 21;
constexpr float kEpsilon = 1.0e-5f;

struct CaseSpec {
    const char* name;
    int dtype;
    int rows;
    int width;
};

struct DeviceBuffers {
    void* x = nullptr;
    void* residual = nullptr;
    void* gamma = nullptr;
    void* bias = nullptr;
    void* parentOutput = nullptr;
    void* candidateOutput = nullptr;
};

struct TimedSample {
    double deviceUs;
    double wallUs;
};

struct Stats {
    double median;
    double mean;
    double stdev;
    double cv;
    double minimum;
    double maximum;
    double mad;
    double p10;
    double p90;
};

constexpr CaseSpec kCorrectnessCases[] = {
    {"target-r1-d64-fp32", 0, 1, 64},
    {"control-r2-d64-fp32", 0, 2, 64},
    {"control-r1-d129-fp32", 0, 1, 129},
    {"target-r1-d64-fp16", 1, 1, 64},
    {"control-r2-d64-fp16", 1, 2, 64},
    {"control-r1-d129-fp16", 1, 1, 129},
    {"target-r1-d64-bf16", 2, 1, 64},
    {"control-r2-d64-bf16", 2, 2, 64},
    {"control-r1-d129-bf16", 2, 1, 129},
};

constexpr CaseSpec kLocalCases[] = {
    {"target-r1-d64-fp32", 0, 1, 64},
    {"control-r2-d64-fp32", 0, 2, 64},
    {"control-r1-d129-fp32", 0, 1, 129},
};

void CheckAcl(aclError status, const char* operation)
{
    if (status != ACL_SUCCESS) {
        std::fprintf(stderr, "%s failed: %d %s\n", operation, status,
                     aclGetRecentErrMsg() ? aclGetRecentErrMsg() : "");
        std::exit(2);
    }
}

size_t ElementBytes(int dtype)
{
    return dtype == 0 ? sizeof(float) : sizeof(uint16_t);
}

uint16_t FloatToHalf(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    const uint32_t sign = (bits >> 16) & 0x8000u;
    const uint32_t exponent = (bits >> 23) & 0xffu;
    uint32_t mantissa = bits & 0x7fffffu;
    if (exponent == 0xffu) {
        return static_cast<uint16_t>(sign | 0x7c00u | (mantissa ? 0x0200u : 0u));
    }
    int32_t halfExponent = static_cast<int32_t>(exponent) - 112;
    if (halfExponent >= 31) {
        return static_cast<uint16_t>(sign | 0x7c00u);
    }
    if (halfExponent <= 0) {
        if (halfExponent < -10) {
            return static_cast<uint16_t>(sign);
        }
        mantissa |= 0x800000u;
        const int32_t shift = 14 - halfExponent;
        const uint32_t round = (1u << (shift - 1)) - 1u + ((mantissa >> shift) & 1u);
        return static_cast<uint16_t>(sign | ((mantissa + round) >> shift));
    }
    mantissa += 0xfffu + ((mantissa >> 13) & 1u);
    if (mantissa & 0x800000u) {
        mantissa = 0;
        ++halfExponent;
        if (halfExponent >= 31) {
            return static_cast<uint16_t>(sign | 0x7c00u);
        }
    }
    return static_cast<uint16_t>(sign | (static_cast<uint32_t>(halfExponent) << 10) |
                                 (mantissa >> 13));
}

float HalfToFloat(uint16_t value)
{
    const float sign = (value & 0x8000u) ? -1.0f : 1.0f;
    const uint32_t exponent = (value >> 10) & 0x1fu;
    const uint32_t mantissa = value & 0x03ffu;
    if (exponent == 0) {
        return sign * std::ldexp(static_cast<float>(mantissa), -24);
    }
    if (exponent == 0x1fu) {
        return mantissa ? NAN : sign * INFINITY;
    }
    return sign * std::ldexp(1.0f + static_cast<float>(mantissa) / 1024.0f,
                             static_cast<int>(exponent) - 15);
}

uint16_t FloatToBfloat16(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    bits += 0x7fffu + ((bits >> 16) & 1u);
    return static_cast<uint16_t>(bits >> 16);
}

float Bfloat16ToFloat(uint16_t value)
{
    const uint32_t bits = static_cast<uint32_t>(value) << 16;
    float result = 0.0f;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

void PutValue(std::vector<uint8_t>& data, size_t index, int dtype, float value)
{
    if (dtype == 0) {
        std::memcpy(data.data() + index * sizeof(float), &value, sizeof(value));
        return;
    }
    const uint16_t encoded = dtype == 1 ? FloatToHalf(value) : FloatToBfloat16(value);
    std::memcpy(data.data() + index * sizeof(uint16_t), &encoded, sizeof(encoded));
}

float GetValue(const std::vector<uint8_t>& data, size_t index, int dtype)
{
    if (dtype == 0) {
        float value = 0.0f;
        std::memcpy(&value, data.data() + index * sizeof(float), sizeof(value));
        return value;
    }
    uint16_t encoded = 0;
    std::memcpy(&encoded, data.data() + index * sizeof(uint16_t), sizeof(encoded));
    return dtype == 1 ? HalfToFloat(encoded) : Bfloat16ToFloat(encoded);
}

void Allocate(void** pointer, size_t bytes, const char* label)
{
    CheckAcl(aclrtMalloc(pointer, bytes, ACL_MEM_MALLOC_HUGE_FIRST), label);
}

void FreeCase(DeviceBuffers& buffers)
{
    CheckAcl(aclrtFree(buffers.candidateOutput), "free candidate output");
    CheckAcl(aclrtFree(buffers.parentOutput), "free parent output");
    CheckAcl(aclrtFree(buffers.bias), "free bias");
    CheckAcl(aclrtFree(buffers.gamma), "free gamma");
    CheckAcl(aclrtFree(buffers.residual), "free residual");
    CheckAcl(aclrtFree(buffers.x), "free x");
}

void MakeInput(const CaseSpec& spec, std::vector<uint8_t>& x,
               std::vector<uint8_t>& residual, std::vector<uint8_t>& gamma,
               std::vector<uint8_t>& bias)
{
    const size_t count = static_cast<size_t>(spec.rows) * spec.width;
    for (size_t i = 0; i < count; ++i) {
        const float xv = 0.19f * std::sin(static_cast<float>((i * 17) % 997) * 0.013f);
        const float rv = 0.11f * std::cos(static_cast<float>((i * 29) % 991) * 0.017f);
        PutValue(x, i, spec.dtype, xv);
        PutValue(residual, i, spec.dtype, rv);
    }
    for (int col = 0; col < spec.width; ++col) {
        PutValue(gamma, col, spec.dtype,
                 0.85f + static_cast<float>(col % 23) * 0.002f);
        PutValue(bias, col, spec.dtype,
                 -0.025f + static_cast<float>(col % 19) * 0.001f);
    }
}

void Launch(RunKernel kernel, void* output, const DeviceBuffers& buffers,
            const TensorGroupInfo& dataGroup, const TensorGroupInfo& paramGroup,
            aclrtStream stream)
{
    kernel(static_cast<uint8_t*>(buffers.x), dataGroup,
           static_cast<uint8_t*>(buffers.residual), dataGroup,
           static_cast<uint8_t*>(buffers.gamma), paramGroup,
           static_cast<uint8_t*>(buffers.bias), paramGroup,
           static_cast<uint8_t*>(output), dataGroup, kCoreCount, stream, kEpsilon);
}

void CheckExact(const CaseSpec& spec, const std::vector<uint8_t>& parent,
                const std::vector<uint8_t>& candidate)
{
    const size_t count = static_cast<size_t>(spec.rows) * spec.width;
    const size_t elementBytes = ElementBytes(spec.dtype);
    size_t mismatches = 0;
    float maxAbs = 0.0f;
    for (size_t i = 0; i < count; ++i) {
        if (std::memcmp(parent.data() + i * elementBytes,
                        candidate.data() + i * elementBytes, elementBytes) != 0) {
            ++mismatches;
            maxAbs = std::max(maxAbs,
                              std::fabs(GetValue(parent, i, spec.dtype) -
                                        GetValue(candidate, i, spec.dtype)));
        }
    }
    std::printf("CORRECTNESS case=%s result=%s mismatches=%zu max_abs=%.9g compare=bitwise\n",
                spec.name, mismatches == 0 ? "PASS" : "FAIL", mismatches, maxAbs);
    if (mismatches != 0) {
        std::exit(3);
    }
}

void RunCorrectnessCase(const CaseSpec& spec, aclrtStream stream)
{
    const size_t count = static_cast<size_t>(spec.rows) * spec.width;
    const size_t dataBytes = count * ElementBytes(spec.dtype);
    const size_t paramBytes = static_cast<size_t>(spec.width) * ElementBytes(spec.dtype);
    std::vector<uint8_t> x(dataBytes), residual(dataBytes), gamma(paramBytes), bias(paramBytes);
    std::vector<uint8_t> parentOutput(dataBytes), candidateOutput(dataBytes);
    MakeInput(spec, x, residual, gamma, bias);

    DeviceBuffers buffers;
    Allocate(&buffers.x, dataBytes, "malloc x");
    Allocate(&buffers.residual, dataBytes, "malloc residual");
    Allocate(&buffers.gamma, paramBytes, "malloc gamma");
    Allocate(&buffers.bias, paramBytes, "malloc bias");
    Allocate(&buffers.parentOutput, dataBytes, "malloc parent output");
    Allocate(&buffers.candidateOutput, dataBytes, "malloc candidate output");
    CheckAcl(aclrtMemcpy(buffers.x, dataBytes, x.data(), dataBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy x");
    CheckAcl(aclrtMemcpy(buffers.residual, dataBytes, residual.data(), dataBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy residual");
    CheckAcl(aclrtMemcpy(buffers.gamma, paramBytes, gamma.data(), paramBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma");
    CheckAcl(aclrtMemcpy(buffers.bias, paramBytes, bias.data(), paramBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy bias");

    const int64_t dataShape[2] = {spec.rows, spec.width};
    const int64_t paramShape[1] = {spec.width};
    const TensorInfo dataInfo{dataShape, 2, spec.dtype};
    const TensorInfo paramInfo{paramShape, 1, spec.dtype};
    const TensorGroupInfo dataGroup{&dataInfo, 1};
    const TensorGroupInfo paramGroup{&paramInfo, 1};
    std::printf("CASE_BEGIN name=%s rows=%d width=%d dtype=%d mode=CORRECTNESS\n",
                spec.name, spec.rows, spec.width, spec.dtype);
    Launch(tiny_parent::run_kernel_parent, buffers.parentOutput, buffers,
           dataGroup, paramGroup, stream);
    Launch(tiny_candidate::run_kernel_candidate, buffers.candidateOutput, buffers,
           dataGroup, paramGroup, stream);
    CheckAcl(aclrtSynchronizeStream(stream), "correctness synchronize");
    CheckAcl(aclrtMemcpy(parentOutput.data(), dataBytes, buffers.parentOutput, dataBytes,
                         ACL_MEMCPY_DEVICE_TO_HOST), "copy parent output");
    CheckAcl(aclrtMemcpy(candidateOutput.data(), dataBytes, buffers.candidateOutput,
                         dataBytes, ACL_MEMCPY_DEVICE_TO_HOST), "copy candidate output");
    CheckExact(spec, parentOutput, candidateOutput);
    FreeCase(buffers);
}

double Quantile(const std::vector<double>& sorted, double fraction)
{
    const double position = fraction * static_cast<double>(sorted.size() - 1);
    const size_t lower = static_cast<size_t>(position);
    const size_t upper = std::min(lower + 1, sorted.size() - 1);
    const double weight = position - static_cast<double>(lower);
    return sorted[lower] * (1.0 - weight) + sorted[upper] * weight;
}

Stats Summarize(const std::vector<double>& values)
{
    std::vector<double> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    const double median = Quantile(sorted, 0.5);
    const double mean = std::accumulate(values.begin(), values.end(), 0.0) /
                        static_cast<double>(values.size());
    double variance = 0.0;
    std::vector<double> deviations(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
        const double delta = values[i] - mean;
        variance += delta * delta;
        deviations[i] = std::fabs(values[i] - median);
    }
    std::sort(deviations.begin(), deviations.end());
    const double stdev = values.size() > 1
                             ? std::sqrt(variance / static_cast<double>(values.size() - 1))
                             : 0.0;
    return {median, mean, stdev, mean == 0.0 ? 0.0 : stdev / mean,
            sorted.front(), sorted.back(), Quantile(deviations, 0.5),
            Quantile(sorted, 0.10), Quantile(sorted, 0.90)};
}

void PrintStats(const char* name, const char* metric, const char* binary,
                const std::vector<double>& values)
{
    const Stats stats = Summarize(values);
    std::printf("SUMMARY case=%s metric=%s binary=%s n=%zu median=%.3f mean=%.3f stdev=%.3f cv=%.5f min=%.3f max=%.3f mad=%.3f p10=%.3f p90=%.3f\n",
                name, metric, binary, values.size(), stats.median, stats.mean,
                stats.stdev, stats.cv, stats.minimum, stats.maximum, stats.mad,
                stats.p10, stats.p90);
}

TimedSample Measure(RunKernel kernel, void* output, const DeviceBuffers& buffers,
                    const TensorGroupInfo& dataGroup, const TensorGroupInfo& paramGroup,
                    aclrtStream stream)
{
    aclrtEvent start = nullptr;
    aclrtEvent stop = nullptr;
    CheckAcl(aclrtCreateEvent(&start), "create start event");
    CheckAcl(aclrtCreateEvent(&stop), "create stop event");
    const auto wallStart = std::chrono::steady_clock::now();
    CheckAcl(aclrtRecordEvent(start, stream), "record start event");
    Launch(kernel, output, buffers, dataGroup, paramGroup, stream);
    CheckAcl(aclrtRecordEvent(stop, stream), "record stop event");
    CheckAcl(aclrtSynchronizeEvent(stop), "synchronize stop event");
    const auto wallStop = std::chrono::steady_clock::now();
    float elapsedMs = 0.0f;
    CheckAcl(aclrtEventElapsedTime(&elapsedMs, start, stop), "read event elapsed time");
    CheckAcl(aclrtDestroyEvent(stop), "destroy stop event");
    CheckAcl(aclrtDestroyEvent(start), "destroy start event");
    return {static_cast<double>(elapsedMs) * 1000.0,
            std::chrono::duration<double, std::micro>(wallStop - wallStart).count()};
}

void PrepareTimingCase(const CaseSpec& spec, std::vector<uint8_t>& x,
                       std::vector<uint8_t>& residual, std::vector<uint8_t>& gamma,
                       std::vector<uint8_t>& bias, DeviceBuffers& buffers,
                       TensorInfo& dataInfo, TensorInfo& paramInfo,
                       TensorGroupInfo& dataGroup, TensorGroupInfo& paramGroup)
{
    const size_t count = static_cast<size_t>(spec.rows) * spec.width;
    const size_t dataBytes = count * ElementBytes(spec.dtype);
    const size_t paramBytes = static_cast<size_t>(spec.width) * ElementBytes(spec.dtype);
    x.resize(dataBytes);
    residual.resize(dataBytes);
    gamma.resize(paramBytes);
    bias.resize(paramBytes);
    MakeInput(spec, x, residual, gamma, bias);
    Allocate(&buffers.x, dataBytes, "malloc x");
    Allocate(&buffers.residual, dataBytes, "malloc residual");
    Allocate(&buffers.gamma, paramBytes, "malloc gamma");
    Allocate(&buffers.bias, paramBytes, "malloc bias");
    Allocate(&buffers.parentOutput, dataBytes, "malloc parent output");
    Allocate(&buffers.candidateOutput, dataBytes, "malloc candidate output");
    CheckAcl(aclrtMemcpy(buffers.x, dataBytes, x.data(), dataBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy x");
    CheckAcl(aclrtMemcpy(buffers.residual, dataBytes, residual.data(), dataBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy residual");
    CheckAcl(aclrtMemcpy(buffers.gamma, paramBytes, gamma.data(), paramBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma");
    CheckAcl(aclrtMemcpy(buffers.bias, paramBytes, bias.data(), paramBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy bias");
    static thread_local int64_t dataShape[2];
    static thread_local int64_t paramShape[1];
    dataShape[0] = spec.rows;
    dataShape[1] = spec.width;
    paramShape[0] = spec.width;
    dataInfo = {dataShape, 2, spec.dtype};
    paramInfo = {paramShape, 1, spec.dtype};
    dataGroup = {&dataInfo, 1};
    paramGroup = {&paramInfo, 1};
}

bool RunParentQualification(const CaseSpec& spec, aclrtStream stream)
{
    std::vector<uint8_t> x, residual, gamma, bias;
    DeviceBuffers buffers;
    TensorInfo dataInfo{}, paramInfo{};
    TensorGroupInfo dataGroup{}, paramGroup{};
    PrepareTimingCase(spec, x, residual, gamma, bias, buffers,
                      dataInfo, paramInfo, dataGroup, paramGroup);
    for (int i = 0; i < kWarmup; ++i) {
        Launch(tiny_parent::run_kernel_parent, buffers.parentOutput, buffers,
               dataGroup, paramGroup, stream);
        CheckAcl(aclrtSynchronizeStream(stream), "parent warmup synchronize");
    }
    std::vector<double> allDevice;
    std::vector<double> allWall;
    double blockMedians[kQualificationBlocks] = {};
    for (int block = 0; block < kQualificationBlocks; ++block) {
        std::vector<double> blockDevice;
        for (int sample = 0; sample < kQualificationSamples; ++sample) {
            const TimedSample timed = Measure(
                tiny_parent::run_kernel_parent, buffers.parentOutput, buffers,
                dataGroup, paramGroup, stream);
            blockDevice.push_back(timed.deviceUs);
            allDevice.push_back(timed.deviceUs);
            allWall.push_back(timed.wallUs);
            std::printf("PARENT_SAMPLE case=%s block=%d sample=%02d device_us=%.3f wall_us=%.3f\n",
                        spec.name, block + 1, sample + 1, timed.deviceUs, timed.wallUs);
        }
        blockMedians[block] = Summarize(blockDevice).median;
        PrintStats(spec.name, "device_us", block == 0 ? "PARENT_BLOCK_1" : "PARENT_BLOCK_2",
                   blockDevice);
    }
    const Stats full = Summarize(allDevice);
    const double madRatio = full.median > 0.0 ? full.mad / full.median : INFINITY;
    const double drift = full.median > 0.0
                             ? std::fabs(blockMedians[0] - blockMedians[1]) / full.median
                             : INFINITY;
    const bool passed = madRatio <= 0.10 && drift <= 0.10;
    const char* result = passed ? "PASS" :
                         (madRatio > 0.25 || drift > 0.25)
                             ? "MEASUREMENT_PROTOCOL_BLOCKED" : "NEEDS_VALIDATION";
    PrintStats(spec.name, "device_us", "PARENT_ALL", allDevice);
    PrintStats(spec.name, "wall_us", "PARENT_ALL", allWall);
    std::printf("PARENT_QUAL case=%s result=%s samples=%zu median_us=%.3f mad_over_median=%.5f block_drift=%.5f\n",
                spec.name, result, allDevice.size(), full.median, madRatio, drift);
    FreeCase(buffers);
    return passed;
}

void RunPairedCase(const CaseSpec& spec, aclrtStream stream)
{
    std::vector<uint8_t> x, residual, gamma, bias;
    std::vector<uint8_t> parentOutput(static_cast<size_t>(spec.rows) * spec.width * sizeof(float));
    std::vector<uint8_t> candidateOutput(parentOutput.size());
    DeviceBuffers buffers;
    TensorInfo dataInfo{}, paramInfo{};
    TensorGroupInfo dataGroup{}, paramGroup{};
    PrepareTimingCase(spec, x, residual, gamma, bias, buffers,
                      dataInfo, paramInfo, dataGroup, paramGroup);
    for (int i = 0; i < kWarmup; ++i) {
        Launch(tiny_parent::run_kernel_parent, buffers.parentOutput, buffers,
               dataGroup, paramGroup, stream);
        CheckAcl(aclrtSynchronizeStream(stream), "parent warmup synchronize");
        Launch(tiny_candidate::run_kernel_candidate, buffers.candidateOutput, buffers,
               dataGroup, paramGroup, stream);
        CheckAcl(aclrtSynchronizeStream(stream), "candidate warmup synchronize");
    }
    std::vector<double> parentDevice, candidateDevice, parentWall, candidateWall, deltas;
    for (int pair = 0; pair < kPairs; ++pair) {
        TimedSample parent{}, candidate{};
        const bool parentFirst = (pair % 2) == 0;
        if (parentFirst) {
            parent = Measure(tiny_parent::run_kernel_parent, buffers.parentOutput, buffers,
                             dataGroup, paramGroup, stream);
            candidate = Measure(tiny_candidate::run_kernel_candidate, buffers.candidateOutput,
                                buffers, dataGroup, paramGroup, stream);
        } else {
            candidate = Measure(tiny_candidate::run_kernel_candidate, buffers.candidateOutput,
                                buffers, dataGroup, paramGroup, stream);
            parent = Measure(tiny_parent::run_kernel_parent, buffers.parentOutput, buffers,
                             dataGroup, paramGroup, stream);
        }
        const double delta = candidate.deviceUs - parent.deviceUs;
        parentDevice.push_back(parent.deviceUs);
        candidateDevice.push_back(candidate.deviceUs);
        parentWall.push_back(parent.wallUs);
        candidateWall.push_back(candidate.wallUs);
        deltas.push_back(delta);
        std::printf("PAIR case=%s pair=%02d order=%s parent_device_us=%.3f candidate_device_us=%.3f parent_wall_us=%.3f candidate_wall_us=%.3f delta_us=%.3f\n",
                    spec.name, pair + 1, parentFirst ? "P,C" : "C,P",
                    parent.deviceUs, candidate.deviceUs, parent.wallUs,
                    candidate.wallUs, delta);
    }
    const size_t dataBytes = static_cast<size_t>(spec.rows) * spec.width * sizeof(float);
    CheckAcl(aclrtMemcpy(parentOutput.data(), dataBytes, buffers.parentOutput, dataBytes,
                         ACL_MEMCPY_DEVICE_TO_HOST), "copy parent output");
    CheckAcl(aclrtMemcpy(candidateOutput.data(), dataBytes, buffers.candidateOutput,
                         dataBytes, ACL_MEMCPY_DEVICE_TO_HOST), "copy candidate output");
    CheckExact(spec, parentOutput, candidateOutput);
    PrintStats(spec.name, "device_us", "PARENT", parentDevice);
    PrintStats(spec.name, "device_us", "CANDIDATE", candidateDevice);
    PrintStats(spec.name, "wall_us", "PARENT", parentWall);
    PrintStats(spec.name, "wall_us", "CANDIDATE", candidateWall);
    PrintStats(spec.name, "paired_delta_us", "CANDIDATE_MINUS_PARENT", deltas);
    const Stats parentStats = Summarize(parentDevice);
    const Stats candidateStats = Summarize(candidateDevice);
    const Stats deltaStats = Summarize(deltas);
    const double percent = parentStats.median == 0.0
                               ? 0.0
                               : (candidateStats.median / parentStats.median - 1.0) * 100.0;
    std::printf("LOCAL_RESULT case=%s parent_median_us=%.3f candidate_median_us=%.3f delta_median_us=%.3f delta_percent=%.4f\n",
                spec.name, parentStats.median, candidateStats.median,
                deltaStats.median, percent);
    FreeCase(buffers);
}

}  // namespace

int main(int argc, char** argv)
{
    if (argc != 3) {
        std::fprintf(stderr,
                     "usage: tiny_runner_v007 --correctness|--parent-qualify|--paired DEVICE\n");
        return 1;
    }
    const std::string mode = argv[1];
    const int device = std::atoi(argv[2]);
    if (device < 0 || device > 7) {
        std::fprintf(stderr, "device must be in [0,7]\n");
        return 1;
    }
    if (mode != "--correctness" && mode != "--parent-qualify" && mode != "--paired") {
        std::fprintf(stderr, "unknown mode: %s\n", argv[1]);
        return 1;
    }

    CheckAcl(aclInit(nullptr), "aclInit");
    CheckAcl(aclrtSetDevice(device), "set device");
    aclrtStream stream = nullptr;
    CheckAcl(aclrtCreateStream(&stream), "create stream");
    std::printf("RUN_BEGIN mode=%s device=%d parent=R31B-V011 cases=%s\n",
                mode.c_str(), device,
                mode == "--correctness" ? "9" : "3");

    if (mode == "--correctness") {
        for (const CaseSpec& spec : kCorrectnessCases) {
            RunCorrectnessCase(spec, stream);
        }
        std::printf("RUNNER_COMPLETE mode=CORRECTNESS result=PASS cases=9 compare=bitwise\n");
    } else if (mode == "--parent-qualify") {
        bool allPassed = true;
        for (const CaseSpec& spec : kLocalCases) {
            allPassed = RunParentQualification(spec, stream) && allPassed;
        }
        std::printf("RUNNER_COMPLETE mode=PARENT_QUALIFY status=%s cases=3 thresholds=mad_median<=0.10,drift<=0.10\n",
                    allPassed ? "PASS" : "NOT_QUALIFIED");
        if (!allPassed) {
            CheckAcl(aclrtDestroyStream(stream), "destroy stream");
            CheckAcl(aclFinalize(), "aclFinalize");
            return 4;
        }
    } else {
        for (const CaseSpec& spec : kLocalCases) {
            RunPairedCase(spec, stream);
        }
        std::printf("RUNNER_COMPLETE mode=PAIRED cases=3 pairs_each=%d timing=DEVICE_EVENT_PRIMARY\n",
                    kPairs);
    }

    CheckAcl(aclrtDestroyStream(stream), "destroy stream");
    CheckAcl(aclFinalize(), "aclFinalize");
    return 0;
}
