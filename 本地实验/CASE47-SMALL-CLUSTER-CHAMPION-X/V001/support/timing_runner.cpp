#include <acl/acl.h>
#include "judge_abi.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <dlfcn.h>
#include <fstream>
#include <iomanip>
#include <string>
#include <thread>
#include <unistd.h>
#include <vector>

using RunKernel = void (*)(
    GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&,
    int64_t, aclrtStream, float);

namespace {

constexpr float kEpsilon = 1.0e-5f;

struct Distribution {
    double median = 0.0;
    double mean = 0.0;
    double stdev = 0.0;
    double cv = 0.0;
    double mad = 0.0;
    double p10 = 0.0;
    double p90 = 0.0;
    double min = 0.0;
    double max = 0.0;
    double maxMin = 0.0;
    double span = 0.0;
};

double Median(std::vector<double> values)
{
    if (values.empty()) return 0.0;
    std::sort(values.begin(), values.end());
    const size_t middle = values.size() / 2;
    if ((values.size() & 1U) != 0) return values[middle];
    return (values[middle - 1] + values[middle]) * 0.5;
}

double Percentile(const std::vector<double>& sorted, double fraction)
{
    if (sorted.empty()) return 0.0;
    const double pos = fraction * static_cast<double>(sorted.size() - 1);
    const size_t low = static_cast<size_t>(pos);
    const size_t high = std::min(low + 1, sorted.size() - 1);
    const double part = pos - static_cast<double>(low);
    return sorted[low] * (1.0 - part) + sorted[high] * part;
}

Distribution Summarize(std::vector<double> values)
{
    Distribution result;
    if (values.empty()) return result;
    result.min = *std::min_element(values.begin(), values.end());
    result.max = *std::max_element(values.begin(), values.end());
    for (double value : values) result.mean += value;
    result.mean /= static_cast<double>(values.size());
    double variance = 0.0;
    for (double value : values) variance += (value - result.mean) * (value - result.mean);
    variance /= static_cast<double>(values.size());
    result.stdev = std::sqrt(variance);
    result.cv = result.mean > 0.0 ? result.stdev / result.mean : 0.0;
    result.median = Median(values);
    std::vector<double> deviations;
    deviations.reserve(values.size());
    for (double value : values) deviations.push_back(std::fabs(value - result.median));
    result.mad = Median(deviations);
    std::sort(values.begin(), values.end());
    result.p10 = Percentile(values, 0.10);
    result.p90 = Percentile(values, 0.90);
    result.maxMin = result.min > 0.0 ? result.max / result.min : 0.0;
    result.span = result.max - result.min;
    return result;
}

bool AclOk(aclError rc, const char* expression, int line)
{
    if (rc == ACL_SUCCESS) return true;
    std::fprintf(stderr, "ACL failure %s=%d at line %d\n", expression, rc, line);
    return false;
}

#define ACL_REQUIRE(expr) do { if (!AclOk((expr), #expr, __LINE__)) return 1; } while (0)

uint16_t FloatToBf16(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    bits += 0x7fffU + ((bits >> 16) & 1U);
    return static_cast<uint16_t>(bits >> 16);
}

void StoreValue(std::vector<uint8_t>& data, size_t index, int dtype, float value)
{
    if (dtype == 0) {
        std::memcpy(data.data() + index * sizeof(value), &value, sizeof(value));
        return;
    }
    uint16_t bits = 0;
    if (dtype == 1) {
        const aclFloat16 half = aclFloatToFloat16(value);
        std::memcpy(&bits, &half, sizeof(bits));
    } else {
        bits = FloatToBf16(value);
    }
    std::memcpy(data.data() + index * sizeof(bits), &bits, sizeof(bits));
}

float InputValue(int64_t index, int multiplier, int offset)
{
    return static_cast<float>((index * multiplier + offset) % 997) / 498.0f - 1.0f;
}

int QueryCores(int device)
{
    int64_t cores = 0;
    if (!AclOk(aclInit(nullptr), "aclInit", __LINE__)) return 1;
    if (!AclOk(aclrtSetDevice(device), "aclrtSetDevice", __LINE__)) return 1;
    if (!AclOk(aclrtGetDeviceInfo(device, ACL_DEV_ATTR_VECTOR_CORE_NUM, &cores),
               "aclrtGetDeviceInfo", __LINE__)) return 1;
    if (!AclOk(aclFinalize(), "aclFinalize", __LINE__)) return 1;
    std::printf("%lld\n", static_cast<long long>(cores));
    return 0;
}

void WriteDistribution(std::ofstream& out, int block, const Distribution& s, size_t count)
{
    out << block << '\t' << count << '\t' << s.median << '\t' << s.mean << '\t'
        << s.stdev << '\t' << s.cv << '\t' << s.mad << '\t' << s.p10 << '\t'
        << s.p90 << '\t' << s.min << '\t' << s.max << '\t' << s.maxMin << '\t'
        << s.span << '\n';
}

}  // namespace

int main(int argc, char** argv)
{
    if (argc == 3 && std::strcmp(argv[1], "--query-cores") == 0) {
        return QueryCores(std::atoi(argv[2]));
    }
    if (argc != 13) {
        std::fprintf(stderr,
            "usage: %s module.so device rows width dtype prefix stage process_rep warmup samples blocks gap_sec\n",
            argv[0]);
        return 2;
    }

    const char* modulePath = argv[1];
    const int device = std::atoi(argv[2]);
    const int64_t rows = std::strtoll(argv[3], nullptr, 10);
    const int64_t width = std::strtoll(argv[4], nullptr, 10);
    const int dtype = std::atoi(argv[5]);
    const std::string prefix = argv[6];
    const std::string stage = argv[7];
    const int processRep = std::atoi(argv[8]);
    const int warmups = std::atoi(argv[9]);
    const int sampleCount = std::atoi(argv[10]);
    const int blockCount = std::atoi(argv[11]);
    const int gapSeconds = std::atoi(argv[12]);
    const bool sameBinary = stage == "SAME_BINARY";
    const bool precheck = stage == "PRECHECK-A" || stage == "PRECHECK-B";
    const bool paired = stage == "PAIR-P" || stage == "PAIR-C";
    if (device < 0 || rows < 1 || width < 1 || dtype < 0 || dtype > 2 ||
        (!sameBinary && !precheck && !paired) || processRep < 1 ||
        (warmups != 45 && warmups != 60) || sampleCount != 21 ||
        (sameBinary && (processRep != 1 || blockCount != 2 || gapSeconds != 30)) ||
        ((precheck || paired) && (blockCount != 1 || gapSeconds != 0)) ||
        (precheck && processRep > 6) || (paired && processRep > 4)) {
        std::fprintf(stderr, "invalid timing parameters\n");
        return 2;
    }

    void* module = dlopen(modulePath, RTLD_NOW | RTLD_LOCAL);
    if (module == nullptr) {
        std::fprintf(stderr, "dlopen failed: %s\n", dlerror());
        return 1;
    }
    dlerror();
    auto runKernel = reinterpret_cast<RunKernel>(dlsym(module, "run_kernel"));
    const char* symbolError = dlerror();
    if (symbolError != nullptr || runKernel == nullptr) {
        std::fprintf(stderr, "dlsym run_kernel failed: %s\n",
                     symbolError == nullptr ? "null symbol" : symbolError);
        dlclose(module);
        return 1;
    }

    const size_t elementBytes = dtype == 0 ? sizeof(float) : sizeof(uint16_t);
    const size_t elements = static_cast<size_t>(rows) * static_cast<size_t>(width);
    const size_t dataBytes = elements * elementBytes;
    const size_t parameterBytes = static_cast<size_t>(width) * elementBytes;
    std::vector<uint8_t> hostX(dataBytes), hostResidual(dataBytes);
    std::vector<uint8_t> hostGamma(parameterBytes), hostBias(parameterBytes);
    std::vector<uint8_t> hostOutput(dataBytes);
    void *deviceX = nullptr, *deviceResidual = nullptr, *deviceGamma = nullptr;
    void *deviceBias = nullptr, *deviceOutput = nullptr;
    aclrtStream stream = nullptr;
    aclrtEvent eventStart = nullptr, eventStop = nullptr;
    int64_t coreCount = 0;

    ACL_REQUIRE(aclInit(nullptr));
    ACL_REQUIRE(aclrtSetDevice(device));
    ACL_REQUIRE(aclrtGetDeviceInfo(device, ACL_DEV_ATTR_VECTOR_CORE_NUM, &coreCount));
    ACL_REQUIRE(aclrtCreateStream(&stream));
    ACL_REQUIRE(aclrtMalloc(&deviceX, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_REQUIRE(aclrtMalloc(&deviceResidual, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_REQUIRE(aclrtMalloc(&deviceGamma, parameterBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_REQUIRE(aclrtMalloc(&deviceBias, parameterBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_REQUIRE(aclrtMalloc(&deviceOutput, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));

    for (size_t i = 0; i < elements; ++i) {
        StoreValue(hostX, i, dtype, InputValue(static_cast<int64_t>(i), 37, 11));
        StoreValue(hostResidual, i, dtype, InputValue(static_cast<int64_t>(i), 17, 3));
    }
    for (int64_t i = 0; i < width; ++i) {
        StoreValue(hostGamma, static_cast<size_t>(i), dtype,
                   0.75f + static_cast<float>((i * 13) % 100) / 200.0f);
        StoreValue(hostBias, static_cast<size_t>(i), dtype,
                   static_cast<float>((i * 7) % 100) / 400.0f - 0.125f);
    }
    ACL_REQUIRE(aclrtMemcpy(deviceX, dataBytes, hostX.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_REQUIRE(aclrtMemcpy(deviceResidual, dataBytes, hostResidual.data(), dataBytes,
                            ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_REQUIRE(aclrtMemcpy(deviceGamma, parameterBytes, hostGamma.data(), parameterBytes,
                            ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_REQUIRE(aclrtMemcpy(deviceBias, parameterBytes, hostBias.data(), parameterBytes,
                            ACL_MEMCPY_HOST_TO_DEVICE));

    int64_t xyShape[] = {rows, width};
    int64_t parameterShape[] = {width};
    const TensorInfo xTensor = {xyShape, 2, dtype};
    const TensorInfo residualTensor = {xyShape, 2, dtype};
    const TensorInfo gammaTensor = {parameterShape, 1, dtype};
    const TensorInfo biasTensor = {parameterShape, 1, dtype};
    const TensorInfo outputTensor = {xyShape, 2, dtype};
    const TensorGroupInfo infoX = {&xTensor, 1};
    const TensorGroupInfo infoResidual = {&residualTensor, 1};
    const TensorGroupInfo infoGamma = {&gammaTensor, 1};
    const TensorGroupInfo infoBias = {&biasTensor, 1};
    const TensorGroupInfo infoOutput = {&outputTensor, 1};
    auto launch = [&]() {
        runKernel(reinterpret_cast<GM_ADDR>(deviceX), infoX,
                  reinterpret_cast<GM_ADDR>(deviceResidual), infoResidual,
                  reinterpret_cast<GM_ADDR>(deviceGamma), infoGamma,
                  reinterpret_cast<GM_ADDR>(deviceBias), infoBias,
                  reinterpret_cast<GM_ADDR>(deviceOutput), infoOutput,
                  coreCount, stream, kEpsilon);
    };

    for (int i = 0; i < warmups; ++i) {
        launch();
        ACL_REQUIRE(aclrtSynchronizeStream(stream));
    }
    ACL_REQUIRE(aclrtCreateEvent(&eventStart));
    ACL_REQUIRE(aclrtCreateEvent(&eventStop));

    const std::string rawPath = prefix + ".raw.tsv";
    const std::string statsPath = prefix + ".stats.tsv";
    std::ofstream raw(rawPath);
    if (!raw) {
        std::fprintf(stderr, "cannot open %s\n", rawPath.c_str());
        return 1;
    }
    raw << "stage\tprocess_rep\tpid\tblock\trep\tdevice_event_us\thost_wall_us\n"
        << std::setprecision(12);
    std::vector<std::vector<double>> blockDevice(static_cast<size_t>(blockCount));
    std::vector<std::vector<double>> blockWall(static_cast<size_t>(blockCount));

    for (int block = 0; block < blockCount; ++block) {
        if (block > 0) std::this_thread::sleep_for(std::chrono::seconds(gapSeconds));
        ACL_REQUIRE(aclrtSynchronizeStream(stream));
        for (int rep = 0; rep < sampleCount; ++rep) {
            const auto wallStart = std::chrono::steady_clock::now();
            ACL_REQUIRE(aclrtRecordEvent(eventStart, stream));
            launch();
            ACL_REQUIRE(aclrtRecordEvent(eventStop, stream));
            ACL_REQUIRE(aclrtSynchronizeEvent(eventStop));
            float deviceMs = 0.0f;
            ACL_REQUIRE(aclrtEventElapsedTime(&deviceMs, eventStart, eventStop));
            ACL_REQUIRE(aclrtSynchronizeStream(stream));
            const auto wallStop = std::chrono::steady_clock::now();
            const double deviceUs = static_cast<double>(deviceMs) * 1000.0;
            const double wallUs = std::chrono::duration<double, std::micro>(wallStop - wallStart).count();
            blockDevice[static_cast<size_t>(block)].push_back(deviceUs);
            blockWall[static_cast<size_t>(block)].push_back(wallUs);
            raw << stage << '\t' << processRep << '\t' << getpid() << '\t'
                << (block + 1) << '\t' << (rep + 1) << '\t'
                << deviceUs << '\t' << wallUs << '\n';
            raw.flush();
            if (!raw) {
                std::fprintf(stderr, "write failed: %s\n", rawPath.c_str());
                return 1;
            }
        }
    }
    raw.close();
    ACL_REQUIRE(aclrtMemcpy(hostOutput.data(), dataBytes, deviceOutput, dataBytes,
                            ACL_MEMCPY_DEVICE_TO_HOST));

    std::ofstream stats(statsPath);
    if (!stats) {
        std::fprintf(stderr, "cannot open %s\n", statsPath.c_str());
        return 1;
    }
    stats << std::setprecision(12)
          << "field\tvalue\n"
          << "method\tACL_DEVICE_EVENT_PRIMARY_HOST_WALL_SECONDARY\n"
          << "stage\t" << stage << '\n'
          << "process_rep\t" << processRep << '\n'
          << "pid\t" << getpid() << '\n'
          << "device\t" << device << '\n'
          << "vector_cores\t" << coreCount << '\n'
          << "rows\t" << rows << '\n'
          << "width\t" << width << '\n'
          << "dtype\t" << dtype << '\n'
          << "warmups\t" << warmups << '\n'
          << "samples_per_block\t" << sampleCount << '\n'
          << "blocks\t" << blockCount << '\n'
          << "gap_seconds\t" << gapSeconds << '\n';
    stats << "block\tn\tmedian_us\tmean_us\tstdev_us\tCV\tMAD_us\tp10_us\tp90_us\tmin_us\tmax_us\tmax_min\tabs_span_us\n";
    for (int block = 0; block < blockCount; ++block) {
        WriteDistribution(stats, block + 1,
                          Summarize(blockDevice[static_cast<size_t>(block)]),
                          blockDevice[static_cast<size_t>(block)].size());
        WriteDistribution(stats, block + blockCount + 1,
                          Summarize(blockWall[static_cast<size_t>(block)]),
                          blockWall[static_cast<size_t>(block)].size());
    }
    std::vector<double> allDevice;
    for (const auto& block : blockDevice) allDevice.insert(allDevice.end(), block.begin(), block.end());
    WriteDistribution(stats, blockCount * 2 + 1, Summarize(allDevice), allDevice.size());
    stats.close();

    std::printf("CASE47_TIMING stage=%s process_rep=%d pid=%d device=%d vector_cores=%lld rows=%lld width=%lld dtype=%d warmups=%d samples=%d blocks=%d raw=%s stats=%s\n",
                stage.c_str(), processRep, getpid(),
                device, static_cast<long long>(coreCount), static_cast<long long>(rows),
                static_cast<long long>(width), dtype, warmups, sampleCount, blockCount,
                rawPath.c_str(), statsPath.c_str());

    ACL_REQUIRE(aclrtDestroyEvent(eventStart));
    ACL_REQUIRE(aclrtDestroyEvent(eventStop));
    ACL_REQUIRE(aclrtFree(deviceX));
    ACL_REQUIRE(aclrtFree(deviceResidual));
    ACL_REQUIRE(aclrtFree(deviceGamma));
    ACL_REQUIRE(aclrtFree(deviceBias));
    ACL_REQUIRE(aclrtFree(deviceOutput));
    ACL_REQUIRE(aclrtDestroyStream(stream));
    ACL_REQUIRE(aclFinalize());
    dlclose(module);
    return 0;
}
