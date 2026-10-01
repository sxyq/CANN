#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {
constexpr float kEpsilon = 1.0e-5f;

uint16_t ToHalf(float value) {
    uint32_t bits; std::memcpy(&bits, &value, sizeof(bits));
    const uint32_t sign = (bits >> 16) & 0x8000u;
    const uint32_t exponent = (bits >> 23) & 0xffu;
    uint32_t mantissa = bits & 0x7fffffu;
    if (exponent == 0xffu) return static_cast<uint16_t>(sign | 0x7c00u | (mantissa ? 0x200u : 0));
    int exponent16 = static_cast<int>(exponent) - 112;
    if (exponent16 >= 31) return static_cast<uint16_t>(sign | 0x7c00u);
    if (exponent16 <= 0) {
        if (exponent16 < -10) return static_cast<uint16_t>(sign);
        mantissa |= 0x800000u;
        const int shift = 14 - exponent16;
        const uint32_t round = (1u << (shift - 1)) - 1u + ((mantissa >> shift) & 1u);
        return static_cast<uint16_t>(sign | ((mantissa + round) >> shift));
    }
    mantissa += 0xfffu + ((mantissa >> 13) & 1u);
    if (mantissa & 0x800000u) { mantissa = 0; if (++exponent16 >= 31) return static_cast<uint16_t>(sign | 0x7c00u); }
    return static_cast<uint16_t>(sign | (static_cast<uint32_t>(exponent16) << 10) | (mantissa >> 13));
}

float FromHalf(uint16_t value) {
    const float sign = (value & 0x8000u) ? -1.0f : 1.0f;
    const uint32_t exponent = (value >> 10) & 0x1fu, mantissa = value & 0x3ffu;
    if (!exponent) return sign * std::ldexp(static_cast<float>(mantissa), -24);
    if (exponent == 0x1fu) return mantissa ? NAN : sign * INFINITY;
    return sign * std::ldexp(1.0f + static_cast<float>(mantissa) / 1024.0f, static_cast<int>(exponent) - 15);
}

uint16_t ToBf16(float value) { uint32_t bits; std::memcpy(&bits, &value, sizeof(bits)); bits += 0x7fffu + ((bits >> 16) & 1u); return static_cast<uint16_t>(bits >> 16); }
float FromBf16(uint16_t value) { uint32_t bits = static_cast<uint32_t>(value) << 16; float result; std::memcpy(&result, &bits, sizeof(result)); return result; }
uint16_t Encode(float value, int dtype) { return dtype == 1 ? ToHalf(value) : ToBf16(value); }
float Decode(uint16_t value, int dtype) { return dtype == 1 ? FromHalf(value) : FromBf16(value); }
float RoundDtype(float value, int dtype) { return Decode(Encode(value, dtype), dtype); }

void Check(aclError status, const char* operation) {
    if (status != ACL_SUCCESS) { std::fprintf(stderr, "ACL_FAIL operation=%s status=%d\n", operation, status); std::exit(2); }
}

struct Case { const char* name; int rows; int width; int dtype; };

void RunCase(const Case& spec, int device, int blocks) {
    const size_t count = static_cast<size_t>(spec.rows) * spec.width;
    const size_t dataBytes = count * sizeof(uint16_t), paramBytes = static_cast<size_t>(spec.width) * sizeof(uint16_t);
    std::vector<uint16_t> x(count), residual(count), gamma(spec.width), bias(spec.width), output(count);
    for (size_t i = 0; i < count; ++i) {
        x[i] = Encode(0.19f * std::sin(static_cast<float>((i * 17) % 997) * 0.013f), spec.dtype);
        residual[i] = Encode(0.11f * std::cos(static_cast<float>((i * 29) % 991) * 0.017f), spec.dtype);
    }
    for (int col = 0; col < spec.width; ++col) {
        gamma[col] = Encode(0.85f + static_cast<float>(col % 23) * 0.002f, spec.dtype);
        bias[col] = Encode(-0.025f + static_cast<float>(col % 19) * 0.001f, spec.dtype);
    }
    void *dx = nullptr, *dr = nullptr, *dg = nullptr, *db = nullptr, *do_ = nullptr;
    Check(aclrtMalloc(&dx, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc_x");
    Check(aclrtMalloc(&dr, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc_residual");
    Check(aclrtMalloc(&dg, paramBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc_gamma");
    Check(aclrtMalloc(&db, paramBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc_bias");
    Check(aclrtMalloc(&do_, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "malloc_output");
    Check(aclrtMemcpy(dx, dataBytes, x.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy_x");
    Check(aclrtMemcpy(dr, dataBytes, residual.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy_residual");
    Check(aclrtMemcpy(dg, paramBytes, gamma.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy_gamma");
    Check(aclrtMemcpy(db, paramBytes, bias.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy_bias");
    const int64_t dataShape[] = {spec.rows, spec.width}, paramShape[] = {spec.width};
    TensorInfo dataInfo{dataShape, 2, spec.dtype}, paramInfo{paramShape, 1, spec.dtype};
    TensorGroupInfo dataGroup{&dataInfo, 1}, paramGroup{&paramInfo, 1};
    aclrtStream stream = nullptr; Check(aclrtCreateStream(&stream), "create_stream");
    run_kernel((GM_ADDR)dx, dataGroup, (GM_ADDR)dr, dataGroup, (GM_ADDR)dg, paramGroup,
               (GM_ADDR)db, paramGroup, (GM_ADDR)do_, dataGroup, blocks, stream, kEpsilon);
    Check(aclrtSynchronizeStream(stream), "synchronize");
    Check(aclrtMemcpy(output.data(), dataBytes, do_, dataBytes, ACL_MEMCPY_DEVICE_TO_HOST), "copy_output");
    float maxAbs = 0.0f, maxRel = 0.0f; size_t failures = 0;
    const float atol = spec.dtype == 1 ? 0.004f : (spec.dtype == 2 ? 0.02f : 1.0e-4f);
    for (int row = 0; row < spec.rows; ++row) {
        std::vector<float> y(spec.width); float squareSum = 0.0f;
        for (int col = 0; col < spec.width; ++col) { const size_t i = static_cast<size_t>(row) * spec.width + col; y[col] = Decode(x[i], spec.dtype) + Decode(residual[i], spec.dtype); if (spec.dtype == 1) y[col] = RoundDtype(y[col], spec.dtype); squareSum += y[col] * y[col]; }
        const float invRms = 1.0f / std::sqrt(squareSum / static_cast<float>(spec.width) + kEpsilon);
        for (int col = 0; col < spec.width; ++col) {
            const size_t i = static_cast<size_t>(row) * spec.width + col;
            float expected = y[col] * invRms;
            if (spec.dtype == 1) { expected = RoundDtype(expected, spec.dtype); expected = RoundDtype(expected * Decode(gamma[col], spec.dtype), spec.dtype); expected = RoundDtype(expected + Decode(bias[col], spec.dtype), spec.dtype); }
            else expected = RoundDtype(expected * Decode(gamma[col], spec.dtype) + Decode(bias[col], spec.dtype), spec.dtype);
            const float actual = Decode(output[i], spec.dtype), absError = std::fabs(actual - expected), relError = absError / std::max(std::fabs(expected), 1.0e-6f);
            maxAbs = std::max(maxAbs, absError); maxRel = std::max(maxRel, relError); if (absError > atol + atol * std::fabs(expected)) ++failures;
        }
    }
    std::printf("CORRECTNESS case=%s dtype=%d rows=%d width=%d blocks=%d device=%d atol=%.8g result=%s failures=%zu max_abs=%.8g max_rel=%.8g\n", spec.name, spec.dtype, spec.rows, spec.width, blocks, device, atol, failures ? "FAIL" : "PASS", failures, maxAbs, maxRel);
    Check(aclrtFree(do_), "free_output"); Check(aclrtFree(db), "free_bias"); Check(aclrtFree(dg), "free_gamma"); Check(aclrtFree(dr), "free_residual"); Check(aclrtFree(dx), "free_x"); Check(aclrtDestroyStream(stream), "destroy_stream");
    if (failures) std::exit(3);
}
}

int main(int argc, char** argv) {
    if (argc != 2) { std::fprintf(stderr, "usage: %s DEVICE\n", argv[0]); return 1; }
    const int device = std::atoi(argv[1]); Check(aclInit(nullptr), "acl_init"); Check(aclrtSetDevice(device), "set_device");
    const Case cases[] = {{"fp16-wide-16384", 1, 16384, 1}, {"fp16-wide-16384-rows3", 3, 16384, 1}, {"fp16-wide-32768", 1, 32768, 1}, {"fp16-wide-32768-rows2", 2, 32768, 1}, {"fp16-boundary-8192", 1, 8192, 1}, {"fp16-tail-16385", 1, 16385, 1}, {"bf16-wide-16384", 1, 16384, 2}, {"fp32-wide-16384", 1, 16384, 0}};
    for (const Case& spec : cases) RunCase(spec, device, 1);
    Check(aclrtResetDevice(device), "reset_device"); Check(aclFinalize(), "acl_finalize"); std::puts("CORRECTNESS_COMPLETE cases=8 result=PASS"); return 0;
}
