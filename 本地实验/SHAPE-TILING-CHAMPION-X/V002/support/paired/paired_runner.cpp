// SHAPE-TILING-CHAMPION-X V001 paired timing runner.
// Device-event primary timing; wall-clock diagnostic only.
// Modes:
//   --same-binary   parent-only, 2 in-process blocks, qualification stats
//   --paired        interleaved parent/candidate pairs (P,C / C,P alternation)
//
// Golden output is reported only. Wide-FP32 golden disagreement is a known
// shared-baseline limitation (see WIDE-FP32-NONDETERMINISM.md); MAIN-1 has
// authorized latency P/C against parent regardless.

#include "paired_runner_abi.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

using StxRunKernel = void (*)(GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
                             GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
                             GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);

constexpr int kWarmup = 45;
constexpr int kPairs = 21;
constexpr int kQualificationBlocks = 2;
constexpr int kQualificationSamplesPerBlock = 31;
constexpr int kCoreCount = 8;
constexpr float kEpsilon = 1.0e-5f;

enum class RunMode { kSameBinary, kPaired };

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
    uint32_t sign = (static_cast<uint32_t>(value & 0x8000u)) << 16;
    uint32_t exponent = (value >> 10) & 0x1fu;
    uint32_t mantissa = value & 0x3ffu;
    uint32_t bits = 0;
    if (exponent == 0) {
        if (mantissa == 0) {
            bits = sign;
        } else {
            int32_t e = -14;
            while ((mantissa & 0x400u) == 0) {
                mantissa <<= 1;
                --e;
            }
            mantissa &= 0x3ffu;
            bits = sign | (static_cast<uint32_t>(e + 127) << 23) | (mantissa << 13);
        }
    } else if (exponent == 31) {
        bits = sign | 0x7f800000u | (mantissa << 13);
    } else {
        bits = sign | ((exponent - 1 + 127) << 23) | (mantissa << 13);
    }
    float out = 0.0f;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

uint16_t FloatToBfloat16(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    if ((bits & 0x7f800000u) == 0x7f800000u && (bits & 0x007fffffu) != 0u) {
        return static_cast<uint16_t>((bits >> 16) | 0x0040u);
    }
    bits += 0x7fffu + ((bits >> 16) & 1u);
    return static_cast<uint16_t>(bits >> 16);
}

float Bfloat16ToFloat(uint16_t value)
{
    const uint32_t bits = static_cast<uint32_t>(value) << 16;
    float out = 0.0f;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

size_t ElemBytes(int dtype)
{
    return dtype == 0 ? sizeof(float) : sizeof(uint16_t);
}

float Decode(const void* base, size_t index, int dtype)
{
    if (dtype == 0) {
        return static_cast<const float*>(base)[index];
    }
    const uint16_t raw = static_cast<const uint16_t*>(base)[index];
    return dtype == 1 ? HalfToFloat(raw) : Bfloat16ToFloat(raw);
}

void Encode(void* base, size_t index, float value, int dtype)
{
    if (dtype == 0) {
        static_cast<float*>(base)[index] = value;
        return;
    }
    static_cast<uint16_t*>(base)[index] =
        dtype == 1 ? FloatToHalf(value) : FloatToBfloat16(value);
}

void Allocate(void** pointer, size_t bytes, const char* label)
{
    CheckAcl(aclrtMalloc(pointer, bytes, ACL_MEM_MALLOC_HUGE_FIRST), label);
}

void FreeCase(DeviceBuffers& buffers)
{
    for (void** p : {&buffers.candidateOutput, &buffers.parentOutput, &buffers.bias,
                     &buffers.gamma, &buffers.residual, &buffers.x}) {
        if (*p != nullptr) {
            CheckAcl(aclrtFree(*p), "free case buffer");
            *p = nullptr;
        }
    }
}

void Launch(StxRunKernel kernel, void* output, const DeviceBuffers& buffers,
            const TensorGroupInfo& dataGroup, const TensorGroupInfo& paramGroup,
            const CaseSpec& spec, aclrtStream stream)
{
    kernel((GM_ADDR)buffers.x, dataGroup,
           (GM_ADDR)buffers.residual, dataGroup,
           (GM_ADDR)buffers.gamma, paramGroup,
           (GM_ADDR)buffers.bias, paramGroup,
           (GM_ADDR)output, dataGroup,
           kCoreCount, stream, kEpsilon);
}

TimedSample Measure(StxRunKernel kernel, void* output, const DeviceBuffers& buffers,
                    const TensorGroupInfo& dataGroup, const TensorGroupInfo& paramGroup,
                    const CaseSpec& spec, aclrtStream stream, Events& events)
{
    if (events.recorded) {
        CheckAcl(aclrtResetEvent(events.start, stream), "reset start event");
        CheckAcl(aclrtResetEvent(events.stop, stream), "reset stop event");
    }
    const auto wallStart = std::chrono::steady_clock::now();
    CheckAcl(aclrtRecordEvent(events.start, stream), "record start event");
    Launch(kernel, output, buffers, dataGroup, paramGroup, spec, stream);
    CheckAcl(aclrtRecordEvent(events.stop, stream), "record stop event");
    CheckAcl(aclrtSynchronizeEvent(events.stop), "synchronize stop event");
    const auto wallStop = std::chrono::steady_clock::now();
    events.recorded = true;

    float elapsedMs = 0.0f;
    CheckAcl(aclrtEventElapsedTime(&elapsedMs, events.start, events.stop), "event elapsed");
    TimedSample sample;
    sample.deviceUs = static_cast<double>(elapsedMs) * 1000.0;
    sample.wallUs = std::chrono::duration<double, std::micro>(wallStop - wallStart).count();
    return sample;
}

double Quantile(std::vector<double> sorted, double q)
{
    if (sorted.empty()) {
        return 0.0;
    }
    std::sort(sorted.begin(), sorted.end());
    const double pos = q * static_cast<double>(sorted.size() - 1);
    const size_t lo = static_cast<size_t>(pos);
    const size_t hi = std::min(lo + 1, sorted.size() - 1);
    const double frac = pos - static_cast<double>(lo);
    return sorted[lo] * (1.0 - frac) + sorted[hi] * frac;
}

Stats Summarize(const std::vector<double>& values)
{
    Stats s{};
    if (values.empty()) {
        return s;
    }
    std::vector<double> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    s.median = Quantile(sorted, 0.5);
    double sum = 0.0;
    for (double v : values) {
        sum += v;
    }
    s.mean = sum / static_cast<double>(values.size());
    double var = 0.0;
    for (double v : values) {
        var += (v - s.mean) * (v - s.mean);
    }
    s.stdev = std::sqrt(var / static_cast<double>(values.size()));
    s.cv = s.mean == 0.0 ? 0.0 : s.stdev / s.mean;
    s.minimum = sorted.front();
    s.maximum = sorted.back();
    s.maxMin = s.minimum == 0.0 ? 0.0 : s.maximum / s.minimum;
    std::vector<double> deviations(values.size());
    for (size_t i = 0; i < values.size(); ++i) {
        deviations[i] = std::fabs(values[i] - s.median);
    }
    s.mad = Quantile(deviations, 0.5);
    s.p10 = Quantile(sorted, 0.10);
    s.p90 = Quantile(sorted, 0.90);
    s.spread = s.maximum - s.minimum;
    std::vector<double> core;
    for (double v : values) {
        if (s.median > 0.0 && std::fabs(v - s.median) <= 0.2 * s.median) {
            core.push_back(v);
        }
    }
    if (core.size() >= 2) {
        double csum = 0.0;
        for (double v : core) {
            csum += v;
        }
        const double cmean = csum / static_cast<double>(core.size());
        double cvar = 0.0;
        for (double v : core) {
            cvar += (v - cmean) * (v - cmean);
        }
        s.coreCv = cmean == 0.0 ? 0.0 : std::sqrt(cvar / static_cast<double>(core.size())) / cmean;
    }
    return s;
}

void PrintStats(const char* caseName, const char* metric, const char* binary,
                const std::vector<double>& values)
{
    const Stats s = Summarize(values);
    std::printf("SUMMARY case=%s metric=%s binary=%s n=%zu median=%.3f mean=%.3f stdev=%.3f cv=%.5f min=%.3f max=%.3f max_min=%.3f mad=%.3f p10=%.3f p90=%.3f spread=%.3f core_cv=%.5f\n",
                caseName, metric, binary, values.size(), s.median, s.mean, s.stdev, s.cv,
                s.minimum, s.maximum, s.maxMin, s.mad, s.p10, s.p90, s.spread, s.coreCv);
}

void ReportGolden(const CaseSpec& spec, const std::vector<uint8_t>& x,
                  const std::vector<uint8_t>& residual, const std::vector<uint8_t>& gamma,
                  const std::vector<uint8_t>& bias, const std::vector<uint8_t>& output,
                  const char* binary)
{
    const size_t width = static_cast<size_t>(spec.width);
    const size_t rows = static_cast<size_t>(spec.rows);
    float maxAbs = 0.0f;
    size_t failures = 0;
    const float tolerance = spec.dtype == 0 ? 1.0e-4f : (spec.dtype == 1 ? 0.004f : 0.02f);
    for (size_t row = 0; row < rows; ++row) {
        double squareSum = 0.0;
        std::vector<float> y(width);
        for (size_t col = 0; col < width; ++col) {
            const size_t index = row * width + col;
            float value = Decode(x.data(), index, spec.dtype) +
                          Decode(residual.data(), index, spec.dtype);
            if (spec.dtype == 1) {
                const uint16_t raw = FloatToHalf(value);
                value = HalfToFloat(raw);
            }
            y[col] = value;
            squareSum += static_cast<double>(value) * static_cast<double>(value);
        }
        const float invRms = static_cast<float>(
            1.0 / std::sqrt(squareSum / static_cast<double>(width) + static_cast<double>(kEpsilon)));
        for (size_t col = 0; col < width; ++col) {
            const size_t index = row * width + col;
            float expected = y[col] * invRms;
            const float gammaCol = Decode(gamma.data(), col, spec.dtype);
            const float biasCol = Decode(bias.data(), col, spec.dtype);
            if (spec.dtype == 1) {
                expected = HalfToFloat(FloatToHalf(expected));
                expected = HalfToFloat(FloatToHalf(expected * gammaCol));
                expected = HalfToFloat(FloatToHalf(expected + biasCol));
            } else if (spec.dtype == 2) {
                expected = Bfloat16ToFloat(FloatToBfloat16(expected * gammaCol + biasCol));
            } else {
                expected = expected * gammaCol + biasCol;
            }
            const float actual = Decode(output.data(), index, spec.dtype);
            const float absError = std::fabs(actual - expected);
            maxAbs = std::max(maxAbs, absError);
            if (absError > tolerance + tolerance * std::fabs(expected)) {
                ++failures;
            }
        }
    }
    std::printf("GOLDEN case=%s binary=%s failures=%zu max_abs=%.8g note=REFERENCE_ONLY\n",
                spec.name, binary, failures, maxAbs);
}

void RunCase(const CaseSpec& spec, aclrtStream stream, Events& events, RunMode mode)
{
    const size_t rows = static_cast<size_t>(spec.rows);
    const size_t width = static_cast<size_t>(spec.width);
    const size_t elementCount = rows * width;
    const size_t elemBytes = ElemBytes(spec.dtype);
    const size_t dataBytes = elementCount * elemBytes;
    const size_t paramBytes = width * elemBytes;

    std::vector<uint8_t> x(dataBytes), residual(dataBytes), gamma(paramBytes), bias(paramBytes);
    std::vector<uint8_t> parentOutput(dataBytes), candidateOutput(dataBytes);
    for (size_t i = 0; i < elementCount; ++i) {
        const float xv = 0.19f * std::sin(static_cast<float>((i * 17) % 997) * 0.013f);
        const float rv = 0.11f * std::cos(static_cast<float>((i * 29) % 991) * 0.017f);
        Encode(x.data(), i, xv, spec.dtype);
        Encode(residual.data(), i, rv, spec.dtype);
    }
    for (size_t col = 0; col < width; ++col) {
        Encode(gamma.data(), col, 0.85f + static_cast<float>(col % 23) * 0.002f, spec.dtype);
        Encode(bias.data(), col, -0.025f + static_cast<float>(col % 19) * 0.001f, spec.dtype);
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

    const int64_t dataShape[2] = {spec.rows, spec.width};
    const int64_t paramShape[1] = {spec.width};
    const TensorInfo dataInfo{dataShape, 2, spec.dtype};
    const TensorInfo paramInfo{paramShape, 1, spec.dtype};
    const TensorGroupInfo dataGroup{&dataInfo, 1};
    const TensorGroupInfo paramGroup{&paramInfo, 1};

    std::printf("CASE_BEGIN name=%s dtype=%d rows=%d width=%d warmup=%d pairs=%d mode=%s\n",
                spec.name, spec.dtype, spec.rows, spec.width, kWarmup, kPairs,
                mode == RunMode::kSameBinary ? "same-binary" : "paired");

    if (mode == RunMode::kSameBinary) {
        for (int i = 0; i < kWarmup; ++i) {
            Launch(run_kernel_parent, buffers.parentOutput, buffers, dataGroup, paramGroup,
                   spec, stream);
            CheckAcl(aclrtSynchronizeStream(stream), "same-binary warmup synchronize");
        }
        std::vector<double> allDevice;
        std::vector<double> allWall;
        double blockMedians[kQualificationBlocks] = {};
        allDevice.reserve(kQualificationBlocks * kQualificationSamplesPerBlock);
        for (int block = 0; block < kQualificationBlocks; ++block) {
            std::vector<double> blockDevice;
            std::vector<double> blockWall;
            for (int sample = 0; sample < kQualificationSamplesPerBlock; ++sample) {
                const TimedSample timed = Measure(run_kernel_parent, buffers.parentOutput, buffers,
                                                  dataGroup, paramGroup, spec, stream, events);
                blockDevice.push_back(timed.deviceUs);
                blockWall.push_back(timed.wallUs);
                allDevice.push_back(timed.deviceUs);
                allWall.push_back(timed.wallUs);
                std::printf("SAME_BINARY_SAMPLE case=%s block=%d sample=%02d device_us=%.3f wall_us=%.3f\n",
                            spec.name, block + 1, sample + 1, timed.deviceUs, timed.wallUs);
            }
            blockMedians[block] = Summarize(blockDevice).median;
            PrintStats(spec.name, "device_us", block == 0 ? "PARENT_BLOCK_1" : "PARENT_BLOCK_2",
                       blockDevice);
        }
        const Stats fullStats = Summarize(allDevice);
        const double madRatio = fullStats.median > 0.0 ? fullStats.mad / fullStats.median : 1e9;
        const double blockDrift = fullStats.median > 0.0
                                      ? std::fabs(blockMedians[0] - blockMedians[1]) / fullStats.median
                                      : 1e9;
        const bool blocked = madRatio > 0.25 || blockDrift > 0.25;
        const bool passed = madRatio <= 0.10 && blockDrift <= 0.10;
        const char* verdict = passed ? "PASS" : (blocked ? "MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE"
                                                         : "NEEDS_VALIDATION");
        PrintStats(spec.name, "device_us", "PARENT_ALL", allDevice);
        PrintStats(spec.name, "wall_us", "PARENT_ALL", allWall);
        std::printf("SAME_BINARY_QUAL case=%s result=%s samples=%zu median_us=%.3f mad_over_median=%.5f block_drift=%.5f\n",
                    spec.name, verdict, allDevice.size(), fullStats.median, madRatio, blockDrift);

        CheckAcl(aclrtMemcpy(parentOutput.data(), dataBytes, buffers.parentOutput, dataBytes,
                             ACL_MEMCPY_DEVICE_TO_HOST), "copy same-binary output");
        ReportGolden(spec, x, residual, gamma, bias, parentOutput, "PARENT");
        std::printf("CASE_END name=%s same_binary=%s\n", spec.name, verdict);
        FreeCase(buffers);
        return;
    }

    for (int i = 0; i < kWarmup; ++i) {
        Launch(run_kernel_parent, buffers.parentOutput, buffers, dataGroup, paramGroup, spec, stream);
        CheckAcl(aclrtSynchronizeStream(stream), "parent warmup synchronize");
        Launch(run_kernel_candidate, buffers.candidateOutput, buffers, dataGroup, paramGroup,
               spec, stream);
        CheckAcl(aclrtSynchronizeStream(stream), "candidate warmup synchronize");
    }

    std::vector<double> parentDevice, candidateDevice, parentWall, candidateWall, deltas;
    for (int pair = 0; pair < kPairs; ++pair) {
        PairSample sample{};
        const bool parentFirst = (pair % 2) == 0;
        if (parentFirst) {
            sample.parent = Measure(run_kernel_parent, buffers.parentOutput, buffers, dataGroup,
                                    paramGroup, spec, stream, events);
            sample.candidate = Measure(run_kernel_candidate, buffers.candidateOutput, buffers,
                                       dataGroup, paramGroup, spec, stream, events);
        } else {
            sample.candidate = Measure(run_kernel_candidate, buffers.candidateOutput, buffers,
                                       dataGroup, paramGroup, spec, stream, events);
            sample.parent = Measure(run_kernel_parent, buffers.parentOutput, buffers, dataGroup,
                                    paramGroup, spec, stream, events);
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
    std::printf("PAIR_BLOCK case=%s pairs=%zu parent_median_us=%.3f candidate_median_us=%.3f paired_delta_median_us=%.3f\n",
                spec.name, deltas.size(), parentBlock.median, candidateBlock.median,
                deltaBlock.median);

    CheckAcl(aclrtMemcpy(parentOutput.data(), dataBytes, buffers.parentOutput, dataBytes,
                         ACL_MEMCPY_DEVICE_TO_HOST), "copy parent output");
    CheckAcl(aclrtMemcpy(candidateOutput.data(), dataBytes, buffers.candidateOutput, dataBytes,
                         ACL_MEMCPY_DEVICE_TO_HOST), "copy candidate output");
    ReportGolden(spec, x, residual, gamma, bias, parentOutput, "PARENT");
    ReportGolden(spec, x, residual, gamma, bias, candidateOutput, "CANDIDATE");

    PrintStats(spec.name, "device_us", "PARENT", parentDevice);
    PrintStats(spec.name, "device_us", "CANDIDATE", candidateDevice);
    PrintStats(spec.name, "wall_us", "PARENT", parentWall);
    PrintStats(spec.name, "wall_us", "CANDIDATE", candidateWall);
    PrintStats(spec.name, "paired_device_delta_us", "CANDIDATE_minus_PARENT", deltas);
    std::printf("CASE_END name=%s\n", spec.name);
    FreeCase(buffers);
}

}  // namespace

int main(int argc, char** argv)
{
    if (argc < 2) {
        std::fprintf(stderr, "usage: paired_runner --same-binary|--paired [device] [--multiscale]\n");
        return 1;
    }
    RunMode mode;
    if (std::strcmp(argv[1], "--same-binary") == 0) {
        mode = RunMode::kSameBinary;
    } else if (std::strcmp(argv[1], "--paired") == 0) {
        mode = RunMode::kPaired;
    } else {
        std::fprintf(stderr, "unknown runner mode: %s\n", argv[1]);
        return 1;
    }
    const int device = argc > 2 ? std::atoi(argv[2]) : 4;
    bool multiscale = false;
    for (int i = 3; i < argc; ++i) {
        if (std::strcmp(argv[i], "--multiscale") == 0) {
            multiscale = true;
        }
    }

    CheckAcl(aclInit(nullptr), "aclInit");
    CheckAcl(aclrtSetDevice(device), "set device");
    aclrtStream stream = nullptr;
    CheckAcl(aclrtCreateStream(&stream), "create stream");
    Events events;
    CheckAcl(aclrtCreateEvent(&events.start), "create start event");
    CheckAcl(aclrtCreateEvent(&events.stop), "create stop event");

    // MAIN-1 V002 priority: change-domain FP32 D=32768; blanks are the
    // bit-identical shapes that establish the noise band.
    const CaseSpec closedLoopCases[] = {
        {"fp32-change-d32768-r2", 0, 2, 32768},
        {"fp32-change-d32768-r8", 0, 8, 32768},
        {"fp32-blank-d16384", 0, 2, 16384},
        {"fp32-blank-d8192", 0, 2, 8192},
        {"fp32-blank-d12288", 0, 2, 12288},
        {"fp16-blank-d32768", 1, 2, 32768},
        {"bf16-blank-d32768", 2, 2, 32768},
    };
    // MAIN-1 instruction A: multi-row supplement on the same V002 binary.
    // Change domain is D=32768 at several row counts; blanks re-measure the
    // noise band on identical code at the matching row counts.
    const CaseSpec multiscaleCases[] = {
        {"fp32-change-d32768-r2", 0, 2, 32768},
        {"fp32-change-d32768-r8", 0, 8, 32768},
        {"fp32-change-d32768-r16", 0, 16, 32768},
        {"fp32-blank-d16384-r8", 0, 8, 16384},
        {"fp32-blank-d16384-r2", 0, 2, 16384},
    };
    const CaseSpec* cases = multiscale ? multiscaleCases : closedLoopCases;
    const size_t caseCount = multiscale
        ? sizeof(multiscaleCases) / sizeof(multiscaleCases[0])
        : sizeof(closedLoopCases) / sizeof(closedLoopCases[0]);
    for (size_t i = 0; i < caseCount; ++i) {
        RunCase(cases[i], stream, events, mode);
    }

    CheckAcl(aclrtDestroyEvent(events.stop), "destroy stop event");
    CheckAcl(aclrtDestroyEvent(events.start), "destroy start event");
    CheckAcl(aclrtDestroyStream(stream), "destroy stream");
    CheckAcl(aclrtResetDevice(device), "reset device");
    CheckAcl(aclFinalize(), "aclFinalize");
    std::printf("RUNNER_COMPLETE mode=%s device=%d cases=%zu multiscale=%d warmup_each=%d pairs_each=%d timing=DEVICE_EVENT_PRIMARY wall=DIAGNOSTIC golden=REFERENCE_ONLY\n",
                mode == RunMode::kSameBinary ? "SAME_BINARY" : "PAIRED", device, caseCount,
                multiscale ? 1 : 0, kWarmup, kPairs);
    return 0;
}
