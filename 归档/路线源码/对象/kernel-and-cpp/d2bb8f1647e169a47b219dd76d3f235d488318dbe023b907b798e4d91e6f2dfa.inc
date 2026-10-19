#include <algorithm>
#include <cerrno>
#include <chrono>
#include <climits>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <vector>

namespace {
constexpr uint64_t kRows = 2;
constexpr int64_t kBlocks = 1;
constexpr int32_t kWarmups = 10;
constexpr int32_t kSamples = 21;
constexpr float kEpsilon = 1.0e-5f;
constexpr double kTolerance = 1.0e-4;

struct TimedSample {
    int32_t pair;
    const char* order;
    const char* version;
    double deviceUs;
    double wallUs;
};

struct Stats {
    double median;
    double mean;
    double stdev;
    double cv;
    double min;
    double max;
    double maxMin;
    double mad;
    double p10;
    double p90;
    double spread;
};

bool CheckAcl(aclError status, const char* operation)
{
    if (status == ACL_SUCCESS) return true;
    fprintf(stderr, "ACL failure: %s status=%d\n", operation, static_cast<int>(status));
    return false;
}

float InputX(size_t index)
{
    return static_cast<float>(static_cast<int>(index % 101) - 50) * 0.01f;
}

float InputResidual(size_t index)
{
    return static_cast<float>(static_cast<int>(index % 37) - 18) * 0.006f;
}

float InputGamma(size_t index)
{
    return 0.8f + static_cast<float>(index % 11) * 0.01f;
}

float InputBias(size_t index)
{
    return static_cast<float>(static_cast<int>(index % 17) - 8) * 0.001f;
}

double Percentile(const std::vector<double>& sorted, double p)
{
    const double index = p * static_cast<double>(sorted.size() - 1);
    const size_t lo = static_cast<size_t>(index);
    const size_t hi = std::min(lo + 1, sorted.size() - 1);
    const double fraction = index - static_cast<double>(lo);
    return sorted[lo] + (sorted[hi] - sorted[lo]) * fraction;
}

Stats Summarize(const std::vector<double>& values)
{
    std::vector<double> sorted(values);
    std::sort(sorted.begin(), sorted.end());
    double mean = 0.0;
    for (double value : values) mean += value;
    mean /= static_cast<double>(values.size());

    double sumSquares = 0.0;
    for (double value : values) {
        const double delta = value - mean;
        sumSquares += delta * delta;
    }
    const double stdev = values.size() > 1
        ? std::sqrt(sumSquares / static_cast<double>(values.size() - 1))
        : 0.0;
    std::vector<double> deviations;
    deviations.reserve(values.size());
    for (double value : values) deviations.push_back(std::fabs(value - Percentile(sorted, 0.5)));
    std::sort(deviations.begin(), deviations.end());

    const double minimum = sorted.front();
    const double maximum = sorted.back();
    return {
        Percentile(sorted, 0.5), mean, stdev,
        mean == 0.0 ? 0.0 : stdev / mean,
        minimum, maximum,
        minimum == 0.0 ? 0.0 : maximum / minimum,
        Percentile(deviations, 0.5), Percentile(sorted, 0.1),
        Percentile(sorted, 0.9), maximum - minimum
    };
}

template <typename Invoke>
bool InvokeAndSync(Invoke invoke, aclrtStream stream)
{
    invoke();
    return CheckAcl(aclrtSynchronizeStream(stream), "aclrtSynchronizeStream");
}

bool CheckCorrectness(const char* version, const std::vector<float>& output,
                      const std::vector<float>& x, const std::vector<float>& residual,
                      const std::vector<float>& gamma, const std::vector<float>& bias,
                      uint64_t width)
{
    double maxAbs = 0.0;
    for (uint64_t row = 0; row < kRows; ++row) {
        const size_t rowOffset = static_cast<size_t>(row * width);
        double squareSum = 0.0;
        for (uint64_t col = 0; col < width; ++col) {
            const size_t index = rowOffset + static_cast<size_t>(col);
            const double value = static_cast<double>(x[index]) + residual[index];
            squareSum += value * value;
        }
        const double invRms = 1.0 / std::sqrt(squareSum / static_cast<double>(width) + kEpsilon);
        for (uint64_t col = 0; col < width; ++col) {
            const size_t index = rowOffset + static_cast<size_t>(col);
            const double value = static_cast<double>(x[index]) + residual[index];
            const double expected = value * invRms * gamma[static_cast<size_t>(col)] +
                bias[static_cast<size_t>(col)];
            if (!std::isfinite(output[index])) {
                fprintf(stderr, "%s correctness failed: non-finite output at %zu\n", version, index);
                return false;
            }
            maxAbs = std::max(maxAbs, std::fabs(expected - output[index]));
        }
    }
    if (!(maxAbs < kTolerance)) {
        fprintf(stderr, "%s correctness failed: max_abs=%.9g\n", version, maxAbs);
        return false;
    }
    printf("%s correctness PASS width=%llu max_abs=%.9g\n", version,
           static_cast<unsigned long long>(width), maxAbs);
    return true;
}

template <typename Invoke>
bool MeasureOne(Invoke invoke, const char* version, int32_t pair,
                const char* order, aclrtStream stream, aclrtEvent start,
                aclrtEvent stop, std::vector<TimedSample>& samples)
{
    if (!CheckAcl(aclrtRecordEvent(start, stream), "aclrtRecordEvent(start)")) return false;
    const auto wallStart = std::chrono::steady_clock::now();
    invoke();
    if (!CheckAcl(aclrtRecordEvent(stop, stream), "aclrtRecordEvent(stop)")) return false;
    if (!CheckAcl(aclrtSynchronizeEvent(stop), "aclrtSynchronizeEvent(stop)")) return false;
    const auto wallStop = std::chrono::steady_clock::now();
    float deviceMs = 0.0f;
    if (!CheckAcl(aclrtEventElapsedTime(&deviceMs, start, stop), "aclrtEventElapsedTime")) return false;
    samples.push_back({
        pair, order, version, static_cast<double>(deviceMs) * 1000.0,
        std::chrono::duration<double, std::micro>(wallStop - wallStart).count()
    });
    return true;
}

bool WriteOutputs(const char* prefix, uint64_t width,
                  const std::vector<TimedSample>& samples)
{
    const std::string samplePath = std::string(prefix) + "-samples.tsv";
    const std::string statsPath = std::string(prefix) + "-jitter.tsv";
    FILE* raw = fopen(samplePath.c_str(), "wx");
    if (raw == nullptr) {
        fprintf(stderr, "cannot create %s: %s\n", samplePath.c_str(), strerror(errno));
        return false;
    }
    FILE* summary = fopen(statsPath.c_str(), "wx");
    if (summary == nullptr) {
        fprintf(stderr, "cannot create %s: %s\n", statsPath.c_str(), strerror(errno));
        fclose(raw);
        return false;
    }

    fprintf(raw, "width\trows\tblocks\tpair\torder\tversion\tdevice_us\twall_us\n");
    for (const TimedSample& sample : samples) {
        fprintf(raw, "%llu\t%llu\t%lld\t%d\t%s\t%s\t%.6f\t%.6f\n",
                static_cast<unsigned long long>(width),
                static_cast<unsigned long long>(kRows), static_cast<long long>(kBlocks),
                sample.pair, sample.order, sample.version, sample.deviceUs, sample.wallUs);
    }

    fprintf(summary, "version\tmetric\tn\tmedian\tmean\tstdev\tCV\tmin\tmax\tmax_min\tMAD\tp10\tp90\tabsolute_spread\n");
    for (const char* version : {"V016", "V021"}) {
        for (const char* metric : {"device_us", "wall_us"}) {
            std::vector<double> values;
            for (const TimedSample& sample : samples) {
                if (std::strcmp(sample.version, version) == 0) {
                    values.push_back(std::strcmp(metric, "device_us") == 0
                        ? sample.deviceUs : sample.wallUs);
                }
            }
            const Stats stats = Summarize(values);
            fprintf(summary, "%s\t%s\t%zu\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\t%.6f\n",
                    version, metric, values.size(), stats.median, stats.mean, stats.stdev,
                    stats.cv, stats.min, stats.max, stats.maxMin, stats.mad,
                    stats.p10, stats.p90, stats.spread);
        }
    }

    const bool writeOk = ferror(raw) == 0 && ferror(summary) == 0;
    const int rawClose = fclose(raw);
    const int summaryClose = fclose(summary);
    return writeOk && rawClose == 0 && summaryClose == 0;
}
}  // namespace

int main(int argc, char** argv)
{
    if (argc != 4) {
        fprintf(stderr, "usage: %s DEVICE WIDTH OUTPUT_PREFIX\n", argv[0]);
        return 2;
    }
    char* deviceEnd = nullptr;
    char* widthEnd = nullptr;
    const long deviceValue = std::strtol(argv[1], &deviceEnd, 10);
    const long widthValue = std::strtol(argv[2], &widthEnd, 10);
    if (deviceEnd == argv[1] || *deviceEnd != '\0' || deviceValue < 0 || deviceValue > INT_MAX ||
        widthEnd == argv[2] || *widthEnd != '\0' ||
        (widthValue != 32768 && widthValue != 24576) || argv[3][0] == '\0') {
        fprintf(stderr, "invalid arguments; WIDTH must be 32768 or 24576\n");
        return 2;
    }
    const int32_t device = static_cast<int32_t>(deviceValue);
    const uint64_t width = static_cast<uint64_t>(widthValue);
    const size_t elementCount = static_cast<size_t>(kRows * width);
    const size_t dataBytes = elementCount * sizeof(float);
    const size_t parameterBytes = static_cast<size_t>(width) * sizeof(float);

    std::vector<float> x(elementCount);
    std::vector<float> residual(elementCount);
    std::vector<float> gamma(static_cast<size_t>(width));
    std::vector<float> bias(static_cast<size_t>(width));
    std::vector<float> output(elementCount);
    for (size_t i = 0; i < elementCount; ++i) {
        x[i] = InputX(i);
        residual[i] = InputResidual(i);
    }
    for (size_t i = 0; i < static_cast<size_t>(width); ++i) {
        gamma[i] = InputGamma(i);
        bias[i] = InputBias(i);
    }

    float* xDevice = nullptr;
    float* residualDevice = nullptr;
    float* gammaDevice = nullptr;
    float* biasDevice = nullptr;
    float* outputDevice = nullptr;
    aclrtStream stream = nullptr;
    aclrtEvent start = nullptr;
    aclrtEvent stop = nullptr;
    bool aclInitialized = false;
    bool deviceSelected = false;
    int result = 0;

    const int64_t inputShape[] = {static_cast<int64_t>(kRows), static_cast<int64_t>(width)};
    const int64_t parameterShape[] = {static_cast<int64_t>(width)};
    TensorInfo xInfo[] = {{inputShape, 2, 0}};
    TensorInfo residualInfo[] = {{inputShape, 2, 0}};
    TensorInfo gammaInfo[] = {{parameterShape, 1, 0}};
    TensorInfo biasInfo[] = {{parameterShape, 1, 0}};
    TensorInfo outputInfo[] = {{inputShape, 2, 0}};
    const TensorGroupInfo infoX = {xInfo, 1};
    const TensorGroupInfo infoResidual = {residualInfo, 1};
    const TensorGroupInfo infoGamma = {gammaInfo, 1};
    const TensorGroupInfo infoBias = {biasInfo, 1};
    const TensorGroupInfo infoOutput = {outputInfo, 1};
    auto invokeV016 = [&]() {
        r31a_v016::r31a_run_kernel_v016(
            reinterpret_cast<GM_ADDR>(xDevice), infoX,
            reinterpret_cast<GM_ADDR>(residualDevice), infoResidual,
            reinterpret_cast<GM_ADDR>(gammaDevice), infoGamma,
            reinterpret_cast<GM_ADDR>(biasDevice), infoBias,
            reinterpret_cast<GM_ADDR>(outputDevice), infoOutput,
            kBlocks, stream, kEpsilon);
    };
    auto invokeV021 = [&]() {
        r31a_v021::r31a_run_kernel_v021(
            reinterpret_cast<GM_ADDR>(xDevice), infoX,
            reinterpret_cast<GM_ADDR>(residualDevice), infoResidual,
            reinterpret_cast<GM_ADDR>(gammaDevice), infoGamma,
            reinterpret_cast<GM_ADDR>(biasDevice), infoBias,
            reinterpret_cast<GM_ADDR>(outputDevice), infoOutput,
            kBlocks, stream, kEpsilon);
    };

    std::vector<TimedSample> samples;
    samples.reserve(static_cast<size_t>(2 * kSamples));

    do {
        if (!CheckAcl(aclInit(nullptr), "aclInit")) { result = 2; break; }
        aclInitialized = true;
        if (!CheckAcl(aclrtSetDevice(device), "aclrtSetDevice")) { result = 2; break; }
        deviceSelected = true;
        if (!CheckAcl(aclrtCreateStream(&stream), "aclrtCreateStream")) { result = 2; break; }
        if (!CheckAcl(aclrtMalloc(reinterpret_cast<void**>(&xDevice), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(x)")) { result = 2; break; }
        if (!CheckAcl(aclrtMalloc(reinterpret_cast<void**>(&residualDevice), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(residual)")) { result = 2; break; }
        if (!CheckAcl(aclrtMalloc(reinterpret_cast<void**>(&gammaDevice), parameterBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(gamma)")) { result = 2; break; }
        if (!CheckAcl(aclrtMalloc(reinterpret_cast<void**>(&biasDevice), parameterBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(bias)")) { result = 2; break; }
        if (!CheckAcl(aclrtMalloc(reinterpret_cast<void**>(&outputDevice), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(output)")) { result = 2; break; }
        if (!CheckAcl(aclrtMemcpy(xDevice, dataBytes, x.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "aclrtMemcpy(x H2D)")) { result = 2; break; }
        if (!CheckAcl(aclrtMemcpy(residualDevice, dataBytes, residual.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "aclrtMemcpy(residual H2D)")) { result = 2; break; }
        if (!CheckAcl(aclrtMemcpy(gammaDevice, parameterBytes, gamma.data(), parameterBytes, ACL_MEMCPY_HOST_TO_DEVICE), "aclrtMemcpy(gamma H2D)")) { result = 2; break; }
        if (!CheckAcl(aclrtMemcpy(biasDevice, parameterBytes, bias.data(), parameterBytes, ACL_MEMCPY_HOST_TO_DEVICE), "aclrtMemcpy(bias H2D)")) { result = 2; break; }

        if (!InvokeAndSync(invokeV016, stream) ||
            !CheckAcl(aclrtMemcpy(output.data(), dataBytes, outputDevice, dataBytes, ACL_MEMCPY_DEVICE_TO_HOST), "aclrtMemcpy(V016 D2H)") ||
            !CheckCorrectness("V016", output, x, residual, gamma, bias, width)) {
            result = 3;
            break;
        }
        if (!InvokeAndSync(invokeV021, stream) ||
            !CheckAcl(aclrtMemcpy(output.data(), dataBytes, outputDevice, dataBytes, ACL_MEMCPY_DEVICE_TO_HOST), "aclrtMemcpy(V021 D2H)") ||
            !CheckCorrectness("V021", output, x, residual, gamma, bias, width)) {
            result = 3;
            break;
        }

        for (int32_t i = 0; i < kWarmups; ++i) {
            const bool parentFirst = (i % 2) == 0;
            if (!(parentFirst
                    ? InvokeAndSync(invokeV016, stream) && InvokeAndSync(invokeV021, stream)
                    : InvokeAndSync(invokeV021, stream) && InvokeAndSync(invokeV016, stream))) {
                result = 4;
                break;
            }
        }
        if (result != 0) break;

        if (!CheckAcl(aclrtCreateEvent(&start), "aclrtCreateEvent(start)") ||
            !CheckAcl(aclrtCreateEvent(&stop), "aclrtCreateEvent(stop)")) {
            result = 4;
            break;
        }
        for (int32_t pair = 0; pair < kSamples; ++pair) {
            const bool parentFirst = (pair % 2) == 0;
            const char* order = parentFirst ? "P-C" : "C-P";
            const bool ok = parentFirst
                ? MeasureOne(invokeV016, "V016", pair + 1, order, stream, start, stop, samples) &&
                  MeasureOne(invokeV021, "V021", pair + 1, order, stream, start, stop, samples)
                : MeasureOne(invokeV021, "V021", pair + 1, order, stream, start, stop, samples) &&
                  MeasureOne(invokeV016, "V016", pair + 1, order, stream, start, stop, samples);
            if (!ok) { result = 4; break; }
        }
        if (result == 0 && !WriteOutputs(argv[3], width, samples)) result = 5;
    } while (false);

    if (start != nullptr) aclrtDestroyEvent(start);
    if (stop != nullptr) aclrtDestroyEvent(stop);
    if (outputDevice != nullptr) aclrtFree(outputDevice);
    if (biasDevice != nullptr) aclrtFree(biasDevice);
    if (gammaDevice != nullptr) aclrtFree(gammaDevice);
    if (residualDevice != nullptr) aclrtFree(residualDevice);
    if (xDevice != nullptr) aclrtFree(xDevice);
    if (stream != nullptr) aclrtDestroyStream(stream);
    if (deviceSelected) aclrtResetDevice(device);
    if (aclInitialized) aclFinalize();
    if (result == 0) {
        printf("paired run complete: width=%llu samples_per_version=%d warmups_per_version=%d output_prefix=%s\n",
               static_cast<unsigned long long>(width), kSamples, kWarmups, argv[3]);
    }
    return result;
}
