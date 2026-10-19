#include "judge_types.hpp"

#include "acl/acl.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

extern "C" void adaptive_parent_run_kernel(
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, int64_t, aclrtStream, float);
extern "C" void adaptive_candidate_run_kernel(
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, int64_t, aclrtStream, float);

namespace {

constexpr float kEpsilon = 1.0e-5f;
constexpr int kWarmupCount = 45;
constexpr int kPairBlocks = 4;
constexpr int kPairsPerBlock = 11;
constexpr int kSameBinaryBlocks = 2;
constexpr int kSameBinarySamplesPerBlock = 41;

struct CaseSpec {
    const char* name;
    int rows;
    int width;
};

struct Buffers {
    explicit Buffers(const CaseSpec& spec)
        : x(static_cast<size_t>(spec.rows) * spec.width),
          residual(x.size()), gamma(spec.width), bias(spec.width),
          reference(x.size()), parentOutput(x.size()), candidateOutput(x.size())
    {
        dataShape[0] = spec.rows;
        dataShape[1] = spec.width;
        paramShape[0] = spec.width;
        dataTensor = {dataShape, 2, 0};
        paramTensor = {paramShape, 1, 0};
        outputTensor = {dataShape, 2, 0};
        dataInfo = {&dataTensor, 1};
        paramInfo = {&paramTensor, 1};
        outputInfo = {&outputTensor, 1};
    }

    ~Buffers()
    {
        if (dOutputCandidate) aclrtFree(dOutputCandidate);
        if (dOutputParent) aclrtFree(dOutputParent);
        if (dBias) aclrtFree(dBias);
        if (dGamma) aclrtFree(dGamma);
        if (dResidual) aclrtFree(dResidual);
        if (dX) aclrtFree(dX);
    }

    int64_t dataShape[2]{};
    int64_t paramShape[1]{};
    TensorInfo dataTensor{};
    TensorInfo paramTensor{};
    TensorInfo outputTensor{};
    TensorGroupInfo dataInfo{};
    TensorGroupInfo paramInfo{};
    TensorGroupInfo outputInfo{};
    std::vector<float> x;
    std::vector<float> residual;
    std::vector<float> gamma;
    std::vector<float> bias;
    std::vector<float> reference;
    std::vector<float> parentOutput;
    std::vector<float> candidateOutput;
    void* dX = nullptr;
    void* dResidual = nullptr;
    void* dGamma = nullptr;
    void* dBias = nullptr;
    void* dOutputParent = nullptr;
    void* dOutputCandidate = nullptr;
};

void CheckAcl(aclError status, const char* operation)
{
    if (status != ACL_SUCCESS) {
        throw std::runtime_error(std::string(operation) + " failed: " + std::to_string(status));
    }
}

void Allocate(void** pointer, size_t bytes, const char* operation)
{
    CheckAcl(aclrtMalloc(pointer, bytes, ACL_MEM_MALLOC_HUGE_FIRST), operation);
}

void InitializeInputs(Buffers& data, const CaseSpec& spec)
{
    std::mt19937 generator(314159u + static_cast<uint32_t>(spec.width));
    std::uniform_real_distribution<float> distribution(-0.5f, 0.5f);
    for (float& value : data.x) value = distribution(generator);
    for (float& value : data.residual) value = distribution(generator) * 0.25f;
    for (float& value : data.gamma) value = distribution(generator) + 1.0f;
    for (float& value : data.bias) value = distribution(generator) * 0.125f;

    for (int row = 0; row < spec.rows; ++row) {
        const size_t rowOffset = static_cast<size_t>(row) * spec.width;
        float squareSum = 0.0f;
        for (int col = 0; col < spec.width; ++col) {
            const size_t index = rowOffset + col;
            const float y = data.x[index] + data.residual[index];
            squareSum += y * y;
        }
        const float inverse = 1.0f / std::sqrt(squareSum / spec.width + kEpsilon);
        for (int col = 0; col < spec.width; ++col) {
            const size_t index = rowOffset + col;
            data.reference[index] =
                (data.x[index] + data.residual[index]) * inverse * data.gamma[col] + data.bias[col];
        }
    }
}

void CopyToDevice(Buffers& data)
{
    const size_t dataBytes = data.x.size() * sizeof(float);
    const size_t paramBytes = data.gamma.size() * sizeof(float);
    Allocate(&data.dX, dataBytes, "allocate x");
    Allocate(&data.dResidual, dataBytes, "allocate residual");
    Allocate(&data.dGamma, paramBytes, "allocate gamma");
    Allocate(&data.dBias, paramBytes, "allocate bias");
    Allocate(&data.dOutputParent, dataBytes, "allocate parent output");
    Allocate(&data.dOutputCandidate, dataBytes, "allocate candidate output");
    CheckAcl(aclrtMemcpy(data.dX, dataBytes, data.x.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy x");
    CheckAcl(aclrtMemcpy(data.dResidual, dataBytes, data.residual.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy residual");
    CheckAcl(aclrtMemcpy(data.dGamma, paramBytes, data.gamma.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma");
    CheckAcl(aclrtMemcpy(data.dBias, paramBytes, data.bias.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy bias");
}

using Kernel = void (*)(
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, int64_t, aclrtStream, float);

void Launch(Kernel kernel, void* output, const Buffers& data,
            int64_t availableCoreNum, aclrtStream stream)
{
    kernel(data.dX, data.dataInfo,
           data.dResidual, data.dataInfo,
           data.dGamma, data.paramInfo,
           data.dBias, data.paramInfo,
           output, data.outputInfo,
           availableCoreNum, stream, kEpsilon);
}

struct ErrorSummary {
    size_t failures = 0;
    float maxAbs = 0.0f;
};

ErrorSummary Compare(const std::vector<float>& actual, const std::vector<float>& expected)
{
    ErrorSummary result;
    for (size_t i = 0; i < actual.size(); ++i) {
        const float error = std::abs(actual[i] - expected[i]);
        result.maxAbs = std::max(result.maxAbs, error);
        const float tolerance = 2.0e-5f + 1.0e-4f * std::abs(expected[i]);
        if (!std::isfinite(actual[i]) || error > tolerance) ++result.failures;
    }
    return result;
}

void CopyFromDevice(std::vector<float>& output, const void* device, const char* operation)
{
    const size_t bytes = output.size() * sizeof(float);
    CheckAcl(aclrtMemcpy(output.data(), bytes, device, bytes, ACL_MEMCPY_DEVICE_TO_HOST), operation);
}

ErrorSummary RunCorrectness(const CaseSpec& spec, Buffers& data,
                           int64_t availableCoreNum, aclrtStream stream)
{
    Launch(adaptive_parent_run_kernel, data.dOutputParent, data, availableCoreNum, stream);
    Launch(adaptive_candidate_run_kernel, data.dOutputCandidate, data, availableCoreNum, stream);
    CheckAcl(aclrtSynchronizeStream(stream), "synchronize correctness");
    CopyFromDevice(data.parentOutput, data.dOutputParent, "copy parent output");
    CopyFromDevice(data.candidateOutput, data.dOutputCandidate, "copy candidate output");
    const ErrorSummary parent = Compare(data.parentOutput, data.reference);
    const ErrorSummary candidate = Compare(data.candidateOutput, data.reference);
    std::cout << "CORRECTNESS case=" << spec.name
              << " parent_failures=" << parent.failures
              << " parent_max_abs=" << parent.maxAbs
              << " candidate_failures=" << candidate.failures
              << " candidate_max_abs=" << candidate.maxAbs << '\n';
    if (parent.failures != 0 || candidate.failures != 0) {
        throw std::runtime_error("parent or candidate failed the FP32 reference comparison");
    }
    return candidate;
}

double Median(std::vector<double> values)
{
    if (values.empty()) return 0.0;
    std::sort(values.begin(), values.end());
    const size_t middle = values.size() / 2;
    if ((values.size() & 1u) != 0) return values[middle];
    return (values[middle - 1] + values[middle]) * 0.5;
}

double MadOverMedian(const std::vector<double>& values)
{
    if (values.empty()) return 0.0;
    const double median = Median(values);
    std::vector<double> deviations;
    deviations.reserve(values.size());
    for (double value : values) deviations.push_back(std::abs(value - median));
    return median > 0.0 ? Median(deviations) / median : 0.0;
}

struct Events {
    aclrtEvent start = nullptr;
    aclrtEvent stop = nullptr;
    bool recorded = false;
    ~Events()
    {
        if (stop) aclrtDestroyEvent(stop);
        if (start) aclrtDestroyEvent(start);
    }
};

double Measure(Kernel kernel, void* output, const Buffers& data,
               int64_t availableCoreNum, aclrtStream stream, Events& events)
{
    static uint64_t debugSample = 0;
    const bool debugFirstSample = debugSample++ == 0;
    if (events.recorded) {
        CheckAcl(aclrtResetEvent(events.start, stream), "reset start event");
        CheckAcl(aclrtResetEvent(events.stop, stream), "reset stop event");
    }
    if (debugFirstSample) { std::fprintf(stderr, "MEASURE_BEFORE_START_EVENT\n"); std::fflush(stderr); }
    CheckAcl(aclrtRecordEvent(events.start, stream), "record start event");
    if (debugFirstSample) { std::fprintf(stderr, "MEASURE_AFTER_START_EVENT\n"); std::fflush(stderr); }
    Launch(kernel, output, data, availableCoreNum, stream);
    if (debugFirstSample) { std::fprintf(stderr, "MEASURE_AFTER_LAUNCH\n"); std::fflush(stderr); }
    CheckAcl(aclrtRecordEvent(events.stop, stream), "record stop event");
    if (debugFirstSample) { std::fprintf(stderr, "MEASURE_AFTER_STOP_EVENT\n"); std::fflush(stderr); }
    CheckAcl(aclrtSynchronizeEvent(events.stop), "synchronize stop event");
    if (debugFirstSample) { std::fprintf(stderr, "MEASURE_AFTER_SYNC\n"); std::fflush(stderr); }
    float elapsedMs = 0.0f;
    CheckAcl(aclrtEventElapsedTime(&elapsedMs, events.start, events.stop), "read elapsed time");
    if (debugFirstSample) { std::fprintf(stderr, "MEASURE_AFTER_ELAPSED\n"); std::fflush(stderr); }
    if (debugFirstSample) { std::fprintf(stderr, "MEASURE_BEFORE_RECORDED_FLAG\n"); std::fflush(stderr); }
    events.recorded = true;
    if (debugFirstSample) { std::fprintf(stderr, "MEASURE_AFTER_RECORDED_FLAG\n"); std::fflush(stderr); }
    const double resultUs = static_cast<double>(elapsedMs) * 1000.0;
    if (debugFirstSample) { std::fprintf(stderr, "MEASURE_BEFORE_RETURN\n"); std::fflush(stderr); }
    return resultUs;
}

void RunLocal(const CaseSpec& spec, const Buffers& data,
              int64_t availableCoreNum, aclrtStream stream, std::ofstream& raw)
{
    for (int i = 0; i < kWarmupCount; ++i) {
        Launch(adaptive_parent_run_kernel, data.dOutputParent, data, availableCoreNum, stream);
        Launch(adaptive_candidate_run_kernel, data.dOutputCandidate, data, availableCoreNum, stream);
    }
    CheckAcl(aclrtSynchronizeStream(stream), "synchronize warmup");

    Events events;
    CheckAcl(aclrtCreateEvent(&events.start), "create start event");
    CheckAcl(aclrtCreateEvent(&events.stop), "create stop event");
    std::vector<double> parentSamples;
    std::vector<double> candidateSamples;
    std::vector<double> deltas;
    parentSamples.reserve(kPairBlocks * kPairsPerBlock);
    candidateSamples.reserve(kPairBlocks * kPairsPerBlock);
    deltas.reserve(kPairBlocks * kPairsPerBlock);

    for (int block = 0; block < kPairBlocks; ++block) {
        for (int pair = 0; pair < kPairsPerBlock; ++pair) {
            const bool parentFirst = ((block + pair) & 1) == 0;
            double parentUs = 0.0;
            double candidateUs = 0.0;
            if (parentFirst) {
                parentUs = Measure(adaptive_parent_run_kernel, data.dOutputParent,
                                   data, availableCoreNum, stream, events);
                candidateUs = Measure(adaptive_candidate_run_kernel, data.dOutputCandidate,
                                      data, availableCoreNum, stream, events);
            } else {
                candidateUs = Measure(adaptive_candidate_run_kernel, data.dOutputCandidate,
                                      data, availableCoreNum, stream, events);
                parentUs = Measure(adaptive_parent_run_kernel, data.dOutputParent,
                                   data, availableCoreNum, stream, events);
            }
            const double deltaPct = parentUs > 0.0
                ? (candidateUs - parentUs) * 100.0 / parentUs : 0.0;
            parentSamples.push_back(parentUs);
            candidateSamples.push_back(candidateUs);
            deltas.push_back(deltaPct);
            raw << spec.name << '\t' << block << '\t' << pair << '\t'
                << (parentFirst ? "P-C" : "C-P") << '\t'
                << parentUs << '\t' << candidateUs << '\t' << deltaPct << '\n';
        }
    }

    std::cout << "LOCAL case=" << spec.name
              << " samples_per_variant=" << parentSamples.size()
              << " parent_median_us=" << Median(parentSamples)
              << " candidate_median_us=" << Median(candidateSamples)
              << " delta_median_pct=" << Median(deltas)
              << " parent_mad_over_median=" << MadOverMedian(parentSamples)
              << " candidate_mad_over_median=" << MadOverMedian(candidateSamples) << '\n';
}

void RunSameBinary(const CaseSpec& spec, const Buffers& data,
                   int64_t availableCoreNum, aclrtStream stream,
                   std::ofstream& raw)
{
    std::cerr << "SAME_BINARY_BEGIN case=" << spec.name << std::endl;
    for (int i = 0; i < kWarmupCount; ++i) {
        Launch(adaptive_parent_run_kernel, data.dOutputParent, data, availableCoreNum, stream);
    }
    CheckAcl(aclrtSynchronizeStream(stream), "synchronize same-binary warmup");
    std::cerr << "SAME_BINARY_WARMUP_DONE case=" << spec.name << std::endl;

    Events events;
    CheckAcl(aclrtCreateEvent(&events.start), "create same-binary start event");
    CheckAcl(aclrtCreateEvent(&events.stop), "create same-binary stop event");
    std::vector<double> allSamples;
    allSamples.reserve(kSameBinaryBlocks * kSameBinarySamplesPerBlock);
    std::vector<double> blockMedians;
    blockMedians.reserve(kSameBinaryBlocks);

    for (int block = 0; block < kSameBinaryBlocks; ++block) {
        if (block != 0) std::this_thread::sleep_for(std::chrono::seconds(2));
        std::cerr << "SAME_BINARY_BLOCK_BEGIN case=" << spec.name
                  << " block=" << block << std::endl;
        std::vector<double> samples;
        samples.reserve(kSameBinarySamplesPerBlock);
        for (int sample = 0; sample < kSameBinarySamplesPerBlock; ++sample) {
            const double us = Measure(adaptive_parent_run_kernel, data.dOutputParent,
                                      data, availableCoreNum, stream, events);
            if (block == 0 && sample == 0) { std::fprintf(stderr, "SAME_BINARY_MEASURE_RETURN\n"); std::fflush(stderr); }
            samples.push_back(us);
            if (block == 0 && sample == 0) { std::fprintf(stderr, "SAME_BINARY_SAMPLE_VECTOR_PUSHED\n"); std::fflush(stderr); }
            allSamples.push_back(us);
            raw << spec.name << '\t' << block << '\t' << sample << '\t' << us << '\n';
            raw.flush();
            if (block == 0 && sample == 0) { std::fprintf(stderr, "SAME_BINARY_RAW_FLUSHED\n"); std::fflush(stderr); }
        }
        blockMedians.push_back(Median(samples));
        std::cerr << "SAME_BINARY_BLOCK_DONE case=" << spec.name
                  << " block=" << block << std::endl;
    }

    const double center = Median(allSamples);
    std::vector<double> deviations;
    deviations.reserve(allSamples.size());
    for (double value : allSamples) deviations.push_back(std::abs(value - center));
    const double madOverMedian = center > 0.0 ? Median(deviations) / center : 0.0;
    const double blockDrift = center > 0.0
        ? std::abs(blockMedians[0] - blockMedians[1]) / center : 0.0;
    const bool pass = madOverMedian <= 0.10 && blockDrift <= 0.10;
    std::cout << "SAME_BINARY case=" << spec.name
              << " samples=" << allSamples.size()
              << " median_us=" << center
              << " mad_over_median=" << madOverMedian
              << " block1_median_us=" << blockMedians[0]
              << " block2_median_us=" << blockMedians[1]
              << " block_drift=" << blockDrift
              << " verdict=" << (pass ? "PASS" : "MEASUREMENT_BLOCKED") << '\n';
}

void RunCase(const CaseSpec& spec, const std::string& mode,
             int64_t availableCoreNum, aclrtStream stream, std::ofstream* raw)
{
    Buffers data(spec);
    InitializeInputs(data, spec);
    CopyToDevice(data);
    RunCorrectness(spec, data, availableCoreNum, stream);
    if (mode == "local") RunLocal(spec, data, availableCoreNum, stream, *raw);
    if (mode == "same") RunSameBinary(spec, data, availableCoreNum, stream, *raw);
}

}  // namespace

int main(int argc, char** argv)
{
    std::cout.setf(std::ios::unitbuf);
    std::string mode;
    std::string outputPath;
    int deviceId = -1;
    for (int i = 1; i < argc; ++i) {
        const std::string option = argv[i];
        if (option == "--mode" && i + 1 < argc) mode = argv[++i];
        else if (option == "--device" && i + 1 < argc) deviceId = std::stoi(argv[++i]);
        else if (option == "--output" && i + 1 < argc) outputPath = argv[++i];
        else {
            std::cerr << "unknown or incomplete option: " << option << '\n';
            return 2;
        }
    }
    if ((mode != "correctness" && mode != "local" && mode != "same") || deviceId < 0 ||
        ((mode == "local" || mode == "same") && outputPath.empty())) {
        std::cerr << "usage: adaptive_probe --mode <correctness|same|local> --device <id> [--output <new-path>]\n";
        return 2;
    }

    std::ofstream raw;
    if (mode == "local" || mode == "same") {
        if (std::filesystem::exists(outputPath)) {
            std::cerr << "output path already exists; refusing to replace evidence\n";
            return 2;
        }
        raw.open(outputPath);
        if (!raw) {
            std::cerr << "cannot create raw sample file\n";
            return 2;
        }
        if (mode == "same") raw << "case\tblock\tsample\tparent_device_us\n";
        else raw << "case\tblock\tpair\torder\tparent_device_us\tcandidate_device_us\tdelta_pct\n";
        raw.flush();
    }

    aclrtStream stream = nullptr;
    bool initialized = false;
    try {
        CheckAcl(aclInit(nullptr), "aclInit");
        initialized = true;
        CheckAcl(aclrtSetDevice(deviceId), "set device");
        int64_t availableCoreNum = 0;
        CheckAcl(aclrtGetDeviceInfo(deviceId, ACL_DEV_ATTR_VECTOR_CORE_NUM, &availableCoreNum),
                 "get vector core count");
        if (availableCoreNum <= 0) throw std::runtime_error("device reported no vector cores");
        CheckAcl(aclrtCreateStream(&stream), "create stream");
        std::cout << "DEVICE id=" << deviceId
                  << " available_vector_cores=" << availableCoreNum
                  << " mode=" << mode
                  << " shape_scope=local_proxy_not_official_mapping\n";

        const CaseSpec cases[] = {
            {"target-fp32-64x8192", 64, 8192},
            {"control-fp32-12x8192", 12, 8192},
        };
        for (const CaseSpec& spec : cases) {
            RunCase(spec, mode, availableCoreNum, stream,
                    (mode == "local" || mode == "same") ? &raw : nullptr);
        }
        if (mode == "local") raw.flush();
        CheckAcl(aclrtDestroyStream(stream), "destroy stream");
        stream = nullptr;
        CheckAcl(aclrtResetDevice(deviceId), "reset device");
        CheckAcl(aclFinalize(), "aclFinalize");
        initialized = false;
        return 0;
    } catch (const std::exception& error) {
        std::cerr << "RUN_FAILED " << error.what() << '\n';
        if (stream) aclrtDestroyStream(stream);
        if (initialized) {
            aclrtResetDevice(deviceId);
            aclFinalize();
        }
        return 1;
    }
}
