#include "local_abi_shim.h"

#include <acl/acl.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <numeric>
#include <string>
#include <vector>

extern "C" void sync_parent_run_kernel(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);
extern "C" void sync_candidate_run_kernel(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);

namespace {

constexpr int kDevice = 4;
constexpr int64_t kRows = 2;
constexpr int64_t kWidth = 12288;
constexpr int kFp16 = 1;
constexpr int kWarmups = 45;
constexpr int kPrecheckSamples = 21;
constexpr int kQualificationSamples = 31;
constexpr int kPairGroups = 4;
constexpr int kPairsPerGroup = 11;
constexpr float kEpsilon = 1.0e-5f;
constexpr float kAtol = 0.001953125f;
constexpr float kRtol = 0.001953125f;
constexpr float kMaxAbsError = 0.1f;
constexpr char kParentSha[] =
    "a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3";
constexpr char kCandidateSha[] =
    "27c853e1cb0c47307899b874338c3aecb912ad5dc125c0d67f6afba76a9117ec";

using KernelFn = void (*)(GM_ADDR, const TensorGroupInfo&, GM_ADDR,
                          const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
                          GM_ADDR, const TensorGroupInfo&, GM_ADDR,
                          const TensorGroupInfo&, int64_t, aclrtStream, float);

struct DeviceBuffers {
    void* x = nullptr;
    void* residual = nullptr;
    void* gamma = nullptr;
    void* bias = nullptr;
    void* parentOutput = nullptr;
    void* candidateOutput = nullptr;

    ~DeviceBuffers()
    {
        if (candidateOutput) aclrtFree(candidateOutput);
        if (parentOutput) aclrtFree(parentOutput);
        if (bias) aclrtFree(bias);
        if (gamma) aclrtFree(gamma);
        if (residual) aclrtFree(residual);
        if (x) aclrtFree(x);
    }
};

struct TimedSample {
    double deviceUs = 0.0;
    double wallUs = 0.0;
};

struct Stats {
    double median = 0.0;
    double mean = 0.0;
    double stdev = 0.0;
    double cv = 0.0;
    double minimum = 0.0;
    double maximum = 0.0;
    double maxMin = 0.0;
    double mad = 0.0;
    double p10 = 0.0;
    double p90 = 0.0;
    double spread = 0.0;
};

void CheckAcl(aclError status, const char* operation)
{
    if (status != ACL_SUCCESS) {
        std::fprintf(stderr, "ACL_FAIL %s=%d %s\n", operation,
                     static_cast<int>(status),
                     aclGetRecentErrMsg() ? aclGetRecentErrMsg() : "");
        std::exit(2);
    }
}

double Quantile(const std::vector<double>& values, double q)
{
    std::vector<double> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    const double index = q * static_cast<double>(sorted.size() - 1);
    const size_t low = static_cast<size_t>(index);
    const size_t high = std::min(low + 1, sorted.size() - 1);
    const double fraction = index - static_cast<double>(low);
    return sorted[low] * (1.0 - fraction) + sorted[high] * fraction;
}

double Median(const std::vector<double>& values)
{
    return Quantile(values, 0.5);
}

Stats Summarize(const std::vector<double>& values)
{
    Stats result{};
    if (values.empty()) return result;
    result.median = Median(values);
    result.mean = std::accumulate(values.begin(), values.end(), 0.0) /
                  static_cast<double>(values.size());
    double squared = 0.0;
    std::vector<double> deviations;
    deviations.reserve(values.size());
    for (double value : values) {
        const double difference = value - result.mean;
        squared += difference * difference;
        deviations.push_back(std::fabs(value - result.median));
    }
    result.stdev = values.size() > 1
        ? std::sqrt(squared / static_cast<double>(values.size() - 1)) : 0.0;
    result.cv = result.mean != 0.0 ? result.stdev / result.mean : INFINITY;
    const auto bounds = std::minmax_element(values.begin(), values.end());
    result.minimum = *bounds.first;
    result.maximum = *bounds.second;
    result.maxMin = result.minimum > 0.0 ? result.maximum / result.minimum : INFINITY;
    result.mad = Median(deviations);
    result.p10 = Quantile(values, 0.10);
    result.p90 = Quantile(values, 0.90);
    result.spread = result.maximum - result.minimum;
    return result;
}

void WriteStats(std::ostream& out, const char* label,
                const std::vector<double>& values)
{
    const Stats s = Summarize(values);
    out << std::fixed << std::setprecision(6)
        << "stats\t" << label
        << "\tcount=" << values.size()
        << "\tmedian=" << s.median
        << "\tmean=" << s.mean
        << "\tstdev=" << s.stdev
        << "\tcv=" << s.cv
        << "\tmin=" << s.minimum
        << "\tmax=" << s.maximum
        << "\tmax_min=" << s.maxMin
        << "\tmad=" << s.mad
        << "\tp10=" << s.p10
        << "\tp90=" << s.p90
        << "\tspan=" << s.spread << '\n';
}

uint16_t ToHalfBits(float value)
{
    const aclFloat16 halfValue = aclFloatToFloat16(value);
    uint16_t bits = 0;
    std::memcpy(&bits, &halfValue, sizeof(bits));
    return bits;
}

float FromHalfBits(uint16_t bits)
{
    aclFloat16 halfValue;
    std::memcpy(&halfValue, &bits, sizeof(bits));
    return aclFloat16ToFloat(halfValue);
}

float InputValue(int64_t index, int multiplier, int offset)
{
    return static_cast<float>((index * multiplier + offset) % 997) / 498.0f - 1.0f;
}

bool Allocate(void** pointer, size_t bytes, const char* name)
{
    const aclError status = aclrtMalloc(pointer, bytes, ACL_MEM_MALLOC_HUGE_FIRST);
    if (status != ACL_SUCCESS) {
        std::fprintf(stderr, "ACL_FAIL aclrtMalloc(%s)=%d\n", name,
                     static_cast<int>(status));
        return false;
    }
    return true;
}

bool CreateBuffers(DeviceBuffers& buffers, std::vector<uint16_t>& xHost,
                   std::vector<uint16_t>& residualHost,
                   std::vector<uint16_t>& gammaHost,
                   std::vector<uint16_t>& biasHost)
{
    const size_t elements = static_cast<size_t>(kRows * kWidth);
    const size_t dataBytes = elements * sizeof(uint16_t);
    const size_t paramBytes = static_cast<size_t>(kWidth) * sizeof(uint16_t);
    xHost.resize(elements);
    residualHost.resize(elements);
    gammaHost.resize(static_cast<size_t>(kWidth));
    biasHost.resize(static_cast<size_t>(kWidth));
    for (size_t i = 0; i < elements; ++i) {
        xHost[i] = ToHalfBits(InputValue(static_cast<int64_t>(i), 3, 1));
        residualHost[i] = ToHalfBits(InputValue(static_cast<int64_t>(i), 5, 7));
    }
    for (int64_t col = 0; col < kWidth; ++col) {
        const size_t i = static_cast<size_t>(col);
        gammaHost[i] = ToHalfBits(InputValue(col, 11, 3) * 0.1f + 1.0f);
        biasHost[i] = ToHalfBits(InputValue(col, 13, 5) * 0.05f);
    }
    if (!Allocate(&buffers.x, dataBytes, "x") ||
        !Allocate(&buffers.residual, dataBytes, "residual") ||
        !Allocate(&buffers.gamma, paramBytes, "gamma") ||
        !Allocate(&buffers.bias, paramBytes, "bias") ||
        !Allocate(&buffers.parentOutput, dataBytes, "parent output") ||
        !Allocate(&buffers.candidateOutput, dataBytes, "candidate output")) {
        return false;
    }
    CheckAcl(aclrtMemcpy(buffers.x, dataBytes, xHost.data(), dataBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy x");
    CheckAcl(aclrtMemcpy(buffers.residual, dataBytes, residualHost.data(), dataBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy residual");
    CheckAcl(aclrtMemcpy(buffers.gamma, paramBytes, gammaHost.data(), paramBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma");
    CheckAcl(aclrtMemcpy(buffers.bias, paramBytes, biasHost.data(), paramBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy bias");
    return true;
}

void Launch(KernelFn kernel, void* output, const DeviceBuffers& buffers,
            const TensorGroupInfo& dataInfo, const TensorGroupInfo& paramInfo,
            aclrtStream stream)
{
    kernel((GM_ADDR)buffers.x, dataInfo,
           (GM_ADDR)buffers.residual, dataInfo,
           (GM_ADDR)buffers.gamma, paramInfo,
           (GM_ADDR)buffers.bias, paramInfo,
           (GM_ADDR)output, dataInfo, 1, stream, kEpsilon);
}

TimedSample Measure(KernelFn kernel, void* output, const DeviceBuffers& buffers,
                    const TensorGroupInfo& dataInfo,
                    const TensorGroupInfo& paramInfo, aclrtStream stream,
                    aclrtEvent startEvent, aclrtEvent stopEvent,
                    bool& eventsRecorded)
{
    if (eventsRecorded) {
        CheckAcl(aclrtResetEvent(startEvent, stream), "reset start event");
        CheckAcl(aclrtResetEvent(stopEvent, stream), "reset stop event");
    }
    const auto wallStart = std::chrono::steady_clock::now();
    CheckAcl(aclrtRecordEvent(startEvent, stream), "record start event");
    Launch(kernel, output, buffers, dataInfo, paramInfo, stream);
    CheckAcl(aclrtRecordEvent(stopEvent, stream), "record stop event");
    CheckAcl(aclrtSynchronizeEvent(stopEvent), "synchronize stop event");
    const auto wallStop = std::chrono::steady_clock::now();
    float elapsedMs = 0.0f;
    CheckAcl(aclrtEventElapsedTime(&elapsedMs, startEvent, stopEvent),
             "read event elapsed time");
    eventsRecorded = true;
    return {static_cast<double>(elapsedMs) * 1000.0,
            std::chrono::duration<double, std::micro>(wallStop - wallStart).count()};
}

bool CheckOutput(const std::vector<uint16_t>& x,
                 const std::vector<uint16_t>& residual,
                 const std::vector<uint16_t>& gamma,
                 const std::vector<uint16_t>& bias,
                 const std::vector<uint16_t>& output, const char* label)
{
    const size_t elements = static_cast<size_t>(kRows * kWidth);
    float maxAbs = 0.0f;
    size_t matched = 0;
    for (int64_t row = 0; row < kRows; ++row) {
        float squareSum = 0.0f;
        for (int64_t col = 0; col < kWidth; ++col) {
            const size_t i = static_cast<size_t>(row * kWidth + col);
            const float y = FromHalfBits(ToHalfBits(
                FromHalfBits(x[i]) + FromHalfBits(residual[i])));
            squareSum += y * y;
        }
        const float invRms = 1.0f / std::sqrt(
            squareSum / static_cast<float>(kWidth) + kEpsilon);
        for (int64_t col = 0; col < kWidth; ++col) {
            const size_t i = static_cast<size_t>(row * kWidth + col);
            const float y = FromHalfBits(ToHalfBits(
                FromHalfBits(x[i]) + FromHalfBits(residual[i])));
            const float scaled = FromHalfBits(ToHalfBits(y * invRms));
            const float affine = FromHalfBits(ToHalfBits(
                scaled * FromHalfBits(gamma[static_cast<size_t>(col)])));
            const float expected = FromHalfBits(ToHalfBits(
                affine + FromHalfBits(bias[static_cast<size_t>(col)])));
            const float actual = FromHalfBits(output[i]);
            const float error = std::fabs(actual - expected);
            maxAbs = std::max(maxAbs, error);
            if (error <= kAtol + kRtol * std::fabs(expected)) ++matched;
        }
    }
    const double ratio = static_cast<double>(matched) / static_cast<double>(elements);
    const bool pass = ratio >= 0.99 && maxAbs <= kMaxAbsError;
    std::printf("CORRECTNESS binary=%s matched_ratio=%.8f max_abs=%.8g result=%s\n",
                label, ratio, maxAbs, pass ? "PASS" : "FAIL");
    return pass;
}

void Warmup(KernelFn kernel, void* output, const DeviceBuffers& buffers,
            const TensorGroupInfo& dataInfo, const TensorGroupInfo& paramInfo,
            aclrtStream stream, int count, const char* label)
{
    for (int i = 0; i < count; ++i) {
        Launch(kernel, output, buffers, dataInfo, paramInfo, stream);
        CheckAcl(aclrtSynchronizeStream(stream), "warmup stream synchronize");
    }
    std::printf("WARMUP binary=%s launches=%d synchronized=1\n", label, count);
}

bool OutputPathsAvailable(const char* prefix)
{
    const std::string rawPath = std::string(prefix) + "-raw.tsv";
    const std::string jitterPath = std::string(prefix) + "-jitter.txt";
    std::ifstream rawProbe(rawPath);
    std::ifstream jitterProbe(jitterPath);
    if (rawProbe.good() || jitterProbe.good()) {
        std::fprintf(stderr, "OUTPUT_EXISTS prefix=%s\n", prefix);
        return false;
    }
    return true;
}

bool RunPrecheck(const char* block, int rep, const char* prefix,
                 DeviceBuffers& buffers, const TensorGroupInfo& dataInfo,
                 const TensorGroupInfo& paramInfo, aclrtStream stream,
                 aclrtEvent startEvent, aclrtEvent stopEvent,
                 const std::vector<uint16_t>& xHost,
                 const std::vector<uint16_t>& residualHost,
                 const std::vector<uint16_t>& gammaHost,
                 const std::vector<uint16_t>& biasHost)
{
    if (!OutputPathsAvailable(prefix)) return false;
    const std::string rawPath = std::string(prefix) + "-raw.tsv";
    const std::string jitterPath = std::string(prefix) + "-jitter.txt";
    std::ofstream raw(rawPath);
    std::ofstream jitter(jitterPath);
    if (!raw || !jitter) {
        std::fprintf(stderr, "OUTPUT_OPEN_FAIL prefix=%s\n", prefix);
        return false;
    }
    Warmup(sync_parent_run_kernel, buffers.parentOutput, buffers, dataInfo,
           paramInfo, stream, kWarmups, "R31B_V011");
    raw << "phase\tprecheck\trep\tsample\tvariant\tdevice_us\twall_us\n";
    jitter << "phase=WINDOW_PRECHECK device=4 shape=2x12288 dtype=FP16 warmup=45 samples=21\n";
    bool eventsRecorded = false;
    std::vector<double> device, wall;
    for (int sample = 0; sample < kPrecheckSamples; ++sample) {
        const TimedSample t = Measure(sync_parent_run_kernel, buffers.parentOutput,
            buffers, dataInfo, paramInfo, stream, startEvent, stopEvent, eventsRecorded);
        raw << "WINDOW_PRECHECK\t" << block << '\t' << rep << '\t'
            << sample + 1 << "\tPARENT\t" << std::fixed << std::setprecision(6)
            << t.deviceUs << '\t' << t.wallUs << '\n';
        std::printf("PRECHECK_SAMPLE block=%s rep=%d sample=%d device_us=%.3f wall_us=%.3f\n",
                    block, rep, sample + 1, t.deviceUs, t.wallUs);
        device.push_back(t.deviceUs);
        wall.push_back(t.wallUs);
    }
    WriteStats(jitter, "parent_device_us", device);
    WriteStats(jitter, "parent_wall_us", wall);
    const Stats s = Summarize(device);
    std::printf("PRECHECK_REP block=%s rep=%d samples=%zu median_device_us=%.6f cv=%.6f max_min=%.6f\n",
                block, rep, device.size(), s.median, s.cv, s.maxMin);

    std::vector<uint16_t> output(xHost.size());
    CheckAcl(aclrtMemcpy(output.data(), output.size() * sizeof(uint16_t),
                         buffers.parentOutput, output.size() * sizeof(uint16_t),
                         ACL_MEMCPY_DEVICE_TO_HOST), "copy precheck output");
    const bool correct = CheckOutput(xHost, residualHost, gammaHost, biasHost,
                                     output, "R31B_V011");
    return raw.good() && jitter.good() && correct;
}

bool RunSameBinary(const char* prefix, DeviceBuffers& buffers,
                   const TensorGroupInfo& dataInfo,
                   const TensorGroupInfo& paramInfo, aclrtStream stream,
                   aclrtEvent startEvent, aclrtEvent stopEvent,
                   const std::vector<uint16_t>& xHost,
                   const std::vector<uint16_t>& residualHost,
                   const std::vector<uint16_t>& gammaHost,
                   const std::vector<uint16_t>& biasHost)
{
    if (!OutputPathsAvailable(prefix)) return false;
    const std::string rawPath = std::string(prefix) + "-raw.tsv";
    const std::string jitterPath = std::string(prefix) + "-jitter.txt";
    std::ofstream raw(rawPath);
    std::ofstream jitter(jitterPath);
    if (!raw || !jitter) {
        std::fprintf(stderr, "OUTPUT_OPEN_FAIL prefix=%s\n", prefix);
        return false;
    }
    Warmup(sync_parent_run_kernel, buffers.parentOutput, buffers, dataInfo,
           paramInfo, stream, kWarmups, "R31B_V011");
    raw << "phase\tblock\tsample\tvariant\tdevice_us\twall_us\n";
    jitter << "phase=SAME_BINARY device=4 shape=2x12288 dtype=FP16 warmup=45 samples_per_block=31\n";
    bool eventsRecorded = false;
    std::vector<double> allDevice, allWall;
    std::vector<double> blockMedians;
    for (int block = 1; block <= 2; ++block) {
        std::vector<double> device, wall;
        for (int sample = 0; sample < kQualificationSamples; ++sample) {
            const TimedSample t = Measure(sync_parent_run_kernel, buffers.parentOutput,
                buffers, dataInfo, paramInfo, stream, startEvent, stopEvent, eventsRecorded);
            raw << "SAME_BINARY\t" << block << '\t' << sample + 1
                << "\tPARENT\t" << std::fixed << std::setprecision(6)
                << t.deviceUs << '\t' << t.wallUs << '\n';
            device.push_back(t.deviceUs);
            wall.push_back(t.wallUs);
            allDevice.push_back(t.deviceUs);
            allWall.push_back(t.wallUs);
        }
        const Stats s = Summarize(device);
        blockMedians.push_back(s.median);
        WriteStats(jitter, block == 1 ? "parent_device_block_1" : "parent_device_block_2", device);
        WriteStats(jitter, block == 1 ? "parent_wall_block_1" : "parent_wall_block_2", wall);
    }
    const Stats all = Summarize(allDevice);
    const double madRatio = all.median > 0.0 ? all.mad / all.median : INFINITY;
    const double drift = all.median > 0.0
        ? std::fabs(blockMedians[0] - blockMedians[1]) / all.median : INFINITY;
    const bool pass = madRatio <= 0.10 && drift <= 0.10;
    WriteStats(jitter, "parent_device_all", allDevice);
    WriteStats(jitter, "parent_wall_all", allWall);
    jitter << std::fixed << std::setprecision(6)
           << "same_binary_result=" << (pass ? "PASS" : "FAIL")
           << "\tmad_over_median=" << madRatio << "\tblock_drift=" << drift << '\n';
    std::printf("SAME_BINARY_RESULT=%s samples=%zu median_device_us=%.6f mad_over_median=%.6f block_drift=%.6f\n",
                pass ? "PASS" : "FAIL", allDevice.size(), all.median, madRatio, drift);
    std::vector<uint16_t> output(xHost.size());
    CheckAcl(aclrtMemcpy(output.data(), output.size() * sizeof(uint16_t),
                         buffers.parentOutput, output.size() * sizeof(uint16_t),
                         ACL_MEMCPY_DEVICE_TO_HOST), "copy same-binary output");
    const bool correct = CheckOutput(xHost, residualHost, gammaHost, biasHost,
                                     output, "R31B_V011");
    return raw.good() && jitter.good() && pass && correct;
}

bool RunPaired(const char* prefix, DeviceBuffers& buffers,
               const TensorGroupInfo& dataInfo,
               const TensorGroupInfo& paramInfo, aclrtStream stream,
               aclrtEvent startEvent, aclrtEvent stopEvent,
               const std::vector<uint16_t>& xHost,
               const std::vector<uint16_t>& residualHost,
               const std::vector<uint16_t>& gammaHost,
               const std::vector<uint16_t>& biasHost)
{
    if (!OutputPathsAvailable(prefix)) return false;
    const std::string rawPath = std::string(prefix) + "-raw.tsv";
    const std::string jitterPath = std::string(prefix) + "-jitter.txt";
    std::ofstream raw(rawPath);
    std::ofstream jitter(jitterPath);
    if (!raw || !jitter) {
        std::fprintf(stderr, "OUTPUT_OPEN_FAIL prefix=%s\n", prefix);
        return false;
    }
    for (int i = 0; i < kWarmups; ++i) {
        const bool parentFirst = (i % 2) == 0;
        KernelFn first = parentFirst ? sync_parent_run_kernel : sync_candidate_run_kernel;
        KernelFn second = parentFirst ? sync_candidate_run_kernel : sync_parent_run_kernel;
        void* firstOutput = parentFirst ? buffers.parentOutput : buffers.candidateOutput;
        void* secondOutput = parentFirst ? buffers.candidateOutput : buffers.parentOutput;
        Launch(first, firstOutput, buffers, dataInfo, paramInfo, stream);
        CheckAcl(aclrtSynchronizeStream(stream), "paired parent/candidate warmup sync");
        Launch(second, secondOutput, buffers, dataInfo, paramInfo, stream);
        CheckAcl(aclrtSynchronizeStream(stream), "paired candidate/parent warmup sync");
    }
    std::printf("WARMUP binary=R31B_V011+SYNC_V001 launches_each=%d synchronized=1\n", kWarmups);

    raw << "phase\tgroup\tpair\torder\tvariant\tdevice_us\twall_us\tpaired_delta_us\tpaired_delta_pct\n";
    jitter << "phase=INTERLEAVED_PAIRS device=4 shape=2x12288 dtype=FP16 warmup_each=45 groups=4 pairs_per_group=11\n";
    bool eventsRecorded = false;
    std::vector<double> allParentDevice, allCandidateDevice, allParentWall,
                        allCandidateWall, allDelta, allDeltaPct;
    size_t candidateFaster = 0;
    for (int group = 1; group <= kPairGroups; ++group) {
        std::vector<double> pDevice, cDevice, pWall, cWall, deltas, deltaPct;
        for (int pair = 0; pair < kPairsPerGroup; ++pair) {
            const bool parentFirst = ((group * kPairsPerGroup + pair) % 2) == 0;
            TimedSample p, c;
            if (parentFirst) {
                p = Measure(sync_parent_run_kernel, buffers.parentOutput, buffers,
                            dataInfo, paramInfo, stream, startEvent, stopEvent, eventsRecorded);
                c = Measure(sync_candidate_run_kernel, buffers.candidateOutput, buffers,
                            dataInfo, paramInfo, stream, startEvent, stopEvent, eventsRecorded);
            } else {
                c = Measure(sync_candidate_run_kernel, buffers.candidateOutput, buffers,
                            dataInfo, paramInfo, stream, startEvent, stopEvent, eventsRecorded);
                p = Measure(sync_parent_run_kernel, buffers.parentOutput, buffers,
                            dataInfo, paramInfo, stream, startEvent, stopEvent, eventsRecorded);
            }
            const double delta = c.deviceUs - p.deviceUs;
            const double percent = p.deviceUs != 0.0 ? 100.0 * delta / p.deviceUs : INFINITY;
            if (delta < 0.0) ++candidateFaster;
            pDevice.push_back(p.deviceUs);
            cDevice.push_back(c.deviceUs);
            pWall.push_back(p.wallUs);
            cWall.push_back(c.wallUs);
            deltas.push_back(delta);
            deltaPct.push_back(percent);
            allParentDevice.push_back(p.deviceUs);
            allCandidateDevice.push_back(c.deviceUs);
            allParentWall.push_back(p.wallUs);
            allCandidateWall.push_back(c.wallUs);
            allDelta.push_back(delta);
            allDeltaPct.push_back(percent);
            const char* order = parentFirst ? "P,C" : "C,P";
            auto writeSample = [&](const char* variant, const TimedSample& sample) {
                raw << "PAIRED\t" << group << '\t' << pair + 1 << '\t'
                    << order << '\t' << variant << '\t'
                    << std::fixed << std::setprecision(6) << sample.deviceUs << '\t'
                    << sample.wallUs << '\t' << delta << '\t' << percent << '\n';
            };
            if (parentFirst) {
                writeSample("PARENT", p);
                writeSample("CANDIDATE", c);
            } else {
                writeSample("CANDIDATE", c);
                writeSample("PARENT", p);
            }
        }
        const std::string suffix = "_group_" + std::to_string(group);
        WriteStats(jitter, ("parent_device" + suffix).c_str(), pDevice);
        WriteStats(jitter, ("candidate_device" + suffix).c_str(), cDevice);
        WriteStats(jitter, ("parent_wall" + suffix).c_str(), pWall);
        WriteStats(jitter, ("candidate_wall" + suffix).c_str(), cWall);
        WriteStats(jitter, ("paired_delta_us" + suffix).c_str(), deltas);
        WriteStats(jitter, ("paired_delta_pct" + suffix).c_str(), deltaPct);
    }
    WriteStats(jitter, "parent_device_all", allParentDevice);
    WriteStats(jitter, "candidate_device_all", allCandidateDevice);
    WriteStats(jitter, "parent_wall_all", allParentWall);
    WriteStats(jitter, "candidate_wall_all", allCandidateWall);
    WriteStats(jitter, "paired_delta_us_all", allDelta);
    WriteStats(jitter, "paired_delta_pct_all", allDeltaPct);
    jitter << "candidate_faster_pairs=" << candidateFaster
           << "\ttotal_pairs=" << allDelta.size() << '\n';

    std::vector<uint16_t> parentOutput(xHost.size()), candidateOutput(xHost.size());
    CheckAcl(aclrtMemcpy(parentOutput.data(), parentOutput.size() * sizeof(uint16_t),
                         buffers.parentOutput, parentOutput.size() * sizeof(uint16_t),
                         ACL_MEMCPY_DEVICE_TO_HOST), "copy paired parent output");
    CheckAcl(aclrtMemcpy(candidateOutput.data(), candidateOutput.size() * sizeof(uint16_t),
                         buffers.candidateOutput, candidateOutput.size() * sizeof(uint16_t),
                         ACL_MEMCPY_DEVICE_TO_HOST), "copy paired candidate output");
    const bool parentCorrect = CheckOutput(xHost, residualHost, gammaHost, biasHost,
                                           parentOutput, "R31B_V011");
    const bool candidateCorrect = CheckOutput(xHost, residualHost, gammaHost, biasHost,
                                              candidateOutput, "SYNC_V001");
    const Stats deltaStats = Summarize(allDeltaPct);
    std::printf("PAIRED_RESULT groups=%d pairs=%zu parent_median_us=%.6f candidate_median_us=%.6f delta_pct_median=%.6f candidate_faster=%zu correctness_parent=%s correctness_candidate=%s\n",
                kPairGroups, allDelta.size(), Summarize(allParentDevice).median,
                Summarize(allCandidateDevice).median, deltaStats.median,
                candidateFaster, parentCorrect ? "PASS" : "FAIL",
                candidateCorrect ? "PASS" : "FAIL");
    return raw.good() && jitter.good() && parentCorrect && candidateCorrect;
}

}  // namespace

int main(int argc, char** argv)
{
    enum class Mode { kPrecheck, kSameBinary, kPaired };
    Mode mode;
    const char* prefix = nullptr;
    const char* precheckBlock = nullptr;
    int precheckRep = 0;
    if (argc == 5 && std::strcmp(argv[1], "--precheck") == 0) {
        if ((std::strcmp(argv[2], "A") != 0 && std::strcmp(argv[2], "B") != 0) ||
            std::atoi(argv[3]) < 1 || std::atoi(argv[3]) > 3) {
            std::fprintf(stderr, "usage: %s --precheck A|B 1..3 output_prefix\n", argv[0]);
            return 2;
        }
        mode = Mode::kPrecheck;
        precheckBlock = argv[2];
        precheckRep = std::atoi(argv[3]);
        prefix = argv[4];
    } else if (argc == 3 && std::strcmp(argv[1], "--same-binary") == 0) {
        mode = Mode::kSameBinary;
        prefix = argv[2];
    } else if (argc == 3 && std::strcmp(argv[1], "--paired") == 0) {
        mode = Mode::kPaired;
        prefix = argv[2];
    } else {
        std::fprintf(stderr,
            "usage: %s --precheck A|B 1..3 output_prefix | --same-binary output_prefix | --paired output_prefix\n",
            argv[0]);
        return 2;
    }

    std::printf("IDENTITY route=SYNC-TOPOLOGY-CHAMPION-X revision=V001 device=4 shape=2x12288 dtype=FP16 parent_sha256=%s candidate_sha256=%s mode=%s\n",
                kParentSha, kCandidateSha,
                mode == Mode::kPrecheck ? "PRECHECK" :
                mode == Mode::kSameBinary ? "SAME_BINARY" : "PAIRED");
    CheckAcl(aclInit(nullptr), "aclInit");
    CheckAcl(aclrtSetDevice(kDevice), "aclrtSetDevice");
    aclrtStream stream = nullptr;
    aclrtEvent startEvent = nullptr;
    aclrtEvent stopEvent = nullptr;
    CheckAcl(aclrtCreateStream(&stream), "aclrtCreateStream");
    CheckAcl(aclrtCreateEvent(&startEvent), "create start event");
    CheckAcl(aclrtCreateEvent(&stopEvent), "create stop event");

    bool success = false;
    {
        DeviceBuffers buffers;
        std::vector<uint16_t> xHost, residualHost, gammaHost, biasHost;
        if (!CreateBuffers(buffers, xHost, residualHost, gammaHost, biasHost)) {
            std::fprintf(stderr, "ALLOC_FAIL\n");
            return 3;
        }
        int64_t dataShape[] = {kRows, kWidth};
        int64_t paramShape[] = {kWidth};
        TensorInfo dataTensor{dataShape, 2, kFp16};
        TensorInfo paramTensor{paramShape, 1, kFp16};
        TensorGroupInfo dataInfo{&dataTensor, 1};
        TensorGroupInfo paramInfo{&paramTensor, 1};
        if (mode == Mode::kPrecheck) {
            success = RunPrecheck(precheckBlock, precheckRep, prefix, buffers,
                dataInfo, paramInfo, stream, startEvent, stopEvent,
                xHost, residualHost, gammaHost, biasHost);
        } else if (mode == Mode::kSameBinary) {
            success = RunSameBinary(prefix, buffers, dataInfo, paramInfo,
                stream, startEvent, stopEvent,
                xHost, residualHost, gammaHost, biasHost);
        } else {
            success = RunPaired(prefix, buffers, dataInfo, paramInfo,
                stream, startEvent, stopEvent,
                xHost, residualHost, gammaHost, biasHost);
        }
    }
    CheckAcl(aclrtDestroyEvent(stopEvent), "destroy stop event");
    CheckAcl(aclrtDestroyEvent(startEvent), "destroy start event");
    CheckAcl(aclrtDestroyStream(stream), "destroy stream");
    CheckAcl(aclFinalize(), "aclFinalize");
    std::printf("RUNNER_COMPLETE mode=%s result=%s device_event_primary=1 wall_us_secondary=1 reset_device=0\n",
                mode == Mode::kPrecheck ? "PRECHECK" :
                mode == Mode::kSameBinary ? "SAME_BINARY" : "PAIRED",
                success ? "PASS" : "FAIL");
    return success ? 0 : 1;
}
