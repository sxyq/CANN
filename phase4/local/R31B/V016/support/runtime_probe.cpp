#include <acl/acl.h>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

#ifndef TENSOR_GROUP_INFO_DEFINED
#define TENSOR_GROUP_INFO_DEFINED
struct TensorInfo { const int64_t* shape; int64_t numDims; int32_t dtype; };
struct TensorGroupInfo { const TensorInfo* tensors; int64_t numTensors; };
#endif

#if defined(R31B_PROBE_PARENT)
#include "R31B-V011-LP-ROW-PIPELINE_kernel.asc"
#else
#include "R31B-V016-WIDE-TILE-SEED_kernel.asc"
#endif

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

static uint16_t FloatToBfloat16(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    bits += 0x7fffu + ((bits >> 16) & 1u);
    return static_cast<uint16_t>(bits >> 16);
}

static float Bfloat16ToFloat(uint16_t value)
{
    const uint32_t bits = static_cast<uint32_t>(value) << 16;
    float result = 0.0f;
    std::memcpy(&result, &bits, sizeof(result));
    return result;
}

static float Decode(uint16_t value, int dtype)
{
    return dtype == 1 ? HalfToFloat(value) : Bfloat16ToFloat(value);
}

static uint16_t Encode(float value, int dtype)
{
    return dtype == 1 ? FloatToHalf(value) : FloatToBfloat16(value);
}

static float Rounded(float value, int dtype)
{
    return Decode(Encode(value, dtype), dtype);
}

int main(int argc, char** argv)
{
    if (argc != 7) {
        std::fprintf(stderr, "usage: probe dtype rows width device warmup repeats\n");
        return 2;
    }
    const int dtype = std::atoi(argv[1]);
    const int64_t rows = std::atoll(argv[2]);
    const int64_t width = std::atoll(argv[3]);
    const int device = std::atoi(argv[4]);
    const int warmup = std::atoi(argv[5]);
    const int repeats = std::atoi(argv[6]);
    if ((dtype != 1 && dtype != 2) || rows <= 0 || width <= 8192 ||
        warmup < 0 || repeats < 1) {
        std::fprintf(stderr, "invalid probe arguments\n");
        return 2;
    }

    CheckAcl(aclInit(nullptr), "aclInit");
    CheckAcl(aclrtSetDevice(device), "aclrtSetDevice");
    aclrtStream stream = nullptr;
    CheckAcl(aclrtCreateStream(&stream), "aclrtCreateStream");

    const size_t elementCount = static_cast<size_t>(rows * width);
    const size_t dataBytes = elementCount * sizeof(uint16_t);
    const size_t paramBytes = static_cast<size_t>(width) * sizeof(uint16_t);
    std::vector<uint16_t> x(elementCount), residual(elementCount), gamma(width), bias(width);
    std::vector<uint16_t> output(elementCount);
    std::vector<float> expected(elementCount);
    for (size_t i = 0; i < elementCount; ++i) {
        const float xv = 0.19f * std::sin(static_cast<float>((i * 17) % 997) * 0.013f);
        const float rv = 0.11f * std::cos(static_cast<float>((i * 29) % 991) * 0.017f);
        x[i] = Encode(xv, dtype);
        residual[i] = Encode(rv, dtype);
    }
    for (int64_t col = 0; col < width; ++col) {
        gamma[col] = Encode(0.85f + static_cast<float>(col % 23) * 0.002f, dtype);
        bias[col] = Encode(-0.025f + static_cast<float>(col % 19) * 0.001f, dtype);
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
    for (int i = 0; i < warmup; ++i) {
        launch();
    }
    std::vector<double> samples;
    samples.reserve(static_cast<size_t>(repeats));
    for (int i = 0; i < repeats; ++i) {
        const auto start = std::chrono::steady_clock::now();
        launch();
        const auto stop = std::chrono::steady_clock::now();
        samples.push_back(std::chrono::duration<double, std::micro>(stop - start).count());
    }
    CheckAcl(aclrtMemcpy(output.data(), dataBytes, dout, dataBytes, ACL_MEMCPY_DEVICE_TO_HOST),
             "copy output");

    float maxAbs = 0.0f;
    float maxRel = 0.0f;
    size_t failures = 0;
    const float tolerance = dtype == 1 ? 0.004f : 0.02f;
    for (int64_t row = 0; row < rows; ++row) {
        float squareSum = 0.0f;
        for (int64_t col = 0; col < width; ++col) {
            const size_t index = static_cast<size_t>(row * width + col);
            float y = Decode(x[index], dtype) + Decode(residual[index], dtype);
            if (dtype == 1) {
                y = Rounded(y, dtype);
            }
            expected[index] = y;
            squareSum += y * y;
        }
        const float invRms = 1.0f / std::sqrt(squareSum / static_cast<float>(width) + epsilon);
        for (int64_t col = 0; col < width; ++col) {
            const size_t index = static_cast<size_t>(row * width + col);
            float value = expected[index] * invRms;
            if (dtype == 1) {
                value = Rounded(value, dtype);
                value = Rounded(value * Decode(gamma[col], dtype), dtype);
                value = Rounded(value + Decode(bias[col], dtype), dtype);
            } else {
                value = value * Decode(gamma[col], dtype) + Decode(bias[col], dtype);
                value = Rounded(value, dtype);
            }
            const float actual = Decode(output[index], dtype);
            const float absError = std::fabs(actual - value);
            const float relError = absError / std::max(std::fabs(value), 1.0e-6f);
            maxAbs = std::max(maxAbs, absError);
            maxRel = std::max(maxRel, relError);
            if (absError > tolerance + tolerance * std::fabs(value)) {
                ++failures;
            }
        }
    }
    std::sort(samples.begin(), samples.end());
    const double medianUs = samples[samples.size() / 2];
    std::printf("dtype=%s rows=%lld width=%lld device=%d correctness=%s failures=%zu max_abs=%.8g max_rel=%.8g median_us=%.3f samples=%d\n",
                dtype == 1 ? "fp16" : "bf16", static_cast<long long>(rows),
                static_cast<long long>(width), device, failures == 0 ? "PASS" : "FAIL",
                failures, maxAbs, maxRel, medianUs, repeats);

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
