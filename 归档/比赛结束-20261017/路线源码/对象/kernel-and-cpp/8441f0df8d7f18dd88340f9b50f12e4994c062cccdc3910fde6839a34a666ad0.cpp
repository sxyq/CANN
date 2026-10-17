#include <acl/acl.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <string>
#include <vector>
#include "../../V001/local_types.h"

extern "C" void r10_parent_run_kernel(void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    int64_t, aclrtStream, float);
extern "C" void r10_candidate_run_kernel(void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    void*, const TensorGroupInfo&, void*, const TensorGroupInfo&, void*, const TensorGroupInfo&,
    int64_t, aclrtStream, float);

namespace {
constexpr float kEpsilon = 1.0e-5f;
constexpr int kWarmup = 60;
constexpr int kBlocks = 2;
constexpr int kPairs = 31;

void RequireAcl(aclError result, const char* operation)
{
    if (result != ACL_SUCCESS) {
        std::fprintf(stderr, "ACL_FAILURE operation=%s code=%d\n", operation, result);
        std::exit(2);
    }
}

FILE* OpenOutput(const std::string& path)
{
    FILE* file = std::fopen(path.c_str(), "wx");
    if (!file) { std::perror(path.c_str()); std::exit(2); }
    return file;
}

bool Memory(const char* stage)
{
    size_t freeBytes = 0, totalBytes = 0;
    RequireAcl(aclrtGetMemInfo(ACL_HBM_MEM, &freeBytes, &totalBytes), "read HBM");
    std::printf("RESOURCE stage=%s free_hbm_mb=%.6f total_hbm_mb=%.6f\n",
                stage, freeBytes / 1048576.0, totalBytes / 1048576.0);
    return freeBytes >= 100ull * 1024 * 1024;
}

uint16_t Bf16(float value)
{
    uint32_t bits;
    std::memcpy(&bits, &value, sizeof(bits));
    bits += 0x7fffU + ((bits >> 16) & 1U);
    return static_cast<uint16_t>(bits >> 16);
}

float HalfRound(float value) { return aclFloat16ToFloat(aclFloatToFloat16(value)); }

void Store(std::vector<uint8_t>& data, size_t index, int dtype, float value)
{
    if (dtype == 0) { std::memcpy(data.data() + index * 4, &value, 4); return; }
    uint16_t bits = 0;
    if (dtype == 1) {
        const aclFloat16 half = aclFloatToFloat16(value);
        std::memcpy(&bits, &half, 2);
    } else bits = Bf16(value);
    std::memcpy(data.data() + index * 2, &bits, 2);
}

float Load(const std::vector<uint8_t>& data, size_t index, int dtype)
{
    float value = 0;
    if (dtype == 0) { std::memcpy(&value, data.data() + index * 4, 4); return value; }
    uint16_t bits = 0;
    std::memcpy(&bits, data.data() + index * 2, 2);
    if (dtype == 1) {
        aclFloat16 half;
        std::memcpy(&half, &bits, 2);
        return aclFloat16ToFloat(half);
    }
    const uint32_t wide = static_cast<uint32_t>(bits) << 16;
    std::memcpy(&value, &wide, 4);
    return value;
}

float InputValue(int64_t index, int multiplier, int offset)
{
    return static_cast<float>((index * multiplier + offset) % 997) / 498.0f - 1.0f;
}

struct Sample {
    int ordinal, block, pair, position, side;
    double deviceUs, wallUs;
};

struct DiagnosticSample {
    int ordinal, block, pair, cell, position, slot, library, intended, output;
    int previousLibrary, previousOutput, previousOrdinal;
    bool conditioner;
    double deviceUs, wallUs;
};

void WriteDiagnostic(const std::string& path, const std::vector<DiagnosticSample>& samples,
                     bool paired, void* const* outputs)
{
    FILE* out = OpenOutput(path);
    std::fprintf(out, "launch_ordinal\tblock\tpair\tcell\tposition\tlogical_slot\tside\tactual_library\toutput_slot\toutput_address\tprevious_library\tprevious_output_slot\tprevious_ordinal\trole\tcondition_library\tcondition_other_output\taddress_swap\tfirst_slot\tkernel_swap\tdevice_us\twall_us\n");
    for (const auto& sample : samples) {
        const char* label = sample.conditioner ? (sample.library ? "C" : "P") :
            (paired ? (sample.intended ? "C" : "P") : (sample.intended ? "P2" : "P1"));
        std::fprintf(out, "%d\t%d\t%d\t%d\t%d\t%d\t%s\t%s\t%c\t%p\t%s\t%c\t%d\t%s\t%s\t%d\t%d\t%d\t%d\t%.9f\t%.9f\n",
            sample.ordinal, sample.block, sample.pair, sample.cell, sample.position, sample.slot,
            label, sample.library ? "C" : "P", 'A' + sample.output, outputs[sample.output],
            sample.previousLibrary ? "C" : "P", 'A' + sample.previousOutput, sample.previousOrdinal,
            sample.conditioner ? "conditioner" : "probe", sample.cell & 1 ? "C" : "P",
            (sample.cell >> 1) & 1, (sample.cell >> 2) & 1, (sample.cell >> 3) & 1,
            (sample.cell >> 4) & 1, sample.deviceUs, sample.wallUs);
    }
    std::fclose(out);
}

void WriteSamples(const std::string& path, const std::vector<Sample>& samples, bool paired)
{
    FILE* out = OpenOutput(path);
    std::fprintf(out, "launch_ordinal\tblock\tpair\tposition\tside\tdevice_us\twall_us\n");
    for (const auto& sample : samples) {
        const char* label = paired ? (sample.side == 0 ? "P" : "C") : (sample.side == 0 ? "P1" : "P2");
        std::fprintf(out, "%d\t%d\t%d\t%d\t%s\t%.9f\t%.9f\n", sample.ordinal,
                     sample.block, sample.pair, sample.position, label, sample.deviceUs, sample.wallUs);
    }
    std::fclose(out);
}
}

int main(int argc, char** argv)
{
    if (argc != 7) {
        std::fprintf(stderr, "usage: r10_probe correctness|measure|diagnose DEVICE ROWS WIDTH DTYPE OUTPUT_PREFIX\n");
        return 2;
    }
    const std::string mode = argv[1];
    const int device = std::stoi(argv[2]);
    const int64_t rows = std::stoll(argv[3]), width = std::stoll(argv[4]);
    const int dtype = std::stoi(argv[5]);
    const std::string prefix = argv[6];
    if ((mode != "correctness" && mode != "measure" && mode != "diagnose") || rows < 1 || width < 64 || width > 32768 || dtype < 0 || dtype > 2) return 2;
    if (mode == "diagnose" && (dtype != 2 || width != 12288 || (rows != 128 && rows != 48))) return 2;
    const size_t count = static_cast<size_t>(rows * width);
    const size_t bytes = dtype == 0 ? 4 : 2;
    const size_t dataBytes = count * bytes, paramBytes = width * bytes;
    std::vector<uint8_t> x(dataBytes), residual(dataBytes), gamma(paramBytes), bias(paramBytes), output(dataBytes);
    for (size_t i = 0; i < count; ++i) {
        Store(x, i, dtype, InputValue(i, 37, 11));
        Store(residual, i, dtype, InputValue(i, 17, 3));
        Store(output, i, dtype, NAN);
    }
    for (int64_t i = 0; i < width; ++i) {
        Store(gamma, i, dtype, 0.75f + static_cast<float>((i * 13) % 100) / 200.0f);
        Store(bias, i, dtype, static_cast<float>((i * 7) % 100) / 400.0f - 0.125f);
    }

    RequireAcl(aclInit(nullptr), "init");
    RequireAcl(aclrtSetDevice(device), "set device");
    int64_t cores = 0;
    RequireAcl(aclrtGetDeviceInfo(device, ACL_DEV_ATTR_VECTOR_CORE_NUM, &cores), "read vector cores");
    aclrtStream stream = nullptr;
    RequireAcl(aclrtCreateStream(&stream), "create stream");
    if (!Memory("before_allocation")) return 4;
    void* buffers[5] = {};
    const std::vector<uint8_t>* host[] = {&x, &residual, &gamma, &bias, &output};
    for (int i = 0; i < 5; ++i) {
        RequireAcl(aclrtMalloc(&buffers[i], host[i]->size(), ACL_MEM_MALLOC_HUGE_FIRST), "allocate");
        RequireAcl(aclrtMemcpy(buffers[i], host[i]->size(), host[i]->data(), host[i]->size(), ACL_MEMCPY_HOST_TO_DEVICE), "copy inputs");
    }
    const int64_t dataShape[] = {rows, width}, paramShape[] = {width};
    const TensorInfo dataInfo{dataShape, 2, dtype}, paramInfo{paramShape, 1, dtype};
    const TensorGroupInfo dataGroup{&dataInfo, 1}, paramGroup{&paramInfo, 1};
    void* outputs[2] = {buffers[4], nullptr};
    if (mode == "diagnose") {
        RequireAcl(aclrtMalloc(&outputs[1], dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "allocate output B");
        RequireAcl(aclrtMemcpy(outputs[1], dataBytes, output.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "fill output B");
    }
    int ordinal = 0, lastLibrary = 0, lastOutput = 0;
    auto launch = [&](bool candidate, int outputIndex = 0) {
        auto function = candidate ? r10_candidate_run_kernel : r10_parent_run_kernel;
        function(buffers[0], dataGroup, buffers[1], dataGroup, buffers[2], paramGroup,
                 buffers[3], paramGroup, outputs[outputIndex], dataGroup, cores, stream, kEpsilon);
        ++ordinal;
        lastLibrary = candidate;
        lastOutput = outputIndex;
    };
    uint64_t parentBlocks = std::min<uint64_t>(std::max<int64_t>(cores, 1), rows);
    uint64_t resident = 1;
    for (uint64_t batch = 8; batch > 1; --batch) {
        if (width * 4 * batch + 4 * 4096 * 2 + 2 * 4096 * 4 + batch * 16 * 4 <= 176 * 1024) {
            resident = batch;
            break;
        }
    }
    const uint64_t q = rows / parentBlocks + (rows % parentBlocks != 0);
    const bool guard = dtype == 2 && width > 8192 && resident > 1 && q > resident && q % resident == 0 && rows % q == 0;
    const uint64_t candidateBlocks = guard ? rows / q : parentBlocks;
    std::printf("PROBE mode=%s device=%d rows=%lld width=%lld dtype=%d availableCoreNum=%lld predicted_parent_blocks=%llu predicted_candidate_blocks=%llu resident_rows=%llu max_rows=%llu guard=%s output_address=%p epsilon=%.9g\n",
                mode.c_str(), device, (long long)rows, (long long)width, dtype, (long long)cores,
                (unsigned long long)parentBlocks, (unsigned long long)candidateBlocks,
                (unsigned long long)resident, (unsigned long long)q, guard ? "ON" : "OFF", buffers[4], kEpsilon);
    auto verifyOutput = [&](FILE* report, const char* label) {
        size_t totalBad = 0, totalNonfinite = 0;
        double totalMax = 0;
        const double atol = dtype == 0 ? std::ldexp(1.0, -16) : std::ldexp(1.0, dtype == 1 ? -9 : -6);
        const double rtol = std::ldexp(1.0, dtype == 0 ? -10 : (dtype == 1 ? -9 : -6));
        const double absLimit = dtype == 0 ? 0.01 : (dtype == 1 ? 0.1 : 1.0);
        for (int64_t row = 0; row < rows; ++row) {
            double squares = 0;
            for (int64_t col = 0; col < width; ++col) {
                const size_t i = row * width + col;
                double u = static_cast<double>(Load(x, i, dtype)) + Load(residual, i, dtype);
                if (dtype == 1) u = HalfRound(static_cast<float>(u));
                squares += u * u;
            }
            const double inverse = 1.0 / std::sqrt(squares / width + kEpsilon);
            size_t bad = 0, nonfinite = 0, worstIndex = row * width;
            double maxAbs = 0, worstReference = 0, worstActual = 0;
            for (int64_t col = 0; col < width; ++col) {
                const size_t i = row * width + col;
                double u = static_cast<double>(Load(x, i, dtype)) + Load(residual, i, dtype);
                if (dtype == 1) u = HalfRound(static_cast<float>(u));
                double reference = u * inverse * Load(gamma, col, dtype) + Load(bias, col, dtype);
                if (dtype == 1) reference = HalfRound(HalfRound(HalfRound(static_cast<float>(u * inverse)) * Load(gamma, col, dtype)) + Load(bias, col, dtype));
                const double actual = Load(output, i, dtype), error = std::fabs(actual - reference);
                const bool finite = std::isfinite(actual);
                nonfinite += !finite;
                bad += !finite || error > atol + rtol * std::fabs(reference) || error > absLimit;
                if (error > maxAbs || !finite) {
                    maxAbs = error; worstIndex = i; worstReference = reference; worstActual = actual;
                }
            }
            totalBad += bad; totalNonfinite += nonfinite; totalMax = std::max(totalMax, maxAbs);
            std::fprintf(report, "%s\t%lld\t%lld\t%zu\t%zu\t%.17g\t%zu\t%.17g\t%.17g\t%.17g\t%.17g\n",
                         label, (long long)row, (long long)width, bad, nonfinite,
                         maxAbs, worstIndex, worstReference, worstActual, atol, rtol);
        }
        std::printf("CORRECTNESS side=%s reference=CPU_FP64 failures=%zu nonfinite=%zu elements=%zu max_abs=%.17g require_all=YES result=%s\n",
                    label, totalBad, totalNonfinite, count, totalMax, totalBad == 0 ? "PASS" : "FAIL");
        return totalBad == 0 ? 0 : 3;
    };
    int result = 0;
    if (mode == "correctness") {
        FILE* report = OpenOutput(prefix + ".reference.tsv");
        std::fprintf(report, "side\trow\telements\tfailures\tnonfinite\tmax_abs\tworst_index\treference\tactual\tatol\trtol\n");
        for (int side = 0; side < 2; ++side) {
            for (size_t i = 0; i < count; ++i) Store(output, i, dtype, NAN);
            RequireAcl(aclrtMemcpy(buffers[4], dataBytes, output.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "write NaN sentinel");
            launch(side == 1);
            RequireAcl(aclrtSynchronizeStream(stream), "reference synchronize");
            RequireAcl(aclrtMemcpy(output.data(), dataBytes, buffers[4], dataBytes, ACL_MEMCPY_DEVICE_TO_HOST), "read output");
            FILE* blob = OpenOutput(prefix + (side == 0 ? ".parent.bin" : ".candidate.bin"));
            const size_t written = std::fwrite(output.data(), 1, output.size(), blob);
            std::fclose(blob);
            if (written != output.size()) return 2;
            if (verifyOutput(report, side == 0 ? "P" : "C") != 0) result = 3;
        }
        std::fclose(report);
    } else if (mode == "diagnose") {
        Dl_info parentInfo{}, candidateInfo{};
        if (!dladdr(reinterpret_cast<void*>(r10_parent_run_kernel), &parentInfo) ||
            !dladdr(reinterpret_cast<void*>(r10_candidate_run_kernel), &candidateInfo)) return 2;
        std::printf("LIBRARIES parent_path=%s parent_base=%p parent_function=%p candidate_path=%s candidate_base=%p candidate_function=%p\n",
            parentInfo.dli_fname, parentInfo.dli_fbase, reinterpret_cast<void*>(r10_parent_run_kernel),
            candidateInfo.dli_fname, candidateInfo.dli_fbase, reinterpret_cast<void*>(r10_candidate_run_kernel));
        std::printf("ADDRESSES x=%p residual=%p gamma=%p bias=%p output_A=%p output_B=%p stream=%p\n",
                    buffers[0], buffers[1], buffers[2], buffers[3], outputs[0], outputs[1], stream);
        FILE* report = OpenOutput(prefix + ".reference.tsv");
        std::fprintf(report, "side\trow\telements\tfailures\tnonfinite\tmax_abs\tworst_index\treference\tactual\tatol\trtol\n");
        for (int side = 0; side < 2; ++side) {
            for (int address = 0; address < 2; ++address) {
                for (size_t i = 0; i < count; ++i) Store(output, i, dtype, NAN);
                RequireAcl(aclrtMemcpy(outputs[address], dataBytes, output.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "write diagnostic NaN");
                launch(side == 1, address);
                RequireAcl(aclrtSynchronizeStream(stream), "diagnostic reference synchronize");
                RequireAcl(aclrtMemcpy(output.data(), dataBytes, outputs[address], dataBytes, ACL_MEMCPY_DEVICE_TO_HOST), "read diagnostic output");
                const std::string label = std::string("PRE_") + (side ? "C_" : "P_") + char('A' + address);
                if (verifyOutput(report, label.c_str()) != 0) result = 3;
            }
        }
        std::fflush(report);
        if (result == 0) {
            if (!Memory("before_diagnostic")) return 4;
            aclrtEvent start = nullptr, stop = nullptr;
            RequireAcl(aclrtCreateEvent(&start), "create diagnostic start");
            RequireAcl(aclrtCreateEvent(&stop), "create diagnostic stop");
            for (int phase = 0; phase < 2; ++phase) {
                const bool paired = phase == 1;
                std::vector<DiagnosticSample> samples;
                samples.reserve(256);
                for (int i = 0; i < kWarmup; ++i) {
                    launch(false, i % 2);
                    RequireAcl(aclrtSynchronizeStream(stream), "diagnostic warmup P");
                    launch(true, i % 2);
                    RequireAcl(aclrtSynchronizeStream(stream), "diagnostic warmup C");
                }
                std::printf("DIAGNOSTIC_PHASE phase=%s output_A=%p output_B=%p parent_base=%p candidate_base=%p start_event=%p stop_event=%p next_ordinal=%d\n",
                    paired ? "PC" : "PP", outputs[0], outputs[1], parentInfo.dli_fbase, candidateInfo.dli_fbase, start, stop, ordinal + 1);
                for (int block = 0; block < 2; ++block) {
                    for (int pair = 0; pair < 32; ++pair) {
                        const int cell = (pair * (block == 0 ? 13 : 21) + (block == 0 ? 7 : 19)) % 32;
                        const int conditionLibrary = cell & 1, otherOutput = (cell >> 1) & 1;
                        const int addressSwap = (cell >> 2) & 1, firstSlot = (cell >> 3) & 1, kernelSwap = (cell >> 4) & 1;
                        for (int position = 0; position < 2; ++position) {
                            const int slot = (firstSlot + position) % 2, address = slot ^ addressSwap;
                            const int intended = slot ^ kernelSwap;
                            for (int part = 0; part < 2; ++part) {
                                const bool conditioner = part == 0;
                                const int library = conditioner ? conditionLibrary : (paired ? intended : 0);
                                const int destination = conditioner ? address ^ otherOutput : address;
                                const int previousLibrary = lastLibrary, previousOutput = lastOutput, previousOrdinal = ordinal;
                                const auto wallStart = std::chrono::steady_clock::now();
                                RequireAcl(aclrtRecordEvent(start, stream), "diagnostic record start");
                                launch(library == 1, destination);
                                RequireAcl(aclrtRecordEvent(stop, stream), "diagnostic record stop");
                                RequireAcl(aclrtSynchronizeEvent(stop), "diagnostic sample synchronize");
                                const auto wallStop = std::chrono::steady_clock::now();
                                float elapsed = 0;
                                RequireAcl(aclrtEventElapsedTime(&elapsed, start, stop), "diagnostic elapsed time");
                                samples.push_back({ordinal, block, pair, cell, position, slot, library, intended,
                                    destination, previousLibrary, previousOutput, previousOrdinal, conditioner,
                                    elapsed * 1000.0, std::chrono::duration<double, std::micro>(wallStop - wallStart).count()});
                            }
                        }
                    }
                }
                WriteDiagnostic(prefix + (paired ? ".pc.tsv" : ".pp.tsv"), samples, paired, outputs);
                std::printf("DIAGNOSTIC_SAMPLES phase=%s count=%zu probes=128 conditioners=128 blocks=2 cells=32 warmup_per_library=60 raw_flush=AFTER_PHASE first_ordinal=%d last_ordinal=%d\n",
                    paired ? "PC" : "PP", samples.size(), samples.front().ordinal, samples.back().ordinal);
            }
            RequireAcl(aclrtDestroyEvent(stop), "destroy diagnostic stop");
            RequireAcl(aclrtDestroyEvent(start), "destroy diagnostic start");
            for (int address = 0; address < 2; ++address) {
                RequireAcl(aclrtMemcpy(output.data(), dataBytes, outputs[address], dataBytes, ACL_MEMCPY_DEVICE_TO_HOST), "read final diagnostic output");
                const std::string label = std::string("POST_") + char('A' + address);
                if (verifyOutput(report, label.c_str()) != 0) result = 3;
            }
            Memory("after_diagnostic");
        }
        std::fclose(report);
    } else {
        if (!Memory("before_local")) return 4;
        aclrtEvent start = nullptr, stop = nullptr;
        RequireAcl(aclrtCreateEvent(&start), "create start event");
        RequireAcl(aclrtCreateEvent(&stop), "create stop event");
        for (int phase = 0; phase < 2; ++phase) {
            const bool paired = phase == 1;
            std::vector<Sample> samples;
            samples.reserve(kBlocks * kPairs * 2);
            for (int i = 0; i < kWarmup; ++i) {
                launch(false);
                RequireAcl(aclrtSynchronizeStream(stream), "warmup P");
                if (paired) { launch(true); RequireAcl(aclrtSynchronizeStream(stream), "warmup C"); }
            }
            for (int block = 0; block < kBlocks; ++block) {
                for (int pair = 0; pair < kPairs; ++pair) {
                    for (int position = 0; position < 2; ++position) {
                        const int side = ((block + pair) % 2 + position) % 2;
                        const auto wallStart = std::chrono::steady_clock::now();
                        RequireAcl(aclrtRecordEvent(start, stream), "record start");
                        launch(paired && side == 1);
                        RequireAcl(aclrtRecordEvent(stop, stream), "record stop");
                        RequireAcl(aclrtSynchronizeEvent(stop), "sample synchronize");
                        const auto wallStop = std::chrono::steady_clock::now();
                        float elapsed = 0;
                        RequireAcl(aclrtEventElapsedTime(&elapsed, start, stop), "elapsed time");
                        samples.push_back({ordinal, block, pair, position, side, elapsed * 1000.0,
                            std::chrono::duration<double, std::micro>(wallStop - wallStart).count()});
                    }
                }
            }
            WriteSamples(prefix + (paired ? ".pc.tsv" : ".pp.tsv"), samples, paired);
            std::printf("SAMPLES phase=%s count=%zu warmup_per_function=60 blocks=2 pairs_per_block=31 batch_n=1 first_ordinal=%d last_ordinal=%d same_output_address=YES raw_flush=AFTER_PHASE\n",
                        paired ? "PC" : "PP", samples.size(), samples.front().ordinal, samples.back().ordinal);
        }
        RequireAcl(aclrtDestroyEvent(stop), "destroy stop");
        RequireAcl(aclrtDestroyEvent(start), "destroy start");
        Memory("after_local");
    }
    RequireAcl(aclrtSynchronizeStream(stream), "final synchronize");
    if (outputs[1]) RequireAcl(aclrtFree(outputs[1]), "free own output B");
    for (void* buffer : buffers) RequireAcl(aclrtFree(buffer), "free own buffer");
    RequireAcl(aclrtDestroyStream(stream), "destroy stream");
    RequireAcl(aclFinalize(), "finalize");
    std::printf("RUN_COMPLETE result=%d host_launch_calls=%d RUNNING_DEVICE_OPERATION=NONE\n", result, ordinal);
    return result;
}
