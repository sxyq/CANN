#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>
#include <vector>

namespace {

constexpr int32_t kDevice = 0;
constexpr int64_t kRows = 80;
constexpr int64_t kWidth = 2056;
constexpr int kWarmupPairs = 45;
constexpr int kMeasuredPairs = 21;
constexpr float kEpsilon = 1.0e-5f;

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

void CheckAcl(aclError status, const char* operation)
{
    if (status != ACL_SUCCESS) {
        std::fprintf(stderr, "ACL_FAIL operation=%s code=%d detail=%s\n", operation,
                     status, aclGetRecentErrMsg() ? aclGetRecentErrMsg() : "");
        std::exit(2);
    }
}

void Allocate(void** pointer, size_t bytes, const char* operation)
{
    CheckAcl(aclrtMalloc(pointer, bytes, ACL_MEM_MALLOC_HUGE_FIRST), operation);
}

void FreeBuffers(DeviceBuffers& buffers)
{
    CheckAcl(aclrtFree(buffers.candidateOutput), "free candidate output");
    CheckAcl(aclrtFree(buffers.parentOutput), "free parent output");
    CheckAcl(aclrtFree(buffers.bias), "free bias");
    CheckAcl(aclrtFree(buffers.gamma), "free gamma");
    CheckAcl(aclrtFree(buffers.residual), "free residual");
    CheckAcl(aclrtFree(buffers.x), "free x");
}

void LaunchParent(const DeviceBuffers& buffers, const TensorGroupInfo& dataGroup,
                  const TensorGroupInfo& parameterGroup, int64_t coreCount,
                  aclrtStream stream)
{
    r03_parent::run_kernel_parent(
        static_cast<uint8_t*>(buffers.x), dataGroup,
        static_cast<uint8_t*>(buffers.residual), dataGroup,
        static_cast<uint8_t*>(buffers.gamma), parameterGroup,
        static_cast<uint8_t*>(buffers.bias), parameterGroup,
        static_cast<uint8_t*>(buffers.parentOutput), dataGroup,
        coreCount, stream, kEpsilon);
}

void LaunchCandidate(const DeviceBuffers& buffers, const TensorGroupInfo& dataGroup,
                     const TensorGroupInfo& parameterGroup, int64_t coreCount,
                     aclrtStream stream)
{
    r03_candidate::run_kernel_candidate(
        static_cast<uint8_t*>(buffers.x), dataGroup,
        static_cast<uint8_t*>(buffers.residual), dataGroup,
        static_cast<uint8_t*>(buffers.gamma), parameterGroup,
        static_cast<uint8_t*>(buffers.bias), parameterGroup,
        static_cast<uint8_t*>(buffers.candidateOutput), dataGroup,
        coreCount, stream, kEpsilon);
}

TimedSample Measure(bool candidate, const DeviceBuffers& buffers,
                    const TensorGroupInfo& dataGroup,
                    const TensorGroupInfo& parameterGroup, int64_t coreCount,
                    aclrtStream stream, Events& events)
{
    if (events.recorded) {
        CheckAcl(aclrtResetEvent(events.start, stream), "reset start event");
        CheckAcl(aclrtResetEvent(events.stop, stream), "reset stop event");
    }
    const auto wallStart = std::chrono::steady_clock::now();
    CheckAcl(aclrtRecordEvent(events.start, stream), "record start event");
    if (candidate) {
        LaunchCandidate(buffers, dataGroup, parameterGroup, coreCount, stream);
    } else {
        LaunchParent(buffers, dataGroup, parameterGroup, coreCount, stream);
    }
    CheckAcl(aclrtRecordEvent(events.stop, stream), "record stop event");
    CheckAcl(aclrtSynchronizeEvent(events.stop), "sync stop event");
    const auto wallStop = std::chrono::steady_clock::now();
    events.recorded = true;
    float elapsedMs = 0.0f;
    CheckAcl(aclrtEventElapsedTime(&elapsedMs, events.start, events.stop),
             "read event duration");
    return {static_cast<double>(elapsedMs) * 1000.0,
            std::chrono::duration<double, std::micro>(wallStop - wallStart).count()};
}

double Median(std::vector<double> values)
{
    std::sort(values.begin(), values.end());
    const size_t middle = values.size() / 2;
    return values.size() % 2 ? values[middle]
                             : (values[middle - 1] + values[middle]) / 2.0;
}

void MakeInputs(std::vector<float>& x, std::vector<float>& residual,
                std::vector<float>& gamma, std::vector<float>& bias)
{
    for (size_t i = 0; i < x.size(); ++i) {
        x[i] = static_cast<float>(static_cast<int>(i % 97) - 48) * 0.01f;
        residual[i] = static_cast<float>(static_cast<int>(i % 71) - 35) * 0.008f;
    }
    for (size_t i = 0; i < gamma.size(); ++i) {
        gamma[i] = 0.75f + static_cast<float>(i % 13) * 0.001f;
        bias[i] = static_cast<float>(static_cast<int>(i % 17) - 8) * 0.002f;
    }
}

std::vector<float> Golden(const std::vector<float>& x,
                          const std::vector<float>& residual,
                          const std::vector<float>& gamma,
                          const std::vector<float>& bias)
{
    std::vector<float> output(x.size());
    for (int64_t row = 0; row < kRows; ++row) {
        const size_t offset = static_cast<size_t>(row * kWidth);
        double squareSum = 0.0;
        for (int64_t col = 0; col < kWidth; ++col) {
            const float y = x[offset + col] + residual[offset + col];
            squareSum += static_cast<double>(y) * static_cast<double>(y);
        }
        const float meanSquare = static_cast<float>(squareSum / kWidth) + kEpsilon;
        const float invRms = 1.0f / std::sqrt(meanSquare);
        for (int64_t col = 0; col < kWidth; ++col) {
            const size_t i = offset + static_cast<size_t>(col);
            const float y = x[i] + residual[i];
            const float normalized = y * invRms;
            const float scaled = normalized * gamma[static_cast<size_t>(col)];
            output[i] = scaled + bias[static_cast<size_t>(col)];
        }
    }
    return output;
}

bool CheckOutput(const char* label, const std::vector<float>& actual,
                 const std::vector<float>& golden)
{
    constexpr double kAtol = 1.0 / 65536.0;
    constexpr double kRtol = 1.0 / 1024.0;
    constexpr double kMaxAbsLimit = 1.0e-2;
    size_t matched = 0;
    double maxAbs = 0.0;
    for (size_t i = 0; i < actual.size(); ++i) {
        const double difference = std::fabs(static_cast<double>(actual[i]) - golden[i]);
        maxAbs = std::max(maxAbs, difference);
        if (difference <= kAtol + kRtol * std::fabs(golden[i])) ++matched;
    }
    const double ratio = static_cast<double>(matched) / actual.size();
    const bool pass = ratio >= 0.99 && maxAbs <= kMaxAbsLimit;
    std::printf("CORRECTNESS_CASE label=%s result=%s matched_ratio=%.8f max_abs=%.9g "
                "atol=%.9g rtol=%.9g max_abs_limit=%.9g elements=%zu\n",
                label, pass ? "PASS" : "FAIL", ratio, maxAbs, kAtol, kRtol,
                kMaxAbsLimit, actual.size());
    return pass;
}

}  // namespace

int main(int argc, char** argv)
{
    const bool paired = argc == 2 && std::string(argv[1]) == "--paired";
    if (argc > 2 || (argc == 2 && !paired && std::string(argv[1]) != "--correctness")) {
        std::fprintf(stderr, "usage: r03_v002_runner [--correctness|--paired]\n");
        return 1;
    }

    CheckAcl(aclInit(nullptr), "aclInit");
    CheckAcl(aclrtSetDevice(kDevice), "set device");
    int64_t coreCount = 0;
    CheckAcl(aclrtGetDeviceInfo(kDevice, ACL_DEV_ATTR_VECTOR_CORE_NUM, &coreCount),
             "get vector core count");
    if (coreCount <= 0) {
        std::fprintf(stderr, "CORE_COUNT_INVALID=%lld\n", static_cast<long long>(coreCount));
        return 2;
    }

    const int64_t dataShape[] = {kRows, kWidth};
    const int64_t parameterShape[] = {kWidth};
    const TensorInfo dataInfo{dataShape, 2, 0};
    const TensorInfo parameterInfo{parameterShape, 1, 0};
    const TensorGroupInfo dataGroup{&dataInfo, 1};
    const TensorGroupInfo parameterGroup{&parameterInfo, 1};
    const size_t elements = static_cast<size_t>(kRows * kWidth);
    const size_t dataBytes = elements * sizeof(float);
    const size_t parameterBytes = static_cast<size_t>(kWidth) * sizeof(float);

    std::vector<float> x(elements), residual(elements), gamma(kWidth), bias(kWidth);
    MakeInputs(x, residual, gamma, bias);
    const std::vector<float> golden = Golden(x, residual, gamma, bias);
    DeviceBuffers buffers;
    Allocate(&buffers.x, dataBytes, "allocate x");
    Allocate(&buffers.residual, dataBytes, "allocate residual");
    Allocate(&buffers.gamma, parameterBytes, "allocate gamma");
    Allocate(&buffers.bias, parameterBytes, "allocate bias");
    Allocate(&buffers.parentOutput, dataBytes, "allocate parent output");
    Allocate(&buffers.candidateOutput, dataBytes, "allocate candidate output");
    CheckAcl(aclrtMemcpy(buffers.x, dataBytes, x.data(), dataBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy x");
    CheckAcl(aclrtMemcpy(buffers.residual, dataBytes, residual.data(), dataBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy residual");
    CheckAcl(aclrtMemcpy(buffers.gamma, parameterBytes, gamma.data(), parameterBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma");
    CheckAcl(aclrtMemcpy(buffers.bias, parameterBytes, bias.data(), parameterBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy bias");

    aclrtStream stream = nullptr;
    Events events;
    CheckAcl(aclrtCreateStream(&stream), "create stream");
    CheckAcl(aclrtCreateEvent(&events.start), "create start event");
    CheckAcl(aclrtCreateEvent(&events.stop), "create stop event");
    std::printf("CASE=80x2056 DTYPE=FP32 DEVICE=%d VECTOR_CORES=%lld BLOCKS=%lld "
                "EPSILON=%.9g RUN_MODE=%s\n",
                kDevice, static_cast<long long>(coreCount),
                static_cast<long long>(std::min<int64_t>(kRows, coreCount)),
                kEpsilon, paired ? "PAIRED" : "CORRECTNESS");

    LaunchParent(buffers, dataGroup, parameterGroup, coreCount, stream);
    CheckAcl(aclrtSynchronizeStream(stream), "sync parent correctness");
    LaunchCandidate(buffers, dataGroup, parameterGroup, coreCount, stream);
    CheckAcl(aclrtSynchronizeStream(stream), "sync candidate correctness");
    std::vector<float> parentOutput(elements), candidateOutput(elements);
    CheckAcl(aclrtMemcpy(parentOutput.data(), dataBytes, buffers.parentOutput, dataBytes,
                         ACL_MEMCPY_DEVICE_TO_HOST), "copy parent output back");
    CheckAcl(aclrtMemcpy(candidateOutput.data(), dataBytes, buffers.candidateOutput,
                         dataBytes, ACL_MEMCPY_DEVICE_TO_HOST), "copy candidate output back");
    const bool parentPass = CheckOutput("PARENT", parentOutput, golden);
    const bool candidatePass = CheckOutput("CANDIDATE", candidateOutput, golden);
    size_t different = 0;
    double maxParentDifference = 0.0;
    for (size_t i = 0; i < elements; ++i) {
        if (parentOutput[i] != candidateOutput[i]) ++different;
        maxParentDifference = std::max(maxParentDifference,
            std::fabs(static_cast<double>(parentOutput[i]) - candidateOutput[i]));
    }
    std::printf("PARENT_CANDIDATE_COMPARE different_elements=%zu max_abs_difference=%.9g\n",
                different, maxParentDifference);
    if (!paired) {
        std::printf("CORRECTNESS_RESULT=%s\n", parentPass && candidatePass ? "PASS" : "FAIL");
    }

    if (paired) {
        for (int i = 0; i < kWarmupPairs; ++i) {
            if ((i % 2) == 0) {
                LaunchParent(buffers, dataGroup, parameterGroup, coreCount, stream);
                CheckAcl(aclrtSynchronizeStream(stream), "warmup parent sync");
                LaunchCandidate(buffers, dataGroup, parameterGroup, coreCount, stream);
                CheckAcl(aclrtSynchronizeStream(stream), "warmup candidate sync");
            } else {
                LaunchCandidate(buffers, dataGroup, parameterGroup, coreCount, stream);
                CheckAcl(aclrtSynchronizeStream(stream), "warmup candidate sync");
                LaunchParent(buffers, dataGroup, parameterGroup, coreCount, stream);
                CheckAcl(aclrtSynchronizeStream(stream), "warmup parent sync");
            }
        }
        std::vector<double> parentDevice, candidateDevice, deltas;
        parentDevice.reserve(kMeasuredPairs);
        candidateDevice.reserve(kMeasuredPairs);
        deltas.reserve(kMeasuredPairs);
        for (int pair = 0; pair < kMeasuredPairs; ++pair) {
            TimedSample parent{};
            TimedSample candidate{};
            const bool parentFirst = (pair % 2) == 0;
            if (parentFirst) {
                parent = Measure(false, buffers, dataGroup, parameterGroup, coreCount,
                                 stream, events);
                candidate = Measure(true, buffers, dataGroup, parameterGroup, coreCount,
                                    stream, events);
            } else {
                candidate = Measure(true, buffers, dataGroup, parameterGroup, coreCount,
                                    stream, events);
                parent = Measure(false, buffers, dataGroup, parameterGroup, coreCount,
                                 stream, events);
            }
            const double delta = candidate.deviceUs - parent.deviceUs;
            parentDevice.push_back(parent.deviceUs);
            candidateDevice.push_back(candidate.deviceUs);
            deltas.push_back(delta);
            std::printf("SAMPLE pair=%02d order=%s parent_event_us=%.3f "
                        "candidate_event_us=%.3f parent_wall_us=%.3f "
                        "candidate_wall_us=%.3f delta_event_us=%.3f\n",
                        pair + 1, parentFirst ? "P,C" : "C,P", parent.deviceUs,
                        candidate.deviceUs, parent.wallUs, candidate.wallUs, delta);
        }
        const double parentMedian = Median(parentDevice);
        const double candidateMedian = Median(candidateDevice);
        const double deltaMedian = Median(deltas);
        const double deltaPercent = parentMedian == 0.0 ? 0.0
            : (candidateMedian / parentMedian - 1.0) * 100.0;
        std::printf("LOCAL_RESULT parent_median_us=%.3f candidate_median_us=%.3f "
                    "paired_delta_median_us=%.3f local_delta_percent=%.6f "
                    "samples_per_side=%d\n",
                    parentMedian, candidateMedian, deltaMedian, deltaPercent,
                    kMeasuredPairs);
    }

    CheckAcl(aclrtDestroyEvent(events.stop), "destroy stop event");
    CheckAcl(aclrtDestroyEvent(events.start), "destroy start event");
    CheckAcl(aclrtDestroyStream(stream), "destroy stream");
    FreeBuffers(buffers);
    CheckAcl(aclFinalize(), "aclFinalize");
    return parentPass && candidatePass ? 0 : 3;
}
