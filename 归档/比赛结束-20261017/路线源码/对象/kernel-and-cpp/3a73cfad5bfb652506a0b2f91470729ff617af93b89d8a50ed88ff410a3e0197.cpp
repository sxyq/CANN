#include "judge_types.hpp"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

using GM_ADDR = void*;
using KernelFn = void (*)(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);

namespace {

constexpr int32_t kFp16 = 1;
constexpr int32_t kBf16 = 2;
constexpr int64_t kRows = 80;
constexpr int64_t kAvailableCores = 40;
constexpr float kEpsilon = 1.0e-5f;
constexpr int kRequiredWarmups = 45;

struct Kernel {
    const char* name;
    const char* sourceSha256;
    KernelFn call;
};

struct Stats {
    double median;
    double mean;
    double stdev;
    double cv;
    double minimum;
    double maximum;
    double maxOverMin;
    double mad;
    double madOverMedian;
    double p10;
    double p90;
};

bool Check(aclError status, const char* operation)
{
    if (status == ACL_SUCCESS) return true;
    std::fprintf(stderr, "%s failed: ACL status %d\n", operation, static_cast<int>(status));
    return false;
}

const char* DTypeName(int32_t dtype)
{
    return dtype == kFp16 ? "fp16" : "bf16";
}

int32_t ParseDType(const char* text)
{
    if (std::strcmp(text, "fp16") == 0) return kFp16;
    if (std::strcmp(text, "bf16") == 0) return kBf16;
    return 0;
}

Stats CalculateStats(const std::vector<double>& values)
{
    std::vector<double> sorted = values;
    std::sort(sorted.begin(), sorted.end());
    const auto quantile = [&](double fraction) {
        const double position = fraction * static_cast<double>(sorted.size() - 1);
        const size_t lower = static_cast<size_t>(position);
        const size_t upper = std::min(lower + 1, sorted.size() - 1);
        return sorted[lower] + (position - static_cast<double>(lower)) * (sorted[upper] - sorted[lower]);
    };
    double sum = 0.0;
    for (double value : values) sum += value;
    const double mean = sum / static_cast<double>(values.size());
    double variance = 0.0;
    for (double value : values) {
        const double delta = value - mean;
        variance += delta * delta;
    }
    const double median = quantile(0.5);
    std::vector<double> deviations;
    deviations.reserve(values.size());
    for (double value : values) deviations.push_back(std::abs(value - median));
    std::sort(deviations.begin(), deviations.end());
    const double mad = deviations[deviations.size() / 2];
    return {
        median, mean, std::sqrt(variance / static_cast<double>(values.size())),
        mean == 0.0 ? 0.0 : std::sqrt(variance / static_cast<double>(values.size())) / mean,
        sorted.front(), sorted.back(), sorted.front() == 0.0 ? 0.0 : sorted.back() / sorted.front(),
        mad, median == 0.0 ? 0.0 : mad / median, quantile(0.1), quantile(0.9)};
}

struct DeviceBuffers {
    void* x = nullptr;
    void* residual = nullptr;
    void* gamma = nullptr;
    void* bias = nullptr;
    void* output = nullptr;
};

void Release(DeviceBuffers& buffers)
{
    void* pointers[] = {buffers.x, buffers.residual, buffers.gamma, buffers.bias, buffers.output};
    for (void* pointer : pointers) {
        if (pointer != nullptr) aclrtFree(pointer);
    }
    buffers = {};
}

bool AllocateAndLoad(DeviceBuffers& buffers, int32_t dtype, int64_t width)
{
    const size_t elementSize = sizeof(uint16_t);
    const size_t inputBytes = static_cast<size_t>(kRows * width) * elementSize;
    const size_t vectorBytes = static_cast<size_t>(width) * elementSize;
    const size_t sizes[] = {inputBytes, inputBytes, vectorBytes, vectorBytes, inputBytes};
    void** pointers[] = {&buffers.x, &buffers.residual, &buffers.gamma, &buffers.bias, &buffers.output};
    for (size_t i = 0; i < 5; ++i) {
        if (!Check(aclrtMalloc(pointers[i], sizes[i], ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc")) {
            Release(buffers);
            return false;
        }
    }
    std::vector<uint8_t> input(inputBytes, 0);
    std::vector<uint8_t> vector(width * elementSize, 0);
    // Deterministic nonzero coefficients keep the timed call on the same path
    // as correctness while the allocation and copies stay outside samples.
    std::fill(vector.begin(), vector.end(), static_cast<uint8_t>(0x3f));
    if (!Check(aclrtMemcpy(buffers.x, inputBytes, input.data(), inputBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy x") ||
        !Check(aclrtMemcpy(buffers.residual, inputBytes, input.data(), inputBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy residual") ||
        !Check(aclrtMemcpy(buffers.gamma, vectorBytes, vector.data(), vectorBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma") ||
        !Check(aclrtMemcpy(buffers.bias, vectorBytes, vector.data(), vectorBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy bias") ||
        !Check(aclrtMemset(buffers.output, inputBytes, 0, inputBytes), "init output")) {
        Release(buffers);
        return false;
    }
    return true;
}

void MakeGroups(int32_t dtype, int64_t width, int64_t inputShape[2], int64_t vectorShape[1],
                TensorInfo& inputInfo, TensorInfo& vectorInfo,
                TensorGroupInfo& inputGroup, TensorGroupInfo& vectorGroup)
{
    inputShape[0] = kRows;
    inputShape[1] = width;
    vectorShape[0] = width;
    inputInfo = {inputShape, 2, dtype};
    vectorInfo = {vectorShape, 1, dtype};
    inputGroup = {&inputInfo, 1};
    vectorGroup = {&vectorInfo, 1};
}

bool Warmup(const Kernel& kernel, const DeviceBuffers& buffers,
            const TensorGroupInfo& inputGroup, const TensorGroupInfo& vectorGroup,
            aclrtStream stream, int count)
{
    for (int i = 0; i < count; ++i) {
        kernel.call(buffers.x, inputGroup, buffers.residual, inputGroup,
                    buffers.gamma, vectorGroup, buffers.bias, vectorGroup,
                    buffers.output, inputGroup, kAvailableCores, stream, kEpsilon);
        if (!Check(aclrtSynchronizeStream(stream), "warmup synchronize")) return false;
    }
    return true;
}

bool Measure(const Kernel& kernel, const DeviceBuffers& buffers,
             const TensorGroupInfo& inputGroup, const TensorGroupInfo& vectorGroup,
             aclrtStream stream, aclrtEvent start, aclrtEvent stop,
             bool& eventsRecorded, double& deviceUs, double& wallUs)
{
    if (eventsRecorded) {
        if (!Check(aclrtResetEvent(start, stream), "reset start event") ||
            !Check(aclrtResetEvent(stop, stream), "reset stop event")) return false;
    }
    const auto wallStart = std::chrono::steady_clock::now();
    if (!Check(aclrtRecordEvent(start, stream), "record start")) return false;
    kernel.call(buffers.x, inputGroup, buffers.residual, inputGroup,
                buffers.gamma, vectorGroup, buffers.bias, vectorGroup,
                buffers.output, inputGroup, kAvailableCores, stream, kEpsilon);
    if (!Check(aclrtRecordEvent(stop, stream), "record stop") ||
        !Check(aclrtSynchronizeEvent(stop), "synchronize stop")) return false;
    const auto wallStop = std::chrono::steady_clock::now();
    float elapsedMs = 0.0f;
    if (!Check(aclrtEventElapsedTime(&elapsedMs, start, stop), "event elapsed")) return false;
    eventsRecorded = true;
    deviceUs = static_cast<double>(elapsedMs) * 1000.0;
    wallUs = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(wallStop - wallStart).count()) / 1000.0;
    return true;
}

bool WriteSample(FILE* output, int block, int sample, const char* order,
                 const Kernel& kernel, int device, int32_t dtype, int64_t width,
                 double deviceUs, double wallUs)
{
    return std::fprintf(output, "%d\t%d\t%s\t%s\t%s\t%d\t%lld\t%s\t%.6f\t%.3f\n",
                        block, sample, order, kernel.name, kernel.sourceSha256, device,
                        static_cast<long long>(width), DTypeName(dtype), deviceUs, wallUs) >= 0;
}

void PrintSummary(const char* mode, const Kernel& kernel, int block,
                  const std::vector<double>& deviceSamples)
{
    const Stats stats = CalculateStats(deviceSamples);
    std::printf("SUMMARY mode=%s side=%s block=%d n=%zu device_event_us_median=%.6f mean=%.6f stdev=%.6f cv=%.6f min=%.6f max=%.6f max_over_min=%.6f mad=%.6f mad_over_median=%.6f p10=%.6f p90=%.6f\n",
                mode, kernel.name, block, deviceSamples.size(), stats.median, stats.mean,
                stats.stdev, stats.cv, stats.minimum, stats.maximum, stats.maxOverMin,
                stats.mad, stats.madOverMedian, stats.p10, stats.p90);
}

bool RunSame(const Kernel& parent, int device, int32_t dtype, int64_t width,
             int warmups, int samples, int blocks, const char* outputPath)
{
    FILE* output = std::fopen(outputPath, "wx");
    if (output == nullptr) {
        std::perror("open raw output");
        return false;
    }
    std::fprintf(output, "block\tsample\torder\tside\tsource_sha256\tdevice\twidth\tdtype\tdevice_event_us\twall_us\n");
    DeviceBuffers buffers;
    aclrtStream stream = nullptr;
    aclrtEvent start = nullptr;
    aclrtEvent stop = nullptr;
    int64_t inputShape[2] = {}, vectorShape[1] = {};
    TensorInfo inputInfo{}, vectorInfo{};
    TensorGroupInfo inputGroup{}, vectorGroup{};
    bool passed = AllocateAndLoad(buffers, dtype, width) &&
                  Check(aclrtCreateStream(&stream), "create stream") &&
                  Check(aclrtCreateEvent(&start), "create start event") &&
                  Check(aclrtCreateEvent(&stop), "create stop event");
    if (passed) MakeGroups(dtype, width, inputShape, vectorShape, inputInfo, vectorInfo, inputGroup, vectorGroup);
    if (passed) passed = Warmup(parent, buffers, inputGroup, vectorGroup, stream, warmups);
    std::printf("QUALIFICATION mode=same side=parent device=%d width=%lld dtype=%s warmups=%d samples=%d blocks=%d\n",
                device, static_cast<long long>(width), DTypeName(dtype), warmups, samples, blocks);
    bool eventsRecorded = false;
    std::vector<double> allDeviceSamples;
    std::vector<double> blockMedians;
    allDeviceSamples.reserve(static_cast<size_t>(samples) * blocks);
    blockMedians.reserve(blocks);
    for (int block = 0; passed && block < blocks; ++block) {
        std::vector<double> deviceSamples;
        deviceSamples.reserve(samples);
        for (int sample = 0; sample < samples; ++sample) {
            double deviceUs = 0.0, wallUs = 0.0;
            passed = Measure(parent, buffers, inputGroup, vectorGroup, stream, start, stop,
                             eventsRecorded, deviceUs, wallUs);
            if (passed && !WriteSample(output, block, sample, "P", parent, device, dtype, width, deviceUs, wallUs)) passed = false;
            if (passed) deviceSamples.push_back(deviceUs);
        }
        if (passed) {
            PrintSummary("same", parent, block, deviceSamples);
            allDeviceSamples.insert(allDeviceSamples.end(), deviceSamples.begin(), deviceSamples.end());
            blockMedians.push_back(CalculateStats(deviceSamples).median);
        }
    }
    if (passed) {
        const Stats full = CalculateStats(allDeviceSamples);
        const double madRatio = full.madOverMedian;
        const double blockDrift = full.median == 0.0 || blockMedians.size() < 2
                                      ? 0.0
                                      : std::abs(blockMedians[0] - blockMedians[1]) / full.median;
        const bool qualified = madRatio <= 0.10 && blockDrift <= 0.10;
        const bool blocked = madRatio > 0.25 || blockDrift > 0.25;
        std::printf("SAME_BINARY_QUAL width=%lld dtype=%s result=%s samples=%zu median_us=%.6f mad_over_median=%.6f block_drift=%.6f\n",
                    static_cast<long long>(width), DTypeName(dtype),
                    qualified ? "PASS" : (blocked ? "MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE" : "NEEDS_VALIDATION"),
                    allDeviceSamples.size(), full.median, madRatio, blockDrift);
        passed = qualified;
    }
    if (stream != nullptr) aclrtSynchronizeStream(stream);
    if (stop != nullptr) aclrtDestroyEvent(stop);
    if (start != nullptr) aclrtDestroyEvent(start);
    if (stream != nullptr) aclrtDestroyStream(stream);
    Release(buffers);
    std::fclose(output);
    return passed;
}

bool RunPaired(const Kernel& parent, const Kernel& candidate, int device, int32_t dtype,
               int64_t width, int warmups, int samples, int blocks, const char* outputPath)
{
    FILE* output = std::fopen(outputPath, "wx");
    if (output == nullptr) {
        std::perror("open raw output");
        return false;
    }
    std::fprintf(output, "block\tsample\torder\tside\tsource_sha256\tdevice\twidth\tdtype\tdevice_event_us\twall_us\n");
    DeviceBuffers buffers;
    aclrtStream stream = nullptr;
    aclrtEvent start = nullptr;
    aclrtEvent stop = nullptr;
    int64_t inputShape[2] = {}, vectorShape[1] = {};
    TensorInfo inputInfo{}, vectorInfo{};
    TensorGroupInfo inputGroup{}, vectorGroup{};
    bool passed = AllocateAndLoad(buffers, dtype, width) &&
                  Check(aclrtCreateStream(&stream), "create stream") &&
                  Check(aclrtCreateEvent(&start), "create start event") &&
                  Check(aclrtCreateEvent(&stop), "create stop event");
    if (passed) MakeGroups(dtype, width, inputShape, vectorShape, inputInfo, vectorInfo, inputGroup, vectorGroup);
    if (passed) passed = Warmup(parent, buffers, inputGroup, vectorGroup, stream, warmups);
    if (passed) passed = Warmup(candidate, buffers, inputGroup, vectorGroup, stream, warmups);
    std::printf("PAIRING mode=paired device=%d width=%lld dtype=%s warmups_per_side=%d samples=%d blocks=%d\n",
                device, static_cast<long long>(width), DTypeName(dtype), warmups, samples, blocks);
    bool eventsRecorded = false;
    for (int block = 0; passed && block < blocks; ++block) {
        const bool parentFirst = (block % 2) == 0;
        std::vector<double> parentSamples;
        std::vector<double> candidateSamples;
        parentSamples.reserve(samples);
        candidateSamples.reserve(samples);
        for (int sample = 0; passed && sample < samples; ++sample) {
            const char* order = parentFirst ? "P_then_C" : "C_then_P";
            const Kernel* first = parentFirst ? &parent : &candidate;
            const Kernel* second = parentFirst ? &candidate : &parent;
            for (const Kernel* kernel : {first, second}) {
                double deviceUs = 0.0, wallUs = 0.0;
                passed = Measure(*kernel, buffers, inputGroup, vectorGroup, stream, start, stop,
                                 eventsRecorded, deviceUs, wallUs);
                if (passed && !WriteSample(output, block, sample, order, *kernel, device, dtype, width, deviceUs, wallUs)) passed = false;
                if (passed) {
                    if (kernel == &parent) parentSamples.push_back(deviceUs);
                    else candidateSamples.push_back(deviceUs);
                }
            }
        }
        if (passed) {
            PrintSummary("paired", parent, block, parentSamples);
            PrintSummary("paired", candidate, block, candidateSamples);
            std::vector<double> deltas;
            deltas.reserve(samples);
            for (int sample = 0; sample < samples; ++sample) {
                deltas.push_back((candidateSamples[sample] - parentSamples[sample]) /
                                 parentSamples[sample] * 100.0);
            }
            const Stats deltaStats = CalculateStats(deltas);
            std::printf("PAIR_DELTA block=%d n=%zu median_percent=%.6f mean_percent=%.6f stdev_percent=%.6f p10_percent=%.6f p90_percent=%.6f positive=%zu negative=%zu zero=%zu\n",
                        block, deltas.size(), deltaStats.median, deltaStats.mean, deltaStats.stdev,
                        deltaStats.p10, deltaStats.p90,
                        static_cast<size_t>(std::count_if(deltas.begin(), deltas.end(), [](double v) { return v > 0.0; })),
                        static_cast<size_t>(std::count_if(deltas.begin(), deltas.end(), [](double v) { return v < 0.0; })),
                        static_cast<size_t>(std::count(deltas.begin(), deltas.end(), 0.0)));
        }
    }
    if (stream != nullptr) aclrtSynchronizeStream(stream);
    if (stop != nullptr) aclrtDestroyEvent(stop);
    if (start != nullptr) aclrtDestroyEvent(start);
    if (stream != nullptr) aclrtDestroyStream(stream);
    Release(buffers);
    std::fclose(output);
    return passed;
}

void Usage(const char* program)
{
    std::fprintf(stderr, "Usage: %s DEVICE WIDTH DTYPE same|paired WARMUPS SAMPLES BLOCKS RAW.tsv\n", program);
}

}  // namespace

extern "C" void run_kernel_parent(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);
extern "C" void run_kernel_candidate(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);

constexpr char kParentSourceSha256[] =
    "a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3";
constexpr char kCandidateSourceSha256[] =
    "4dc1973ef198cd67693610b6ffedad6883b59f52207b9804052975a13f4e032b";

int main(int argc, char** argv)
{
    if (argc != 9) {
        Usage(argv[0]);
        return 2;
    }
    const int device = std::atoi(argv[1]);
    const int64_t width = std::atoll(argv[2]);
    const int32_t dtype = ParseDType(argv[3]);
    const std::string mode(argv[4]);
    const int warmups = std::atoi(argv[5]);
    const int samples = std::atoi(argv[6]);
    const int blocks = std::atoi(argv[7]);
    const char* rawPath = argv[8];
    if (device < 0 || width <= 8192 || dtype == 0 || warmups < kRequiredWarmups || samples < 21 ||
        (mode != "same" && mode != "paired") || (mode == "same" && blocks < 2) ||
        (mode == "paired" && blocks < 4)) {
        Usage(argv[0]);
        return 2;
    }
    if (!Check(aclInit(nullptr), "aclInit") || !Check(aclrtSetDevice(device), "aclrtSetDevice")) return 1;
    const Kernel parent{"parent", kParentSourceSha256, run_kernel_parent};
    const Kernel candidate{"candidate", kCandidateSourceSha256, run_kernel_candidate};
    std::printf("PARENT_SOURCE_SHA256=%s\nCANDIDATE_SOURCE_SHA256=%s\n",
                parent.sourceSha256, candidate.sourceSha256);
    bool passed = false;
    if (mode == "same") passed = RunSame(parent, device, dtype, width, warmups, samples, blocks, rawPath);
    else passed = RunPaired(parent, candidate, device, dtype, width, warmups, samples, blocks, rawPath);
    if (!Check(aclFinalize(), "aclFinalize")) passed = false;
    std::printf("PERFORMANCE_RUN_RESULT=%s\n", passed ? "PASS" : "FAIL");
    return passed ? 0 : 1;
}
