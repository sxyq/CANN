#include <acl/acl.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
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
        std::fprintf(stderr, "usage: r10_probe correctness|measure DEVICE ROWS WIDTH DTYPE OUTPUT_PREFIX\n");
        return 2;
    }
    const std::string mode = argv[1];
    const int device = std::stoi(argv[2]);
    const int64_t rows = std::stoll(argv[3]), width = std::stoll(argv[4]);
    const int dtype = std::stoi(argv[5]);
    const std::string prefix = argv[6];
    if ((mode != "correctness" && mode != "measure") || rows < 1 || width < 64 || width > 32768 || dtype < 0 || dtype > 2) return 2;
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
    int ordinal = 0;
    auto launch = [&](bool candidate) {
        auto function = candidate ? r10_candidate_run_kernel : r10_parent_run_kernel;
        function(buffers[0], dataGroup, buffers[1], dataGroup, buffers[2], paramGroup,
                 buffers[3], paramGroup, buffers[4], dataGroup, cores, stream, kEpsilon);
        ++ordinal;
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
                             side == 0 ? "P" : "C", (long long)row, (long long)width, bad, nonfinite,
                             maxAbs, worstIndex, worstReference, worstActual, atol, rtol);
            }
            std::printf("CORRECTNESS side=%s reference=CPU_FP64 failures=%zu nonfinite=%zu elements=%zu max_abs=%.17g require_all=YES result=%s\n",
                        side == 0 ? "P" : "C", totalBad, totalNonfinite, count, totalMax, totalBad == 0 ? "PASS" : "FAIL");
            if (totalBad != 0) result = 3;
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
    for (void* buffer : buffers) RequireAcl(aclrtFree(buffer), "free own buffer");
    RequireAcl(aclrtDestroyStream(stream), "destroy stream");
    RequireAcl(aclFinalize(), "finalize");
    std::printf("RUN_COMPLETE result=%d host_launch_calls=%d RUNNING_DEVICE_OPERATION=NONE\n", result, ordinal);
    return result;
}
