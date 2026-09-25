#include "paired_runner_abi.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <numeric>
#include <vector>

namespace {

using R31BRunKernel = decltype(&r31b_v011::run_kernel_v011);

constexpr int kDevice = 4;
constexpr int kRows = 2;
constexpr int kWarmup = 10;
constexpr int kPairs = 21;
constexpr int kQualificationBlocks = 2;
constexpr int kQualificationSamplesPerBlock = 31;
constexpr int kCoreCount = 8;
constexpr float kEpsilon = 1.0e-5f;

enum class RunMode {
    kQualifyV011,
    kPaired,
};

struct CaseSpec {
    const char* name;
    int dtype;
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

struct Events {
    aclrtEvent start = nullptr;
    aclrtEvent stop = nullptr;
    bool recorded = false;
};

struct TimedSample {
    double deviceUs;
    double wallUs;
};

struct PairSample {
    TimedSample parent;
    TimedSample candidate;
    double deltaUs;
};

struct Stats {
    double median;
    double mean;
    double stdev;
    double cv;
    double minimum;
    double maximum;
    double maxMin;
    double mad;
    double p10;
    double p90;
    double spread;
    double coreCv;
};

void CheckAcl(aclError status, const char* operation)
{
    if (status != ACL_SUCCESS) {
        std::fprintf(stderr, "%s failed: %d %s\n", operation, status,
                     aclGetRecentErrMsg() ? aclGetRecentErrMsg() : "");
        std::exit(2);
    }
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

float Decode(uint16_t value, int dtype)
{
    return dtype == 1 ? HalfToFloat(value) : Bfloat16ToFloat(value);
}

uint16_t Encode(float value, int dtype)
{
    return dtype == 1 ? FloatToHalf(value) : FloatToBfloat16(value);
}

float Rounded(float value, int dtype)
{
    return Decode(Encode(value, dtype), dtype);
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

void Launch(R31BRunKernel kernel, void* output, const DeviceBuffers& buffers,
            const TensorGroupInfo& dataGroup, const TensorGroupInfo& paramGroup,
            aclrtStream stream)
{
    kernel(static_cast<uint8_t*>(buffers.x), dataGroup,
           static_cast<uint8_t*>(buffers.residual), dataGroup,
           static_cast<uint8_t*>(buffers.gamma), paramGroup,
           static_cast<uint8_t*>(buffers.bias), paramGroup,
           static_cast<uint8_t*>(output), dataGroup, kCoreCount, stream, kEpsilon);
}

TimedSample Measure(R31BRunKernel kernel, void* output, const DeviceBuffers& buffers,
                    const TensorGroupInfo& dataGroup, const TensorGroupInfo& paramGroup,
                    aclrtStream stream, Events& events)
{
    if (events.recorded) {
        CheckAcl(aclrtResetEvent(events.start, stream), "reset start event");
        CheckAcl(aclrtResetEvent(events.stop, stream), "reset stop event");
    }
    const auto wallStart = std::chrono::steady_clock::now();
    CheckAcl(aclrtRecordEvent(events.start, stream), "record start event");
    Launch(kernel, output, buffers, dataGroup, paramGroup, stream);
    CheckAcl(aclrtRecordEvent(events.stop, stream), "record stop event");
    CheckAcl(aclrtSynchronizeEvent(events.stop), "synchronize stop event");
    const auto wallStop = std::chrono::steady_clock::now();
    events.recorded = true;

    float elapsedMs = 0.0f;
    CheckAcl(aclrtEventElapsedTime(&elapsedMs, events.start, events.stop),
             "read event elapsed time");
    return {static_cast<double>(elapsedMs) * 1000.0,
            std::chrono::duration<double, std::micro>(wallStop - wallStart).count()};
}

void CheckOutput(const CaseSpec& spec, const std::vector<uint16_t>& x,
                 const std::vector<uint16_t>& residual,
                 const std::vector<uint16_t>& gamma,
                 const std::vector<uint16_t>& bias,
                 const std::vector<uint16_t>& output, const char* label)
{
    const size_t count = static_cast<size_t>(kRows) * spec.width;
    const float tolerance = spec.dtype == 1 ? 0.004f : 0.02f;
    float maxAbs = 0.0f;
    float maxRel = 0.0f;
    size_t failures = 0;
    for (int row = 0; row < kRows; ++row) {
        float squareSum = 0.0f;
        std::vector<float> y(spec.width);
        for (int col = 0; col < spec.width; ++col) {
            const size_t index = static_cast<size_t>(row) * spec.width + col;
            float value = Decode(x[index], spec.dtype) + Decode(residual[index], spec.dtype);
            if (spec.dtype == 1) {
                value = Rounded(value, spec.dtype);
            }
            y[col] = value;
            squareSum += value * value;
        }
        const float invRms = 1.0f / std::sqrt(squareSum / static_cast<float>(spec.width) + kEpsilon);
        for (int col = 0; col < spec.width; ++col) {
            const size_t index = static_cast<size_t>(row) * spec.width + col;
            float expected = y[col] * invRms;
            if (spec.dtype == 1) {
                expected = Rounded(expected, spec.dtype);
                expected = Rounded(expected * Decode(gamma[col], spec.dtype), spec.dtype);
                expected = Rounded(expected + Decode(bias[col], spec.dtype), spec.dtype);
            } else {
                expected = Rounded(expected * Decode(gamma[col], spec.dtype) +
                                   Decode(bias[col], spec.dtype), spec.dtype);
            }
            const float actual = Decode(output[index], spec.dtype);
            const float absError = std::fabs(actual - expected);
            const float relError = absError / std::max(std::fabs(expected), 1.0e-6f);
            maxAbs = std::max(maxAbs, absError);
            maxRel = std::max(maxRel, relError);
            if (absError > tolerance + tolerance * std::fabs(expected)) {
                ++failures;
            }
        }
    }
    std::printf("CORRECTNESS case=%s binary=%s result=%s failures=%zu max_abs=%.8g max_rel=%.8g\n",
                spec.name, label, failures == 0 ? "PASS" : "FAIL", failures, maxAbs, maxRel);
    if (count == 0 || failures != 0) {
        std::exit(3);
    }
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
    std::vector<double> core;
    for (size_t i = 0; i < values.size(); ++i) {
        const double delta = values[i] - mean;
        variance += delta * delta;
        deviations[i] = std::fabs(values[i] - median);
        if (median > 0.0 && std::fabs(values[i] - median) <= 0.2 * median) {
            core.push_back(values[i]);
        }
    }
    std::sort(deviations.begin(), deviations.end());
    const double stdev = std::sqrt(variance / static_cast<double>(values.size() - 1));
    double coreCv = 0.0;
    if (core.size() > 1) {
        const double coreMean = std::accumulate(core.begin(), core.end(), 0.0) /
                                static_cast<double>(core.size());
        double coreVariance = 0.0;
        for (double value : core) {
            const double delta = value - coreMean;
            coreVariance += delta * delta;
        }
        coreCv = coreMean == 0.0 ? 0.0 :
                 std::sqrt(coreVariance / static_cast<double>(core.size() - 1)) / coreMean;
    }
    return {median, mean, stdev, mean == 0.0 ? 0.0 : stdev / mean,
            sorted.front(), sorted.back(), sorted.front() == 0.0 ? 0.0 : sorted.back() / sorted.front(),
            Quantile(deviations, 0.5), Quantile(sorted, 0.10), Quantile(sorted, 0.90),
            sorted.back() - sorted.front(), coreCv};
}

void PrintStats(const char* caseName, const char* metric, const char* binary,
                const std::vector<double>& values)
{
    const Stats s = Summarize(values);
    std::printf("SUMMARY case=%s metric=%s binary=%s n=%zu median=%.3f mean=%.3f stdev=%.3f cv=%.5f min=%.3f max=%.3f max_min=%.3f mad=%.3f p10=%.3f p90=%.3f spread=%.3f core_cv=%.5f\n",
                caseName, metric, binary, values.size(), s.median, s.mean, s.stdev,
                s.cv, s.minimum, s.maximum, s.maxMin, s.mad, s.p10, s.p90,
                s.spread, s.coreCv);
}

void RunCase(const CaseSpec& spec, aclrtStream stream, Events& events, RunMode mode)
{
    const size_t elementCount = static_cast<size_t>(kRows) * spec.width;
    const size_t dataBytes = elementCount * sizeof(uint16_t);
    const size_t paramBytes = static_cast<size_t>(spec.width) * sizeof(uint16_t);
    std::vector<uint16_t> x(elementCount), residual(elementCount), gamma(spec.width), bias(spec.width);
    std::vector<uint16_t> parentOutput(elementCount), candidateOutput(elementCount);
    for (size_t i = 0; i < elementCount; ++i) {
        const float xv = 0.19f * std::sin(static_cast<float>((i * 17) % 997) * 0.013f);
        const float rv = 0.11f * std::cos(static_cast<float>((i * 29) % 991) * 0.017f);
        x[i] = Encode(xv, spec.dtype);
        residual[i] = Encode(rv, spec.dtype);
    }
    for (int col = 0; col < spec.width; ++col) {
        gamma[col] = Encode(0.85f + static_cast<float>(col % 23) * 0.002f, spec.dtype);
        bias[col] = Encode(-0.025f + static_cast<float>(col % 19) * 0.001f, spec.dtype);
    }

    DeviceBuffers buffers;
    Allocate(&buffers.x, dataBytes, "malloc x");
    Allocate(&buffers.residual, dataBytes, "malloc residual");
    Allocate(&buffers.gamma, paramBytes, "malloc gamma");
    Allocate(&buffers.bias, paramBytes, "malloc bias");
    Allocate(&buffers.parentOutput, dataBytes, "malloc parent output");
    Allocate(&buffers.candidateOutput, dataBytes, "malloc candidate output");
    CheckAcl(aclrtMemcpy(buffers.x, dataBytes, x.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy x");
    CheckAcl(aclrtMemcpy(buffers.residual, dataBytes, residual.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy residual");
    CheckAcl(aclrtMemcpy(buffers.gamma, paramBytes, gamma.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma");
    CheckAcl(aclrtMemcpy(buffers.bias, paramBytes, bias.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy bias");

    const int64_t dataShape[2] = {kRows, spec.width};
    const int64_t paramShape[1] = {spec.width};
    const TensorInfo dataInfo{dataShape, 2, spec.dtype};
    const TensorInfo paramInfo{paramShape, 1, spec.dtype};
    const TensorGroupInfo dataGroup{&dataInfo, 1};
    const TensorGroupInfo paramGroup{&paramInfo, 1};

    std::printf("CASE_BEGIN name=%s dtype=%s rows=%d width=%d device=%d warmup_each=%d pairs=%d samples_per_binary=%d\n",
                spec.name, spec.dtype == 1 ? "fp16" : "bf16", kRows, spec.width,
                kDevice, kWarmup, kPairs, kPairs);
    if (mode == RunMode::kQualifyV011) {
        for (int i = 0; i < kWarmup; ++i) {
            Launch(r31b_v011::run_kernel_v011, buffers.parentOutput, buffers,
                   dataGroup, paramGroup, stream);
            CheckAcl(aclrtSynchronizeStream(stream), "same-binary warmup synchronize");
        }

        std::vector<double> allDevice;
        std::vector<double> allWall;
        double blockMedians[kQualificationBlocks] = {};
        allDevice.reserve(kQualificationBlocks * kQualificationSamplesPerBlock);
        allWall.reserve(kQualificationBlocks * kQualificationSamplesPerBlock);
        for (int block = 0; block < kQualificationBlocks; ++block) {
            std::vector<double> blockDevice;
            std::vector<double> blockWall;
            blockDevice.reserve(kQualificationSamplesPerBlock);
            blockWall.reserve(kQualificationSamplesPerBlock);
            for (int sample = 0; sample < kQualificationSamplesPerBlock; ++sample) {
                const TimedSample timed = Measure(
                    r31b_v011::run_kernel_v011, buffers.parentOutput, buffers,
                    dataGroup, paramGroup, stream, events);
                blockDevice.push_back(timed.deviceUs);
                blockWall.push_back(timed.wallUs);
                allDevice.push_back(timed.deviceUs);
                allWall.push_back(timed.wallUs);
                std::printf("SAME_BINARY_SAMPLE case=%s block=%d sample=%02d device_us=%.3f wall_us=%.3f binary=V011\n",
                            spec.name, block + 1, sample + 1,
                            timed.deviceUs, timed.wallUs);
            }
            const Stats blockStats = Summarize(blockDevice);
            blockMedians[block] = blockStats.median;
            PrintStats(spec.name, "device_us", block == 0 ? "V011_BLOCK_1" : "V011_BLOCK_2",
                       blockDevice);
        }

        const Stats fullStats = Summarize(allDevice);
        const double madRatio = fullStats.median > 0.0
                                    ? fullStats.mad / fullStats.median
                                    : INFINITY;
        const double blockDrift = fullStats.median > 0.0
                                      ? std::fabs(blockMedians[0] - blockMedians[1]) /
                                            fullStats.median
                                      : INFINITY;
        const bool blocked = madRatio > 0.25 || blockDrift > 0.25;
        const bool passed = madRatio <= 0.10 && blockDrift <= 0.10;
        const char* verdict = passed ? "PASS" :
                              blocked ? "MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE" :
                                        "NEEDS_VALIDATION";
        PrintStats(spec.name, "device_us", "V011_ALL", allDevice);
        PrintStats(spec.name, "wall_us", "V011_ALL", allWall);
        std::printf("SAME_BINARY_QUAL case=%s result=%s samples=%zu median_us=%.3f mad_over_median=%.5f block_drift=%.5f\n",
                    spec.name, verdict, allDevice.size(), fullStats.median,
                    madRatio, blockDrift);

        CheckAcl(aclrtMemcpy(parentOutput.data(), dataBytes, buffers.parentOutput, dataBytes,
                             ACL_MEMCPY_DEVICE_TO_HOST), "copy same-binary output");
        CheckOutput(spec, x, residual, gamma, bias, parentOutput, "V011");
        std::printf("CASE_END name=%s same_binary=V011 correctness=PASS\n", spec.name);
        FreeCase(buffers);
        return;
    }

    for (int i = 0; i < kWarmup; ++i) {
        Launch(r31b_v011::run_kernel_v011, buffers.parentOutput, buffers, dataGroup, paramGroup, stream);
        CheckAcl(aclrtSynchronizeStream(stream), "parent warmup synchronize");
        Launch(r31b_v016::run_kernel_v016, buffers.candidateOutput, buffers, dataGroup, paramGroup, stream);
        CheckAcl(aclrtSynchronizeStream(stream), "candidate warmup synchronize");
    }

    std::vector<double> parentDevice, candidateDevice, parentWall, candidateWall, deltas;
    parentDevice.reserve(kPairs);
    candidateDevice.reserve(kPairs);
    parentWall.reserve(kPairs);
    candidateWall.reserve(kPairs);
    deltas.reserve(kPairs);
    for (int pair = 0; pair < kPairs; ++pair) {
        PairSample sample{};
        const bool parentFirst = (pair % 2) == 0;
        if (parentFirst) {
            sample.parent = Measure(r31b_v011::run_kernel_v011, buffers.parentOutput, buffers,
                                    dataGroup, paramGroup, stream, events);
            sample.candidate = Measure(r31b_v016::run_kernel_v016, buffers.candidateOutput, buffers,
                                       dataGroup, paramGroup, stream, events);
        } else {
            sample.candidate = Measure(r31b_v016::run_kernel_v016, buffers.candidateOutput, buffers,
                                       dataGroup, paramGroup, stream, events);
            sample.parent = Measure(r31b_v011::run_kernel_v011, buffers.parentOutput, buffers,
                                    dataGroup, paramGroup, stream, events);
        }
        sample.deltaUs = sample.candidate.deviceUs - sample.parent.deviceUs;
        parentDevice.push_back(sample.parent.deviceUs);
        candidateDevice.push_back(sample.candidate.deviceUs);
        parentWall.push_back(sample.parent.wallUs);
        candidateWall.push_back(sample.candidate.wallUs);
        deltas.push_back(sample.deltaUs);
        std::printf("SAMPLE case=%s pair=%02d order=%s parent_device_us=%.3f candidate_device_us=%.3f parent_wall_us=%.3f candidate_wall_us=%.3f paired_device_delta_us=%.3f\n",
                    spec.name, pair + 1, parentFirst ? "P,C" : "C,P",
                    sample.parent.deviceUs, sample.candidate.deviceUs,
                    sample.parent.wallUs, sample.candidate.wallUs, sample.deltaUs);
    }

    const Stats parentBlock = Summarize(parentDevice);
    const Stats candidateBlock = Summarize(candidateDevice);
    const Stats deltaBlock = Summarize(deltas);
    std::printf("PAIR_BLOCK case=%s block=1 pairs=%zu parent_median_us=%.3f candidate_median_us=%.3f paired_delta_median_us=%.3f\n",
                spec.name, deltas.size(), parentBlock.median,
                candidateBlock.median, deltaBlock.median);

    CheckAcl(aclrtMemcpy(parentOutput.data(), dataBytes, buffers.parentOutput, dataBytes,
                         ACL_MEMCPY_DEVICE_TO_HOST), "copy parent output");
    CheckAcl(aclrtMemcpy(candidateOutput.data(), dataBytes, buffers.candidateOutput, dataBytes,
                         ACL_MEMCPY_DEVICE_TO_HOST), "copy candidate output");
    CheckOutput(spec, x, residual, gamma, bias, parentOutput, "V011");
    CheckOutput(spec, x, residual, gamma, bias, candidateOutput, "V016");

    PrintStats(spec.name, "device_us", "V011", parentDevice);
    PrintStats(spec.name, "device_us", "V016", candidateDevice);
    PrintStats(spec.name, "wall_us", "V011", parentWall);
    PrintStats(spec.name, "wall_us", "V016", candidateWall);
    PrintStats(spec.name, "paired_device_delta_us", "V016_minus_V011", deltas);
    std::printf("CASE_END name=%s correctness=PASS\n", spec.name);
    FreeCase(buffers);
}

}  // namespace

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::fprintf(stderr, "usage: paired_runner_v016 --qualify-v011|--paired\n");
        return 1;
    }
    RunMode mode;
    if (std::strcmp(argv[1], "--qualify-v011") == 0) {
        mode = RunMode::kQualifyV011;
    } else if (std::strcmp(argv[1], "--paired") == 0) {
        mode = RunMode::kPaired;
    } else {
        std::fprintf(stderr, "unknown runner mode: %s\n", argv[1]);
        return 1;
    }

    CheckAcl(aclInit(nullptr), "aclInit");
    CheckAcl(aclrtSetDevice(kDevice), "set device");
    aclrtStream stream = nullptr;
    CheckAcl(aclrtCreateStream(&stream), "create stream");
    Events events;
    CheckAcl(aclrtCreateEvent(&events.start), "create start event");
    CheckAcl(aclrtCreateEvent(&events.stop), "create stop event");

    const CaseSpec cases[] = {
        {"fp16-tail-d12288", 1, 12288},
        {"fp16-wide-d32768", 1, 32768},
        {"bf16-tail-d12288", 2, 12288},
        {"bf16-wide-d32768", 2, 32768},
    };
    for (const CaseSpec& spec : cases) {
        RunCase(spec, stream, events, mode);
    }

    CheckAcl(aclrtDestroyEvent(events.stop), "destroy stop event");
    CheckAcl(aclrtDestroyEvent(events.start), "destroy start event");
    CheckAcl(aclrtDestroyStream(stream), "destroy stream");
    CheckAcl(aclrtResetDevice(kDevice), "reset device");
    CheckAcl(aclFinalize(), "aclFinalize");
    std::printf("RUNNER_COMPLETE mode=%s correctness=PASS cases=4 warmup_each=10 pairs_each=21 timing=DEVICE_EVENT_PRIMARY wall=DIAGNOSTIC\n",
                mode == RunMode::kQualifyV011 ? "QUALIFY_V011" : "PAIRED");
    return 0;
}
