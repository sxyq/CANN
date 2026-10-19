#include "correctness_runner_abi.h"

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <type_traits>
#include <vector>

namespace {

using RunKernel = decltype(&addr_v001_parent::addr_v001_parent_run_kernel);
static_assert(std::is_same<RunKernel,
                           decltype(&addr_v001_candidate::addr_v001_candidate_run_kernel)>::value,
              "parent/candidate runner ABI mismatch");

constexpr int kCoreCount = 8;
constexpr float kEpsilon = 1.0e-5f;

struct CaseSpec {
    const char* name;
    int dtype;
    int width;
    int rows;
    int expectedTileWidth;
    int expectedTail;
};

struct DeviceBuffers {
    void* x = nullptr;
    void* residual = nullptr;
    void* gamma = nullptr;
    void* bias = nullptr;
    void* parentOutput = nullptr;
    void* candidateOutput = nullptr;
};

void CheckAcl(aclError status, const char* operation)
{
    if (status != ACL_SUCCESS) {
        std::fprintf(stderr, "%s failed: %d %s\n", operation, status,
                     aclGetRecentErrMsg() ? aclGetRecentErrMsg() : "");
        std::exit(2);
    }
}

uint16_t FloatToHalf(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    const uint32_t sign = (bits >> 16) & 0x8000u;
    const uint32_t exponent = (bits >> 23) & 0xffu;
    uint32_t mantissa = bits & 0x7fffffu;
    if (exponent == 0xffu) {
        return static_cast<uint16_t>(sign | 0x7c00u | (mantissa ? 0x0200u : 0u));
    }
    int32_t halfExponent = static_cast<int32_t>(exponent) - 112;
    if (halfExponent >= 31) {
        return static_cast<uint16_t>(sign | 0x7c00u);
    }
    if (halfExponent <= 0) {
        if (halfExponent < -10) {
            return static_cast<uint16_t>(sign);
        }
        mantissa |= 0x800000u;
        const int32_t shift = 14 - halfExponent;
        const uint32_t round = (1u << (shift - 1)) - 1u + ((mantissa >> shift) & 1u);
        return static_cast<uint16_t>(sign | ((mantissa + round) >> shift));
    }
    mantissa += 0xfffu + ((mantissa >> 13) & 1u);
    if (mantissa & 0x800000u) {
        mantissa = 0;
        ++halfExponent;
        if (halfExponent >= 31) {
            return static_cast<uint16_t>(sign | 0x7c00u);
        }
    }
    return static_cast<uint16_t>(sign | (static_cast<uint32_t>(halfExponent) << 10) |
                                 (mantissa >> 13));
}

float HalfToFloat(uint16_t value)
{
    const float sign = (value & 0x8000u) ? -1.0f : 1.0f;
    const uint32_t exponent = (value >> 10) & 0x1fu;
    const uint32_t mantissa = value & 0x03ffu;
    if (exponent == 0) {
        return sign * std::ldexp(static_cast<float>(mantissa), -24);
    }
    if (exponent == 0x1fu) {
        return mantissa ? NAN : sign * INFINITY;
    }
    return sign * std::ldexp(1.0f + static_cast<float>(mantissa) / 1024.0f,
                             static_cast<int>(exponent) - 15);
}

uint16_t FloatToBfloat16(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    bits += 0x7fffu + ((bits >> 16) & 1u);
    return static_cast<uint16_t>(bits >> 16);
}

float Bfloat16ToFloat(uint16_t value)
{
    const uint32_t bits = static_cast<uint32_t>(value) << 16;
    float result = 0.0f;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

uint16_t Encode(float value, int dtype)
{
    return dtype == 1 ? FloatToHalf(value) : FloatToBfloat16(value);
}

float Decode(uint16_t value, int dtype)
{
    return dtype == 1 ? HalfToFloat(value) : Bfloat16ToFloat(value);
}

float Rounded(float value, int dtype)
{
    return Decode(Encode(value, dtype), dtype);
}

void Allocate(void** pointer, size_t bytes, const char* label)
{
    CheckAcl(aclrtMalloc(pointer, bytes, ACL_MEM_MALLOC_HUGE_FIRST), label);
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

void Launch(RunKernel kernel, void* output, const DeviceBuffers& buffers,
            const TensorGroupInfo& dataGroup, const TensorGroupInfo& paramGroup,
            aclrtStream stream)
{
    kernel(static_cast<GM_ADDR>(buffers.x), dataGroup,
           static_cast<GM_ADDR>(buffers.residual), dataGroup,
           static_cast<GM_ADDR>(buffers.gamma), paramGroup,
           static_cast<GM_ADDR>(buffers.bias), paramGroup,
           static_cast<GM_ADDR>(output), dataGroup, kCoreCount, stream, kEpsilon);
    CheckAcl(aclrtSynchronizeStream(stream), "kernel synchronize");
}

bool CheckOutput(const CaseSpec& spec, const std::vector<uint16_t>& x,
                 const std::vector<uint16_t>& residual,
                 const std::vector<uint16_t>& gamma,
                 const std::vector<uint16_t>& bias,
                 const std::vector<uint16_t>& output, const char* binary)
{
    float maxAbs = 0.0f;
    float maxRel = 0.0f;
    size_t failures = 0;
    const float tolerance = spec.dtype == 1 ? 0.004f : 0.02f;
    for (int row = 0; row < spec.rows; ++row) {
        float squareSum = 0.0f;
        std::vector<float> y(static_cast<size_t>(spec.width));
        for (int col = 0; col < spec.width; ++col) {
            const size_t index = static_cast<size_t>(row) * spec.width + col;
            float value = Decode(x[index], spec.dtype) + Decode(residual[index], spec.dtype);
            if (spec.dtype == 1) {
                value = Rounded(value, spec.dtype);
            }
            y[static_cast<size_t>(col)] = value;
            squareSum += value * value;
        }
        const float invRms = 1.0f / std::sqrt(squareSum / static_cast<float>(spec.width) +
                                               kEpsilon);
        for (int col = 0; col < spec.width; ++col) {
            const size_t index = static_cast<size_t>(row) * spec.width + col;
            float expected = y[static_cast<size_t>(col)] * invRms;
            if (spec.dtype == 1) {
                expected = Rounded(expected, spec.dtype);
                expected = Rounded(expected * Decode(gamma[static_cast<size_t>(col)], spec.dtype),
                                   spec.dtype);
                expected = Rounded(expected + Decode(bias[static_cast<size_t>(col)], spec.dtype),
                                   spec.dtype);
            } else {
                expected = Rounded(expected * Decode(gamma[static_cast<size_t>(col)], spec.dtype) +
                                   Decode(bias[static_cast<size_t>(col)], spec.dtype), spec.dtype);
            }
            const float actual = Decode(output[index], spec.dtype);
            const float absError = std::fabs(actual - expected);
            const float relError = absError / std::max(std::fabs(expected), 1.0e-6f);
            maxAbs = std::max(maxAbs, absError);
            maxRel = std::max(maxRel, relError);
            if (absError > tolerance + tolerance * std::fabs(expected)) {
                ++failures;
            }
        }
    }
    std::printf("CORRECTNESS case=%s binary=%s result=%s failures=%zu max_abs=%.8g max_rel=%.8g\n",
                spec.name, binary, failures == 0 ? "PASS" : "FAIL", failures, maxAbs, maxRel);
    return failures == 0;
}

size_t CountOutputMismatches(const CaseSpec& spec, const std::vector<uint16_t>& parent,
                             const std::vector<uint16_t>& candidate)
{
    size_t mismatches = 0;
    for (size_t index = 0; index < parent.size(); ++index) {
        if (parent[index] != candidate[index]) {
            ++mismatches;
        }
    }
    std::printf("PARENT_CANDIDATE_EQUALITY case=%s result=%s mismatches=%zu elements=%zu\n",
                spec.name, mismatches == 0 ? "PASS" : "DIFFERENT", mismatches,
                parent.size());
    return mismatches;
}

bool RunCase(const CaseSpec& spec, int device, aclrtStream stream)
{
    const size_t elementCount = static_cast<size_t>(spec.rows) * spec.width;
    const size_t dataBytes = elementCount * sizeof(uint16_t);
    const size_t paramBytes = static_cast<size_t>(spec.width) * sizeof(uint16_t);
    std::vector<uint16_t> x(elementCount), residual(elementCount);
    std::vector<uint16_t> gamma(static_cast<size_t>(spec.width));
    std::vector<uint16_t> bias(static_cast<size_t>(spec.width));
    std::vector<uint16_t> parentOutput(elementCount, 0xffffu);
    std::vector<uint16_t> candidateOutput(elementCount, 0xffffu);
    for (size_t i = 0; i < elementCount; ++i) {
        const float xv = 0.19f * std::sin(static_cast<float>((i * 17) % 997) * 0.013f);
        const float rv = 0.11f * std::cos(static_cast<float>((i * 29) % 991) * 0.017f);
        x[i] = Encode(xv, spec.dtype);
        residual[i] = Encode(rv, spec.dtype);
    }
    for (int col = 0; col < spec.width; ++col) {
        gamma[static_cast<size_t>(col)] =
            Encode(0.85f + static_cast<float>(col % 23) * 0.002f, spec.dtype);
        bias[static_cast<size_t>(col)] =
            Encode(-0.025f + static_cast<float>(col % 19) * 0.001f, spec.dtype);
    }

    DeviceBuffers buffers;
    Allocate(&buffers.x, dataBytes, "malloc x");
    Allocate(&buffers.residual, dataBytes, "malloc residual");
    Allocate(&buffers.gamma, paramBytes, "malloc gamma");
    Allocate(&buffers.bias, paramBytes, "malloc bias");
    Allocate(&buffers.parentOutput, dataBytes, "malloc parent output");
    Allocate(&buffers.candidateOutput, dataBytes, "malloc candidate output");
    CheckAcl(aclrtMemcpy(buffers.x, dataBytes, x.data(), dataBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy x");
    CheckAcl(aclrtMemcpy(buffers.residual, dataBytes, residual.data(), dataBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy residual");
    CheckAcl(aclrtMemcpy(buffers.gamma, paramBytes, gamma.data(), paramBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma");
    CheckAcl(aclrtMemcpy(buffers.bias, paramBytes, bias.data(), paramBytes,
                         ACL_MEMCPY_HOST_TO_DEVICE), "copy bias");

    const int64_t dataShape[2] = {spec.rows, spec.width};
    const int64_t paramShape[1] = {spec.width};
    const TensorInfo dataInfo{dataShape, 2, spec.dtype};
    const TensorInfo paramInfo{paramShape, 1, spec.dtype};
    const TensorGroupInfo dataGroup{&dataInfo, 1};
    const TensorGroupInfo paramGroup{&paramInfo, 1};

    std::printf("CASE_BEGIN name=%s dtype=%s rows=%d width=%d device=%d core_count=%d expected_tile=%d expected_tail=%d\n",
                spec.name, spec.dtype == 1 ? "fp16" : "bf16", spec.rows, spec.width,
                device, kCoreCount, spec.expectedTileWidth, spec.expectedTail);
    Launch(&addr_v001_parent::addr_v001_parent_run_kernel, buffers.parentOutput, buffers,
           dataGroup, paramGroup, stream);
    Launch(&addr_v001_candidate::addr_v001_candidate_run_kernel, buffers.candidateOutput,
           buffers, dataGroup, paramGroup, stream);
    CheckAcl(aclrtMemcpy(parentOutput.data(), dataBytes, buffers.parentOutput, dataBytes,
                         ACL_MEMCPY_DEVICE_TO_HOST), "copy parent output");
    CheckAcl(aclrtMemcpy(candidateOutput.data(), dataBytes, buffers.candidateOutput, dataBytes,
                         ACL_MEMCPY_DEVICE_TO_HOST), "copy candidate output");

    const bool parentPass = CheckOutput(spec, x, residual, gamma, bias, parentOutput, "PARENT_R31B_V011");
    const bool candidatePass = CheckOutput(spec, x, residual, gamma, bias, candidateOutput, "CANDIDATE_ADDR_V001");
    const bool identical = CountOutputMismatches(spec, parentOutput, candidateOutput) == 0;
    std::printf("CASE_END name=%s parent=%s candidate=%s exact_output_match=%s\n",
                spec.name, parentPass ? "PASS" : "FAIL", candidatePass ? "PASS" : "FAIL",
                identical ? "PASS" : "DIFFERENT");
    FreeBuffers(buffers);
    return parentPass && candidatePass && identical;
}

}  // namespace

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::fprintf(stderr, "usage: addr_v001_correctness <device>\n");
        return 2;
    }
    const int device = std::atoi(argv[1]);
    if (device < 0 || device > 7) {
        std::fprintf(stderr, "invalid device: %s\n", argv[1]);
        return 2;
    }

    CheckAcl(aclInit(nullptr), "aclInit");
    CheckAcl(aclrtSetDevice(device), "set device");
    aclrtStream stream = nullptr;
    CheckAcl(aclrtCreateStream(&stream), "create stream");

    // rows = 8 * planned per-core batch + 1: exercises row rollover and a
    // partial final batch while keeping the exact planning width/dtype matrix.
    const CaseSpec cases[] = {
        {"fp16-d12288", 1, 12288, 25, 4096, 4096},
        {"fp16-d16384", 1, 16384, 17, 4096, 4096},
        {"fp16-d24576", 1, 24576, 9, 4096, 4096},
        {"fp16-d32768", 1, 32768, 9, 4096, 4096},
        {"bf16-d12288", 2, 12288, 17, 4096, 4096},
        {"bf16-d16384", 2, 16384, 9, 4096, 4096},
        {"bf16-d24576", 2, 24576, 9, 4096, 4096},
        {"bf16-d32768-tail", 2, 32768, 9, 2560, 2048},
        {"bf16-d40960", 2, 40960, 9, 2048, 2048},
    };

    bool allPass = true;
    for (const CaseSpec& spec : cases) {
        allPass = RunCase(spec, device, stream) && allPass;
    }

    CheckAcl(aclrtDestroyStream(stream), "destroy stream");
    CheckAcl(aclrtResetDevice(device), "reset device");
    CheckAcl(aclFinalize(), "aclFinalize");
    std::printf("CORRECTNESS_COMPLETE result=%s cases=%zu parent_candidate_same_runner=PASS target_dtypes=FP16,BF16\n",
                allPass ? "PASS" : "FAIL", sizeof(cases) / sizeof(cases[0]));
    return allPass ? 0 : 1;
}
