#include "local_abi_shim.h"

#include <acl/acl.h>

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <limits>
#include <string>
#include <vector>

#include <fcntl.h>
#include <unistd.h>

#ifndef WIDE_PARENT_SOURCE_SHA256
#error WIDE_PARENT_SOURCE_SHA256 is required
#endif
#ifndef WIDE_CANDIDATE_SOURCE_SHA256
#error WIDE_CANDIDATE_SOURCE_SHA256 is required
#endif

extern "C" void run_kernel_parent(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);
extern "C" void run_kernel_candidate(
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
    GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);

namespace {

constexpr int32_t kFp32 = 0;
constexpr int32_t kFp16 = 1;
constexpr int32_t kBf16 = 2;
constexpr int64_t kRows = 2;
constexpr int64_t kAvailableCores = 40;
constexpr float kEpsilon = 1.0e-5f;
constexpr int kMinimumWarmups = 45;
constexpr int kMinimumSamples = 21;
constexpr int kMinimumSameBlocks = 2;
constexpr int kMinimumPairedBlocks = 4;
constexpr char kInputGeneratorId[] = "WIDE_X_FRESH4_NPU_CORRECTNESS_GEN1_SEED0";

struct Kernel {
    const char* name;
    const char* sourceSha256;
    void (*call)(GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
                 GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
                 GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);
};

const Kernel kParent{"parent", WIDE_PARENT_SOURCE_SHA256, run_kernel_parent};
const Kernel kCandidate{"candidate", WIDE_CANDIDATE_SOURCE_SHA256, run_kernel_candidate};

bool CheckAcl(aclError status, const char* operation)
{
    if (status == ACL_SUCCESS) return true;
    std::fprintf(stderr, "%s failed: ACL status %d\n", operation, static_cast<int>(status));
    return false;
}

const char* DTypeName(int32_t dtype)
{
    if (dtype == kFp32) return "fp32";
    if (dtype == kFp16) return "fp16";
    return "bf16";
}

size_t ElementSize(int32_t dtype)
{
    return dtype == kFp32 ? sizeof(float) : sizeof(uint16_t);
}

uint16_t FloatToBf16(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    bits += 0x7fffU + ((bits >> 16) & 1U);
    return static_cast<uint16_t>(bits >> 16);
}

uint16_t FloatToFp16(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    const uint16_t sign = static_cast<uint16_t>((bits >> 16) & 0x8000U);
    const uint32_t exponent = (bits >> 23) & 0xffU;
    uint32_t mantissa = bits & 0x7fffffU;
    if (exponent == 0xffU) {
        return static_cast<uint16_t>(sign | 0x7c00U | (mantissa == 0 ? 0 : 0x0200U));
    }
    const int32_t halfExponent = static_cast<int32_t>(exponent) - 127 + 15;
    if (halfExponent >= 31) return static_cast<uint16_t>(sign | 0x7c00U);
    if (halfExponent <= 0) {
        if (halfExponent < -10) return sign;
        mantissa |= 0x800000U;
        const uint32_t shift = static_cast<uint32_t>(14 - halfExponent);
        uint32_t result = mantissa >> shift;
        const uint32_t remainder = mantissa & ((1U << shift) - 1U);
        const uint32_t halfway = 1U << (shift - 1U);
        if (remainder > halfway || (remainder == halfway && (result & 1U))) ++result;
        return static_cast<uint16_t>(sign | result);
    }
    uint32_t result = (static_cast<uint32_t>(halfExponent) << 10) | (mantissa >> 13);
    const uint32_t remainder = mantissa & 0x1fffU;
    if (remainder > 0x1000U || (remainder == 0x1000U && (result & 1U))) ++result;
    return static_cast<uint16_t>(sign | result);
}

void Store(std::vector<uint8_t>& data, size_t index, int32_t dtype, float value)
{
    if (dtype == kFp32) {
        std::memcpy(data.data() + index * sizeof(value), &value, sizeof(value));
        return;
    }
    const uint16_t packed = dtype == kFp16 ? FloatToFp16(value) : FloatToBf16(value);
    std::memcpy(data.data() + index * sizeof(packed), &packed, sizeof(packed));
}

float Load(const std::vector<uint8_t>& data, size_t index, int32_t dtype)
{
    if (dtype == kFp32) {
        float value = 0.0f;
        std::memcpy(&value, data.data() + index * sizeof(value), sizeof(value));
        return value;
    }
    uint16_t packed = 0;
    std::memcpy(&packed, data.data() + index * sizeof(packed), sizeof(packed));
    if (dtype == kBf16) {
        const uint32_t bits = static_cast<uint32_t>(packed) << 16;
        float value = 0.0f;
        std::memcpy(&value, &bits, sizeof(value));
        return value;
    }
    const int exponent = (packed >> 10) & 0x1f;
    const int mantissa = packed & 0x03ff;
    float value = exponent == 0 ? std::ldexp(static_cast<float>(mantissa), -24) :
                  exponent == 31 ? (mantissa == 0 ? std::numeric_limits<float>::infinity() :
                                    std::numeric_limits<float>::quiet_NaN()) :
                  std::ldexp(static_cast<float>(1024 + mantissa), exponent - 25);
    return (packed & 0x8000U) != 0 ? -value : value;
}

bool CompareOutput(FILE* output, const Kernel& kernel, int device, int32_t dtype,
                   int64_t width, const std::vector<uint8_t>& x,
                   const std::vector<uint8_t>& residual, const std::vector<uint8_t>& gamma,
                   const std::vector<uint8_t>& bias, const std::vector<uint8_t>& actual)
{
    // Preserve the route's existing diagnostic tolerance and rounded CPU reference.
    const float tolerance = dtype == kFp32 ? 3.0e-5f : (dtype == kFp16 ? 2.5e-3f : 1.5e-2f);
    std::vector<uint8_t> expected(ElementSize(dtype));
    float maxError = 0.0f;
    size_t mismatches = 0, nonfinite = 0;
    for (int64_t row = 0; row < kRows; ++row) {
        double squareSum = 0.0;
        for (int64_t j = 0; j < width; ++j) {
            const size_t index = static_cast<size_t>(row * width + j);
            const float value = Load(x, index, dtype) + Load(residual, index, dtype);
            squareSum += static_cast<double>(value) * value;
        }
        const float invRms = 1.0f / std::sqrt(static_cast<float>(squareSum / width) + kEpsilon);
        for (int64_t j = 0; j < width; ++j) {
            const size_t index = static_cast<size_t>(row * width + j);
            const float value = (Load(x, index, dtype) + Load(residual, index, dtype)) * invRms *
                                Load(gamma, static_cast<size_t>(j), dtype) + Load(bias, static_cast<size_t>(j), dtype);
            Store(expected, 0, dtype, value);
            const float got = Load(actual, index, dtype), want = Load(expected, 0, dtype);
            if (!std::isfinite(got) || !std::isfinite(want)) {
                ++nonfinite;
                ++mismatches;
                continue;
            }
            const float error = std::abs(got - want);
            maxError = std::max(maxError, error);
            if (error > tolerance) ++mismatches;
        }
    }
    const bool written = std::fprintf(output, "%s\t%s\t%d\t%lld\t%lld\t%s\t%.9g\t%.9g\t%zu\t%zu\t%s\n",
        kernel.name, kernel.sourceSha256, device, static_cast<long long>(kRows),
        static_cast<long long>(width), DTypeName(dtype), maxError, tolerance,
        mismatches, nonfinite, mismatches == 0 ? "PASS" : "FAIL") >= 0;
    std::printf("CORRECTNESS_ONLY side=%s rows=%lld width=%lld dtype=%s max_abs_error=%.9g mismatches=%zu nonfinite=%zu\n",
        kernel.name, static_cast<long long>(kRows), static_cast<long long>(width),
        DTypeName(dtype), maxError, mismatches, nonfinite);
    return written && mismatches == 0;
}

bool ParseInteger(const char* text, long long minimum, long long maximum, long long& value)
{
    errno = 0;
    char* end = nullptr;
    const long long parsed = std::strtoll(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0' || parsed < minimum || parsed > maximum) return false;
    value = parsed;
    return true;
}

bool WriteSample(FILE* output, const Kernel& kernel, int block, int sample,
                 const char* order, int device, int32_t dtype, int64_t width,
                 double deviceUs, double wallUs)
{
    return std::fprintf(output, "%d\t%d\t%s\t%s\t%s\t%d\t%lld\t%d\t%.6f\t%.3f\n",
                        block, sample, order, kernel.name, kernel.sourceSha256,
                        device, static_cast<long long>(width), dtype, deviceUs, wallUs) >= 0;
}

bool LaunchAndMeasure(const Kernel& kernel, void* deviceX, void* deviceResidual,
                      void* deviceGamma, void* deviceBias, void* deviceOutput,
                      const TensorGroupInfo& inputGroup, const TensorGroupInfo& vectorGroup,
                      int device, aclrtStream stream,
                      aclrtEvent startEvent, aclrtEvent stopEvent,
                      double& deviceUs, double& wallUs)
{
    const auto wallStart = std::chrono::steady_clock::now();
    if (!CheckAcl(aclrtRecordEvent(startEvent, stream), "aclrtRecordEvent(start)")) return false;
    kernel.call(deviceX, inputGroup, deviceResidual, inputGroup,
                deviceGamma, vectorGroup, deviceBias, vectorGroup,
                deviceOutput, inputGroup, kAvailableCores, stream, kEpsilon);
    if (!CheckAcl(aclrtRecordEvent(stopEvent, stream), "aclrtRecordEvent(stop)")) return false;
    if (!CheckAcl(aclrtSynchronizeEvent(stopEvent), "aclrtSynchronizeEvent")) return false;
    const auto wallStop = std::chrono::steady_clock::now();
    float elapsedMs = 0.0f;
    if (!CheckAcl(aclrtEventElapsedTime(&elapsedMs, startEvent, stopEvent), "aclrtEventElapsedTime")) return false;
    deviceUs = static_cast<double>(elapsedMs) * 1000.0;
    wallUs = static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(wallStop - wallStart).count()) / 1000.0;
    return true;
}

bool Warmup(const Kernel& kernel, int count, void* deviceX, void* deviceResidual,
            void* deviceGamma, void* deviceBias, void* deviceOutput,
            const TensorGroupInfo& inputGroup, const TensorGroupInfo& vectorGroup,
            aclrtStream stream)
{
    for (int i = 0; i < count; ++i) {
        kernel.call(deviceX, inputGroup, deviceResidual, inputGroup,
                    deviceGamma, vectorGroup, deviceBias, vectorGroup,
                    deviceOutput, inputGroup, kAvailableCores, stream, kEpsilon);
        if (!CheckAcl(aclrtSynchronizeStream(stream), "aclrtSynchronizeStream(warmup)")) return false;
    }
    return true;
}

void PrintUsage(const char* program)
{
    std::printf("Usage: %s DEVICE WIDTH DTYPE MODE SIDE WARMUPS SAMPLES BLOCKS OUTPUT.tsv\n", program);
    std::printf("DTYPE: fp32 | fp16 | bf16; MODE: correctness-only | same | paired; SIDE: parent | candidate | -\n");
    std::printf("correctness-only requires SIDE parent/candidate and WARMUPS=0 SAMPLES=0 BLOCKS=1; no events or timing.\n");
    std::printf("same requires >=45 warmups, >=21 samples, >=2 blocks; paired requires >=4 blocks.\n");
}

}  // namespace

int main(int argc, char** argv)
{
    if (argc == 2 && std::strcmp(argv[1], "--help") == 0) {
        PrintUsage(argv[0]);
        return 0;
    }
    if (argc != 10) {
        PrintUsage(argv[0]);
        return 2;
    }

    const bool correctnessOnly = std::strcmp(argv[4], "correctness-only") == 0;
    long long deviceArg = 0, widthArg = 0, warmupsArg = 0, samplesArg = 0, blocksArg = 0;
    if (!ParseInteger(argv[1], 0, 7, deviceArg) ||
        !ParseInteger(argv[2], 1, 32768, widthArg) ||
        !ParseInteger(argv[6], correctnessOnly ? 0 : kMinimumWarmups, 100000, warmupsArg) ||
        !ParseInteger(argv[7], correctnessOnly ? 0 : kMinimumSamples, 100000, samplesArg) ||
        !ParseInteger(argv[8], 1, 100000, blocksArg)) {
        std::fprintf(stderr, "invalid numeric argument or below protocol minimum\n");
        return 2;
    }

    const int32_t dtype = std::strcmp(argv[3], "fp32") == 0 ? kFp32 :
                          std::strcmp(argv[3], "fp16") == 0 ? kFp16 :
                          std::strcmp(argv[3], "bf16") == 0 ? kBf16 : -1;
    const bool paired = std::strcmp(argv[4], "paired") == 0;
    const bool same = std::strcmp(argv[4], "same") == 0;
    const Kernel* sameKernel = std::strcmp(argv[5], "parent") == 0 ? &kParent :
                               std::strcmp(argv[5], "candidate") == 0 ? &kCandidate : nullptr;
    if ((dtype < 0) || (!paired && !same && !correctnessOnly) ||
        (widthArg != 2048 && widthArg != 16384 && widthArg != 32768) ||
        (paired && std::strcmp(argv[5], "-") != 0) || ((same || correctnessOnly) && sameKernel == nullptr) ||
        (correctnessOnly && (warmupsArg != 0 || samplesArg != 0 || blocksArg != 1)) ||
        (same && blocksArg < kMinimumSameBlocks) || (paired && blocksArg < kMinimumPairedBlocks)) {
        std::fprintf(stderr, "unsupported dtype, mode, side, width, or block count\n");
        return 2;
    }

    const int device = static_cast<int>(deviceArg);
    const int64_t width = static_cast<int64_t>(widthArg);
    const int warmups = static_cast<int>(warmupsArg);
    const int samples = static_cast<int>(samplesArg);
    const int blocks = static_cast<int>(blocksArg);
    const size_t elementSize = ElementSize(dtype);
    const size_t inputElements = static_cast<size_t>(kRows * width);
    const size_t vectorElements = static_cast<size_t>(width);
    const size_t inputBytes = inputElements * elementSize;
    const size_t vectorBytes = vectorElements * elementSize;
    std::vector<uint8_t> x(inputBytes), residual(inputBytes), gamma(vectorBytes), bias(vectorBytes);
    for (size_t i = 0; i < inputElements; ++i) {
        Store(x, i, dtype, static_cast<float>(static_cast<int>((i * 17) % 101) - 50) * 0.0015f);
        Store(residual, i, dtype, static_cast<float>(static_cast<int>((i * 13) % 71) - 35) * 0.0011f);
    }
    for (size_t i = 0; i < vectorElements; ++i) {
        Store(gamma, i, dtype, 0.8f + static_cast<float>(i % 19) * 0.003f);
        Store(bias, i, dtype, static_cast<float>(static_cast<int>(i % 23) - 11) * 0.001f);
    }

    const int outputFd = open(argv[9], O_WRONLY | O_CREAT | O_EXCL, 0644);
    if (outputFd < 0) {
        std::fprintf(stderr, "cannot create new output file %s: %s\n", argv[9], std::strerror(errno));
        return 1;
    }
    FILE* output = fdopen(outputFd, "w");
    if (output == nullptr) {
        std::fprintf(stderr, "fdopen failed: %s\n", std::strerror(errno));
        close(outputFd);
        return 1;
    }

    const bool runtimeInitialized = CheckAcl(aclInit(nullptr), "aclInit");
    const bool deviceSelected = runtimeInitialized && CheckAcl(aclrtSetDevice(device), "aclrtSetDevice");
    bool passed = deviceSelected;
    aclrtStream stream = nullptr;
    aclrtEvent startEvent = nullptr, stopEvent = nullptr;
    void *deviceX = nullptr, *deviceResidual = nullptr, *deviceGamma = nullptr;
    void *deviceBias = nullptr, *deviceOutput = nullptr;
    int exitCode = passed ? 0 : 1;
    const auto cleanup = [&]() {
        if (stream != nullptr) {
            const aclError drainStatus = aclrtSynchronizeStream(stream);
            if (drainStatus != ACL_SUCCESS) {
                std::fprintf(stderr,
                             "cleanup failed: aclrtSynchronizeStream returned %d; "
                             "skipping event, buffer, stream, device reset, and runtime release\n",
                             static_cast<int>(drainStatus));
                if (exitCode == 0) exitCode = 1;
                return;
            }
        }

        const auto release = [&exitCode](aclError status, const char* operation) {
            if (status == ACL_SUCCESS) return;
            std::fprintf(stderr, "cleanup failed: %s returned %d\n",
                         operation, static_cast<int>(status));
            if (exitCode == 0) exitCode = 1;
        };
        if (startEvent != nullptr) release(aclrtDestroyEvent(startEvent), "aclrtDestroyEvent(start)");
        if (stopEvent != nullptr) release(aclrtDestroyEvent(stopEvent), "aclrtDestroyEvent(stop)");
        if (deviceX != nullptr) release(aclrtFree(deviceX), "aclrtFree(x)");
        if (deviceResidual != nullptr) release(aclrtFree(deviceResidual), "aclrtFree(residual)");
        if (deviceGamma != nullptr) release(aclrtFree(deviceGamma), "aclrtFree(gamma)");
        if (deviceBias != nullptr) release(aclrtFree(deviceBias), "aclrtFree(bias)");
        if (deviceOutput != nullptr) release(aclrtFree(deviceOutput), "aclrtFree(output)");
        if (stream != nullptr) release(aclrtDestroyStream(stream), "aclrtDestroyStream");
        if (deviceSelected) release(aclrtResetDevice(device), "aclrtResetDevice");
        if (runtimeInitialized) release(aclFinalize(), "aclFinalize");
    };

    if (passed) passed = CheckAcl(aclrtCreateStream(&stream), "aclrtCreateStream");
    if (passed && !correctnessOnly) passed = CheckAcl(aclrtCreateEvent(&startEvent), "aclrtCreateEvent(start)");
    if (passed && !correctnessOnly) passed = CheckAcl(aclrtCreateEvent(&stopEvent), "aclrtCreateEvent(stop)");
    if (passed) passed = CheckAcl(aclrtMalloc(&deviceX, inputBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(x)");
    if (passed) passed = CheckAcl(aclrtMalloc(&deviceResidual, inputBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(residual)");
    if (passed) passed = CheckAcl(aclrtMalloc(&deviceGamma, vectorBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(gamma)");
    if (passed) passed = CheckAcl(aclrtMalloc(&deviceBias, vectorBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(bias)");
    if (passed) passed = CheckAcl(aclrtMalloc(&deviceOutput, inputBytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(output)");
    if (passed) passed = CheckAcl(aclrtMemcpy(deviceX, inputBytes, x.data(), inputBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy x");
    if (passed) passed = CheckAcl(aclrtMemcpy(deviceResidual, inputBytes, residual.data(), inputBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy residual");
    if (passed) passed = CheckAcl(aclrtMemcpy(deviceGamma, vectorBytes, gamma.data(), vectorBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma");
    if (passed) passed = CheckAcl(aclrtMemcpy(deviceBias, vectorBytes, bias.data(), vectorBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy bias");

    const int64_t inputShape[] = {kRows, width};
    const int64_t vectorShape[] = {width};
    const TensorInfo inputInfo{inputShape, 2, dtype};
    const TensorInfo vectorInfo{vectorShape, 1, dtype};
    const TensorGroupInfo inputGroup{&inputInfo, 1};
    const TensorGroupInfo vectorGroup{&vectorInfo, 1};

    if (passed) {
        std::fprintf(output, "# ROUTE=WIDE-X-FRESH4\n");
        std::fprintf(output, "# PARENT_SOURCE_SHA256=%s\n", kParent.sourceSha256);
        std::fprintf(output, "# CANDIDATE_SOURCE_SHA256=%s\n", kCandidate.sourceSha256);
        std::fprintf(output, "# INPUT_ID=%s:dtype=%s:rows=%lld:width=%lld\n",
                     kInputGeneratorId, DTypeName(dtype), static_cast<long long>(kRows),
                     static_cast<long long>(width));
        std::fprintf(output, "# DEVICE=%d MODE=%s WARMUPS=%d SAMPLES_PER_BLOCK=%d BLOCKS=%d\n",
                     device, argv[4], warmups, samples, blocks);
        std::fprintf(output, "# AVAILABLE_CORES=%lld EPSILON=%.9g\n", static_cast<long long>(kAvailableCores), kEpsilon);
        if (correctnessOnly) {
            std::fprintf(output, "# VALIDATION=ROUTE_DIAGNOSTIC_ABS_TOLERANCE; rtol=0; no Official result\n");
            std::fprintf(output, "side\tsource_sha256\tdevice\trows\twidth\tdtype\tmax_abs_error\tatol\tmismatches\tnonfinite\tstatus\n");
        } else {
            std::fprintf(output, "block\tsample\torder\tside\tsource_sha256\tdevice\twidth\tdtype_id\tdevice_event_us\twall_us\n");
        }
        passed = std::fflush(output) == 0;
    }

    if (passed && correctnessOnly) {
        std::vector<uint8_t> actual(inputBytes);
        passed = CheckAcl(aclrtMemset(deviceOutput, inputBytes, 0xff, inputBytes), "initialize output to NaN");
        if (passed) {
            sameKernel->call(deviceX, inputGroup, deviceResidual, inputGroup,
                             deviceGamma, vectorGroup, deviceBias, vectorGroup,
                             deviceOutput, inputGroup, kAvailableCores, stream, kEpsilon);
            passed = CheckAcl(aclrtSynchronizeStream(stream), "aclrtSynchronizeStream(correctness)") &&
                     CheckAcl(aclrtMemcpy(actual.data(), inputBytes, deviceOutput, inputBytes,
                                         ACL_MEMCPY_DEVICE_TO_HOST), "copy correctness output");
        }
        if (passed) passed = CompareOutput(output, *sameKernel, device, dtype, width, x, residual, gamma, bias, actual);
    }

    if (passed && !correctnessOnly) {
        if (paired) {
            passed = Warmup(kParent, warmups, deviceX, deviceResidual, deviceGamma,
                            deviceBias, deviceOutput, inputGroup, vectorGroup, stream) &&
                     Warmup(kCandidate, warmups, deviceX, deviceResidual, deviceGamma,
                            deviceBias, deviceOutput, inputGroup, vectorGroup, stream);
        } else {
            passed = Warmup(*sameKernel, warmups, deviceX, deviceResidual, deviceGamma,
                            deviceBias, deviceOutput, inputGroup, vectorGroup, stream);
        }
    }

    for (int block = 0; passed && !correctnessOnly && block < blocks; ++block) {
        for (int sample = 0; passed && sample < samples; ++sample) {
            if (paired) {
                const bool candidateFirst = ((block + sample) % 2) != 0;
                const Kernel& first = candidateFirst ? kCandidate : kParent;
                const Kernel& second = candidateFirst ? kParent : kCandidate;
                const char* firstOrder = candidateFirst ? "CP" : "PC";
                double deviceUs = 0.0, wallUs = 0.0;
                passed = LaunchAndMeasure(first, deviceX, deviceResidual, deviceGamma, deviceBias,
                                          deviceOutput, inputGroup, vectorGroup, device,
                                          stream, startEvent, stopEvent, deviceUs, wallUs) &&
                         WriteSample(output, first, block + 1, sample + 1, firstOrder, device,
                                     dtype, width, deviceUs, wallUs);
                if (passed) {
                    passed = LaunchAndMeasure(second, deviceX, deviceResidual, deviceGamma, deviceBias,
                                              deviceOutput, inputGroup, vectorGroup, device,
                                              stream, startEvent, stopEvent, deviceUs, wallUs) &&
                             WriteSample(output, second, block + 1, sample + 1, firstOrder, device,
                                         dtype, width, deviceUs, wallUs);
                }
            } else {
                double deviceUs = 0.0, wallUs = 0.0;
                passed = LaunchAndMeasure(*sameKernel, deviceX, deviceResidual, deviceGamma, deviceBias,
                                          deviceOutput, inputGroup, vectorGroup, device,
                                          stream, startEvent, stopEvent, deviceUs, wallUs) &&
                         WriteSample(output, *sameKernel, block + 1, sample + 1, "SAME", device,
                                     dtype, width, deviceUs, wallUs);
            }
        }
        if (passed) passed = std::fflush(output) == 0;
    }

    if (!passed && exitCode == 0) exitCode = 1;
    cleanup();
    if (std::fclose(output) != 0) passed = false;
    if (!passed && exitCode == 0) exitCode = 1;
    return exitCode;
}
