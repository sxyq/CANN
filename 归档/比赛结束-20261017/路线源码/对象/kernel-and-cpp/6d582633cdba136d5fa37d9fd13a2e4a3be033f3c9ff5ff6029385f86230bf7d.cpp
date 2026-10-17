#include "judge_types.hpp"

#include "acl/acl.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstring>
#include <dlfcn.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

extern "C" void r04_parent_run_kernel(
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, int64_t, aclrtStream, float);
extern "C" void r04_candidate_run_kernel(
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, int64_t, aclrtStream, float);

namespace {

constexpr float kEpsilon = 1.0e-5f;
constexpr int kWarmupCount = 45;
constexpr int kBlocks = 4;
constexpr int kSameCallsPerBlock = 32;
constexpr int kPairedCyclesPerBlock = 4;
constexpr char kPairedCycle[] = "PPPCCPCC";

struct CaseSpec {
    const char* name;
    int rows;
    int width;
};

struct Buffers {
    explicit Buffers(const CaseSpec& spec)
        : x(static_cast<size_t>(spec.rows) * spec.width),
          residual(x.size()), gamma(spec.width), bias(spec.width),
          reference(x.size()), output(x.size()), parentOutput(x.size())
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
        if (dOutput) aclrtFree(dOutput);
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
    std::vector<float> output;
    std::vector<float> parentOutput;
    void* dX = nullptr;
    void* dResidual = nullptr;
    void* dGamma = nullptr;
    void* dBias = nullptr;
    void* dOutput = nullptr;
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

void Initialize(Buffers& data, const CaseSpec& spec)
{
    std::mt19937 generator(314159u + static_cast<uint32_t>(spec.width));
    std::uniform_real_distribution<float> distribution(-0.5f, 0.5f);
    for (float& value : data.x) value = distribution(generator);
    for (float& value : data.residual) value = distribution(generator) * 0.25f;
    for (float& value : data.gamma) value = distribution(generator) + 1.0f;
    for (float& value : data.bias) value = distribution(generator) * 0.125f;

    for (int row = 0; row < spec.rows; ++row) {
        const size_t offset = static_cast<size_t>(row) * spec.width;
        double squareSum = 0.0;
        for (int col = 0; col < spec.width; ++col) {
            const size_t i = offset + col;
            const double y = static_cast<double>(data.x[i]) + data.residual[i];
            squareSum += y * y;
        }
        const double inverse = 1.0 / std::sqrt(squareSum / spec.width + static_cast<double>(kEpsilon));
        for (int col = 0; col < spec.width; ++col) {
            const size_t i = offset + col;
            data.reference[i] = static_cast<float>(
                (static_cast<double>(data.x[i]) + data.residual[i]) * inverse * data.gamma[col] + data.bias[col]);
        }
    }

    const size_t dataBytes = data.x.size() * sizeof(float);
    const size_t paramBytes = data.gamma.size() * sizeof(float);
    Allocate(&data.dX, dataBytes, "allocate x");
    Allocate(&data.dResidual, dataBytes, "allocate residual");
    Allocate(&data.dGamma, paramBytes, "allocate gamma");
    Allocate(&data.dBias, paramBytes, "allocate bias");
    Allocate(&data.dOutput, dataBytes, "allocate shared output");
    CheckAcl(aclrtMemcpy(data.dX, dataBytes, data.x.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy x");
    CheckAcl(aclrtMemcpy(data.dResidual, dataBytes, data.residual.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy residual");
    CheckAcl(aclrtMemcpy(data.dGamma, paramBytes, data.gamma.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma");
    CheckAcl(aclrtMemcpy(data.dBias, paramBytes, data.bias.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy bias");
}

using Kernel = void (*)(
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, int64_t, aclrtStream, float);

void Launch(Kernel kernel, const Buffers& data, int64_t coreCount, aclrtStream stream)
{
    kernel(data.dX, data.dataInfo,
           data.dResidual, data.dataInfo,
           data.dGamma, data.paramInfo,
           data.dBias, data.paramInfo,
           data.dOutput, data.outputInfo,
           coreCount, stream, kEpsilon);
}

void CopyOutput(Buffers& data)
{
    const size_t bytes = data.output.size() * sizeof(float);
    CheckAcl(aclrtMemcpy(data.output.data(), bytes, data.dOutput, bytes, ACL_MEMCPY_DEVICE_TO_HOST), "copy output");
}

struct ErrorSummary {
    size_t failures = 0;
    float maxAbs = 0.0f;
};

ErrorSummary CompareReference(const Buffers& data)
{
    ErrorSummary result;
    for (size_t i = 0; i < data.output.size(); ++i) {
        const float error = std::abs(data.output[i] - data.reference[i]);
        result.maxAbs = std::max(result.maxAbs, error);
        const float tolerance = 1.0e-5f + 1.0e-4f * std::abs(data.reference[i]);
        if (!std::isfinite(data.output[i]) || error > tolerance || error > 1.0e-2f) ++result.failures;
    }
    return result;
}

void RunReferenceCheck(const CaseSpec& spec, Buffers& data, int64_t coreCount,
                       aclrtStream stream, const char* phase, std::ofstream& references)
{
    Launch(r04_parent_run_kernel, data, coreCount, stream);
    CheckAcl(aclrtSynchronizeStream(stream), "sync parent reference check");
    CopyOutput(data);
    data.parentOutput = data.output;
    const ErrorSummary parent = CompareReference(data);

    Launch(r04_candidate_run_kernel, data, coreCount, stream);
    CheckAcl(aclrtSynchronizeStream(stream), "sync candidate reference check");
    CopyOutput(data);
    const ErrorSummary candidate = CompareReference(data);
    size_t mismatches = 0;
    for (size_t i = 0; i < data.output.size(); ++i) {
        if (std::memcmp(&data.output[i], &data.parentOutput[i], sizeof(float)) != 0) ++mismatches;
    }

    references << phase << '\t' << spec.name << '\t' << parent.failures << '\t'
               << parent.maxAbs << '\t' << candidate.failures << '\t'
               << candidate.maxAbs << '\t' << mismatches << '\n';
    references.flush();
    std::cout << "REFERENCE phase=" << phase << " case=" << spec.name
              << " parent_failures=" << parent.failures << " parent_max_abs=" << parent.maxAbs
              << " candidate_failures=" << candidate.failures << " candidate_max_abs=" << candidate.maxAbs
              << " parent_candidate_bitwise_mismatches=" << mismatches
              << " golden=CPU_FP64 atol=1e-5 rtol=1e-4 max_abs_limit=1e-2\n";
    if (parent.failures || candidate.failures || mismatches) {
        throw std::runtime_error("Parent or Candidate failed the independent CPU FP64 reference");
    }
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

double Measure(Kernel kernel, const Buffers& data, int64_t coreCount,
               aclrtStream stream, Events& events, double& wallUs)
{
    const auto wallStart = std::chrono::steady_clock::now();
    if (events.recorded) {
        CheckAcl(aclrtResetEvent(events.start, stream), "reset start event");
        CheckAcl(aclrtResetEvent(events.stop, stream), "reset stop event");
    }
    CheckAcl(aclrtRecordEvent(events.start, stream), "record start event");
    Launch(kernel, data, coreCount, stream);
    CheckAcl(aclrtRecordEvent(events.stop, stream), "record stop event");
    CheckAcl(aclrtSynchronizeEvent(events.stop), "sync stop event");
    float elapsedMs = 0.0f;
    CheckAcl(aclrtEventElapsedTime(&elapsedMs, events.start, events.stop), "read event elapsed time");
    wallUs = std::chrono::duration<double, std::micro>(
        std::chrono::steady_clock::now() - wallStart).count();
    events.recorded = true;
    return static_cast<double>(elapsedMs) * 1000.0;
}

void WriteSample(std::ofstream& raw, uint64_t& sequence, const CaseSpec& spec,
                 int block, const char* mode, int cycle, int position,
                 char previous, char variant, Kernel kernel, const Buffers& data,
                 int64_t coreCount, aclrtStream stream, Events& events)
{
    double wallUs = 0.0;
    const double eventUs = Measure(kernel, data, coreCount, stream, events, wallUs);
    ++sequence;
    raw << sequence << '\t' << spec.name << '\t' << block << '\t' << mode << '\t'
        << cycle << '\t' << position << '\t' << previous << '\t' << variant << '\t'
        << std::setprecision(12) << eventUs << '\t' << wallUs << '\n';
}

void RunMeasurements(const CaseSpec& spec, const Buffers& data, int64_t coreCount,
                     aclrtStream stream, std::ofstream& raw, uint64_t& sequence)
{
    for (int i = 0; i < kWarmupCount; ++i) Launch(r04_parent_run_kernel, data, coreCount, stream);
    for (int i = 0; i < kWarmupCount; ++i) Launch(r04_candidate_run_kernel, data, coreCount, stream);
    CheckAcl(aclrtSynchronizeStream(stream), "sync both-library warmup");

    Events events;
    CheckAcl(aclrtCreateEvent(&events.start), "create start event");
    CheckAcl(aclrtCreateEvent(&events.stop), "create stop event");
    int samePhaseOrdinal = 0;
    int pairedPhaseOrdinal = 0;
    for (int block = 0; block < kBlocks; ++block) {
        auto runSame = [&]() {
            Launch(r04_parent_run_kernel, data, coreCount, stream);
            CheckAcl(aclrtSynchronizeStream(stream), "set P/P predecessor");
            char previous = 'P';
            for (int i = 0; i < kSameCallsPerBlock; ++i) {
                const int position = i % 2 + 1;
                WriteSample(raw, sequence, spec, block + 1, "PP", samePhaseOrdinal / 8 + 1,
                            position, previous, 'P', r04_parent_run_kernel,
                            data, coreCount, stream, events);
                previous = 'P';
                ++samePhaseOrdinal;
            }
            raw.flush();
        };

        auto runPaired = [&]() {
            Launch(r04_candidate_run_kernel, data, coreCount, stream);
            CheckAcl(aclrtSynchronizeStream(stream), "set P/C predecessor");
            char previous = 'C';
            for (int cycle = 0; cycle < kPairedCyclesPerBlock; ++cycle) {
                for (int i = 0; i < 8; ++i) {
                    const char variant = kPairedCycle[i];
                    const Kernel kernel = variant == 'P'
                        ? r04_parent_run_kernel : r04_candidate_run_kernel;
                    const int position = i % 2 + 1;
                    WriteSample(raw, sequence, spec, block + 1, "PC", pairedPhaseOrdinal / 8 + 1,
                                position, previous, variant, kernel,
                                data, coreCount, stream, events);
                    previous = variant;
                    ++pairedPhaseOrdinal;
                }
            }
            raw.flush();
        };

        if ((block & 1) == 0) {
            runSame();
            runPaired();
        } else {
            runPaired();
            runSame();
        }
    }
}

const char* LibraryPath(Kernel kernel)
{
    Dl_info info{};
    if (!dladdr(reinterpret_cast<void*>(kernel), &info) || !info.dli_fname) return "UNKNOWN";
    return info.dli_fname;
}

}  // namespace

int main(int argc, char** argv)
{
    int deviceId = -1;
    std::string rawPath;
    for (int i = 1; i < argc; ++i) {
        const std::string option = argv[i];
        if (option == "--device" && i + 1 < argc) deviceId = std::stoi(argv[++i]);
        else if (option == "--output" && i + 1 < argc) rawPath = argv[++i];
        else {
            std::cerr << "unknown or incomplete option: " << option << '\n';
            return 2;
        }
    }
    if (deviceId < 0 || rawPath.empty()) {
        std::cerr << "usage: r04_timing_host --device <id> --output <new-tsv-path>\n";
        return 2;
    }
    if (std::ifstream(rawPath).good()) {
        std::cerr << "raw output path already exists; refusing to replace evidence\n";
        return 2;
    }
    std::ofstream raw(rawPath);
    std::ofstream references(rawPath + ".reference.tsv");
    if (!raw || !references) {
        std::cerr << "cannot create raw or reference output\n";
        return 2;
    }
    raw << std::setprecision(12);
    raw << "sequence\tcase\tblock\tmode\tcycle\tposition\tprevious_variant\tvariant\tdevice_event_us\twall_us\n";
    references << "phase\tcase\tparent_failures\tparent_max_abs\tcandidate_failures\tcandidate_max_abs\tparent_candidate_bitwise_mismatches\n";

    aclrtStream stream = nullptr;
    bool initialized = false;
    try {
        CheckAcl(aclInit(nullptr), "aclInit");
        initialized = true;
        CheckAcl(aclrtSetDevice(deviceId), "set device");
        int64_t coreCount = 0;
        CheckAcl(aclrtGetDeviceInfo(deviceId, ACL_DEV_ATTR_VECTOR_CORE_NUM, &coreCount), "get vector core count");
        if (coreCount <= 0) throw std::runtime_error("device reported no vector cores");
        CheckAcl(aclrtCreateStream(&stream), "create stream");

        std::cout << std::setprecision(12)
                  << "RUN_META device=" << deviceId << " available_vector_cores=" << coreCount
                  << " dtype=FP32 epsilon=" << kEpsilon << " warmup_parent=" << kWarmupCount
                  << " warmup_candidate=" << kWarmupCount << " output_address_shared_per_case=YES\n"
                  << "LIBRARY variant=P path=" << LibraryPath(r04_parent_run_kernel) << '\n'
                  << "LIBRARY variant=C path=" << LibraryPath(r04_candidate_run_kernel) << '\n'
                  << "PROTOCOL cases=3 pp_samples_per_case=128 pc_samples_per_case=128 total_timed=768"
                  << " blocks=4 pc_cycle=PPPCCPCC pc_cycle_repetitions=16"
                  << " output_address_and_inputs_fixed_within_case=YES\n";

        const CaseSpec cases[] = {
            {"target-fp32-16x2048", 16, 2048},
            {"target-fp32-16x2056", 16, 2056},
            {"guard-off-fp32-12x8192", 12, 8192},
        };
        uint64_t sequence = 0;
        for (const CaseSpec& spec : cases) {
            Buffers data(spec);
            Initialize(data, spec);
            std::cout << "CASE_META case=" << spec.name
                      << " rows=" << spec.rows << " width=" << spec.width
                      << " input_x=" << data.dX << " input_residual=" << data.dResidual
                      << " gamma=" << data.dGamma << " bias=" << data.dBias
                      << " shared_output=" << data.dOutput << '\n';
            RunReferenceCheck(spec, data, coreCount, stream, "before", references);
            RunMeasurements(spec, data, coreCount, stream, raw, sequence);
            RunReferenceCheck(spec, data, coreCount, stream, "after", references);
        }
        raw.flush();
        CheckAcl(aclrtDestroyStream(stream), "destroy stream");
        stream = nullptr;
        CheckAcl(aclrtResetDevice(deviceId), "reset device");
        CheckAcl(aclFinalize(), "aclFinalize");
        initialized = false;
        std::cout << "RUN_COMPLETE=YES timed_samples=" << sequence << '\n';
        return sequence == 768 ? 0 : 3;
    } catch (const std::exception& error) {
        raw.flush();
        references.flush();
        std::cerr << "RUN_FAILED " << error.what() << '\n';
        if (stream) aclrtDestroyStream(stream);
        if (initialized) {
            aclrtResetDevice(deviceId);
            aclFinalize();
        }
        return 1;
    }
}
