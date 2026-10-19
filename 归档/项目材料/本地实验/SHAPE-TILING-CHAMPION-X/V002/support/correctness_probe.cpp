// SHAPE-TILING-CHAMPION-X V001 NPU correctness probe.
// Host runner; the candidate kernel is linked as shape_tiling_v001.so.
// usage: correctness_probe <dtype 0|1|2> <rows> <width> <device> [warmup] [repeats]
// repeats=0 -> single launch, correctness only.

#include "local_abi_shim.h"

#include <acl/acl.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

extern "C" void run_kernel(
    GM_ADDR x, const TensorGroupInfo& info_x,
    GM_ADDR residual, const TensorGroupInfo& info_residual,
    GM_ADDR gamma, const TensorGroupInfo& info_gamma,
    GM_ADDR bias, const TensorGroupInfo& info_bias,
    GM_ADDR output, const TensorGroupInfo& info_output,
    int64_t availableCoreNum, aclrtStream stream, float epsilon);

static void CheckAcl(aclError status, const char* operation)
{
    if (status != ACL_SUCCESS) {
        std::fprintf(stderr, "%s failed: %d %s\n", operation, status,
                     aclGetRecentErrMsg() ? aclGetRecentErrMsg() : "");
        std::exit(2);
    }
}

static uint16_t FloatToHalf(float value)
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

static float HalfToFloat(uint16_t value)
{
    const uint32_t sign = (static_cast<uint32_t>(value & 0x8000u)) << 16;
    const uint32_t exponent = (value >> 10) & 0x1fu;
    uint32_t mantissa = value & 0x3ffu;
    uint32_t bits = 0;
    if (exponent == 0) {
        if (mantissa == 0) {
            bits = sign;
        } else {
            int32_t e = -14;
            while ((mantissa & 0x400u) == 0) {
                mantissa <<= 1;
                --e;
            }
            mantissa &= 0x3ffu;
            bits = sign | (static_cast<uint32_t>(e + 127) << 23) | (mantissa << 13);
        }
    } else if (exponent == 31) {
        bits = sign | 0x7f800000u | (mantissa << 13);
    } else {
        bits = sign | ((exponent - 1 + 127) << 23) | (mantissa << 13);
    }
    float out = 0.0f;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

static uint16_t FloatToBfloat16(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    if ((bits & 0x7f800000u) == 0x7f800000u && (bits & 0x007fffffu) != 0u) {
        return static_cast<uint16_t>((bits >> 16) | 0x0040u);
    }
    bits += 0x7fffu + ((bits >> 16) & 1u);
    return static_cast<uint16_t>(bits >> 16);
}

static float Bfloat16ToFloat(uint16_t value)
{
    const uint32_t bits = static_cast<uint32_t>(value) << 16;
    float out = 0.0f;
    std::memcpy(&out, &bits, sizeof(out));
    return out;
}

static float Decode(const void* base, size_t index, int dtype)
{
    if (dtype == 0) {
        return static_cast<const float*>(base)[index];
    }
    const uint16_t raw = static_cast<const uint16_t*>(base)[index];
    return dtype == 1 ? HalfToFloat(raw) : Bfloat16ToFloat(raw);
}

static void Encode(void* base, size_t index, float value, int dtype)
{
    if (dtype == 0) {
        static_cast<float*>(base)[index] = value;
        return;
    }
    static_cast<uint16_t*>(base)[index] =
        dtype == 1 ? FloatToHalf(value) : FloatToBfloat16(value);
}

static float Rounded(float value, int dtype)
{
    if (dtype == 0) {
        return value;
    }
    return dtype == 1 ? HalfToFloat(FloatToHalf(value))
                      : Bfloat16ToFloat(FloatToBfloat16(value));
}

int main(int argc, char** argv)
{
    if (argc < 5) {
        std::fprintf(stderr, "usage: correctness_probe dtype rows width device [warmup] [repeats] [dump_path]\n");
        return 2;
    }
    const int dtype = std::atoi(argv[1]);
    const int64_t rows = std::atoll(argv[2]);
    const int64_t width = std::atoll(argv[3]);
    const int device = std::atoi(argv[4]);
    const int warmup = argc > 5 ? std::atoi(argv[5]) : 0;
    const int repeats = argc > 6 ? std::atoi(argv[6]) : 0;
    const char* dumpPath = argc > 7 ? argv[7] : nullptr;
    if (dtype < 0 || dtype > 2 || rows <= 0 || width <= 0 || warmup < 0 || repeats < 0) {
        std::fprintf(stderr, "invalid probe arguments\n");
        return 2;
    }

    CheckAcl(aclInit(nullptr), "aclInit");
    CheckAcl(aclrtSetDevice(device), "aclrtSetDevice");
    aclrtStream stream = nullptr;
    CheckAcl(aclrtCreateStream(&stream), "aclrtCreateStream");

    const size_t elementCount = static_cast<size_t>(rows) * static_cast<size_t>(width);
    const size_t elemBytes = dtype == 0 ? sizeof(float) : sizeof(uint16_t);
    const size_t dataBytes = elementCount * elemBytes;
    const size_t paramBytes = static_cast<size_t>(width) * elemBytes;

    std::vector<uint8_t> x(dataBytes), residual(dataBytes), gamma(paramBytes), bias(paramBytes);
    std::vector<uint8_t> output(dataBytes);
    std::vector<float> expected(elementCount);
    for (size_t i = 0; i < elementCount; ++i) {
        const float xv = 0.19f * std::sin(static_cast<float>((i * 17) % 997) * 0.013f);
        const float rv = 0.11f * std::cos(static_cast<float>((i * 29) % 991) * 0.017f);
        Encode(x.data(), i, xv, dtype);
        Encode(residual.data(), i, rv, dtype);
    }
    for (int64_t col = 0; col < width; ++col) {
        Encode(gamma.data(), static_cast<size_t>(col),
               0.85f + static_cast<float>(col % 23) * 0.002f, dtype);
        Encode(bias.data(), static_cast<size_t>(col),
               -0.025f + static_cast<float>(col % 19) * 0.001f, dtype);
    }

    void* dx = nullptr;
    void* dr = nullptr;
    void* dg = nullptr;
    void* db = nullptr;
    void* dout = nullptr;
    CheckAcl(aclrtMalloc(&dx, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc x");
    CheckAcl(aclrtMalloc(&dr, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc residual");
    CheckAcl(aclrtMalloc(&dg, paramBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc gamma");
    CheckAcl(aclrtMalloc(&db, paramBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc bias");
    CheckAcl(aclrtMalloc(&dout, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc output");
    CheckAcl(aclrtMemcpy(dx, dataBytes, x.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy x");
    CheckAcl(aclrtMemcpy(dr, dataBytes, residual.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy residual");
    CheckAcl(aclrtMemcpy(dg, paramBytes, gamma.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma");
    CheckAcl(aclrtMemcpy(db, paramBytes, bias.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy bias");

    const int64_t xShape[2] = {rows, width};
    const int64_t paramShape[1] = {width};
    const TensorInfo dataInfo{xShape, 2, dtype};
    const TensorInfo paramInfo{paramShape, 1, dtype};
    const TensorGroupInfo dataGroup{&dataInfo, 1};
    const TensorGroupInfo paramGroup{&paramInfo, 1};
    const float epsilon = 1.0e-5f;
    const int64_t coreCount = 8;

    auto launch = [&]() {
        run_kernel((GM_ADDR)dx, dataGroup,
                   (GM_ADDR)dr, dataGroup,
                   (GM_ADDR)dg, paramGroup,
                   (GM_ADDR)db, paramGroup,
                   (GM_ADDR)dout, dataGroup,
                   coreCount, stream, epsilon);
        CheckAcl(aclrtSynchronizeStream(stream), "kernel synchronize");
    };

    std::vector<double> samples;
    if (repeats == 0 && warmup == 0) {
        launch();
    } else {
        for (int i = 0; i < warmup; ++i) {
            launch();
        }
        samples.reserve(static_cast<size_t>(repeats));
        for (int i = 0; i < repeats; ++i) {
            const auto start = std::chrono::steady_clock::now();
            launch();
            const auto stop = std::chrono::steady_clock::now();
            samples.push_back(std::chrono::duration<double, std::micro>(stop - start).count());
        }
        if (repeats == 0) {
            launch();
        }
    }
    CheckAcl(aclrtMemcpy(output.data(), dataBytes, dout, dataBytes, ACL_MEMCPY_DEVICE_TO_HOST),
             "copy output");

    if (dumpPath != nullptr) {
        std::FILE* dump = std::fopen(dumpPath, "wb");
        if (dump == nullptr) {
            std::fprintf(stderr, "cannot open dump path %s\n", dumpPath);
            return 2;
        }
        // Normalise to float32 text so parent/candidate dumps are comparable
        // across dtypes.
        for (size_t i = 0; i < elementCount; ++i) {
            const float v = Decode(output.data(), i, dtype);
            std::fprintf(dump, "%.9g\n", v);
        }
        std::fclose(dump);
    }

    // Reference: all arithmetic in FP32.  FP16 rounds y and the output path the
    // same way the kernel's native half ops do; BF16 keeps FP32 y and rounds the
    // final store (matches the Champion source's dtype policy).
    float maxAbs = 0.0f;
    float maxRel = 0.0f;
    size_t failures = 0;
    // FP32 must absorb ReduceSum partial-regrouping ULPs (tile count changes);
    // FP16/BF16 use the tolerances that already passed on this kernel family.
    const float tolerance = dtype == 0 ? 1.0e-4f : (dtype == 1 ? 0.004f : 0.02f);
    for (int64_t row = 0; row < rows; ++row) {
        double squareSum = 0.0;
        for (int64_t col = 0; col < width; ++col) {
            const size_t index = static_cast<size_t>(row) * static_cast<size_t>(width) +
                                 static_cast<size_t>(col);
            float y = Decode(x.data(), index, dtype) + Decode(residual.data(), index, dtype);
            if (dtype == 1) {
                y = Rounded(y, dtype);
            }
            expected[index] = y;
            squareSum += static_cast<double>(y) * static_cast<double>(y);
        }
        const float invRms = static_cast<float>(
            1.0 / std::sqrt(squareSum / static_cast<double>(width) + static_cast<double>(epsilon)));
        for (int64_t col = 0; col < width; ++col) {
            const size_t index = static_cast<size_t>(row) * static_cast<size_t>(width) +
                                 static_cast<size_t>(col);
            float value = expected[index] * invRms;
            const float gammaCol = Decode(gamma.data(), static_cast<size_t>(col), dtype);
            const float biasCol = Decode(bias.data(), static_cast<size_t>(col), dtype);
            if (dtype == 1) {
                value = Rounded(value, dtype);
                value = Rounded(value * gammaCol, dtype);
                value = Rounded(value + biasCol, dtype);
            } else if (dtype == 2) {
                value = Rounded(value * gammaCol + biasCol, dtype);
            } else {
                value = value * gammaCol + biasCol;
            }
            const float actual = Decode(output.data(), index, dtype);
            const float absError = std::fabs(actual - value);
            const float relError = absError / std::max(std::fabs(value), 1.0e-6f);
            maxAbs = std::max(maxAbs, absError);
            maxRel = std::max(maxRel, relError);
            if (absError > tolerance + tolerance * std::fabs(value)) {
                if (failures < 5) {
                    std::printf("MISMATCH row=%lld col=%lld expected=%.8g actual=%.8g y=%.8g invRms=%.8g gamma=%.8g bias=%.8g\n",
                                static_cast<long long>(row), static_cast<long long>(col),
                                value, actual, expected[index], invRms, gammaCol, biasCol);
                }
                ++failures;
            }
        }
    }

    const char* dtypeName = dtype == 0 ? "fp32" : (dtype == 1 ? "fp16" : "bf16");
    if (samples.empty()) {
        std::printf("dtype=%s rows=%lld width=%lld device=%d correctness=%s failures=%zu max_abs=%.8g max_rel=%.8g median_us=NA samples=0\n",
                    dtypeName, static_cast<long long>(rows), static_cast<long long>(width),
                    device, failures == 0 ? "PASS" : "FAIL", failures, maxAbs, maxRel);
    } else {
        std::sort(samples.begin(), samples.end());
        const double medianUs = samples[samples.size() / 2];
        std::printf("dtype=%s rows=%lld width=%lld device=%d correctness=%s failures=%zu max_abs=%.8g max_rel=%.8g median_us=%.3f samples=%d\n",
                    dtypeName, static_cast<long long>(rows), static_cast<long long>(width),
                    device, failures == 0 ? "PASS" : "FAIL", failures, maxAbs, maxRel, medianUs,
                    repeats);
    }

    CheckAcl(aclrtFree(dout), "free output");
    CheckAcl(aclrtFree(db), "free bias");
    CheckAcl(aclrtFree(dg), "free gamma");
    CheckAcl(aclrtFree(dr), "free residual");
    CheckAcl(aclrtFree(dx), "free x");
    CheckAcl(aclrtDestroyStream(stream), "destroy stream");
    CheckAcl(aclrtResetDevice(device), "reset device");
    CheckAcl(aclFinalize(), "aclFinalize");
    return failures == 0 ? 0 : 1;
}
