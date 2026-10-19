#include <acl/acl.h>
#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#ifndef TENSOR_GROUP_INFO_DEFINED
#define TENSOR_GROUP_INFO_DEFINED
struct TensorInfo {
    const int64_t* shape;
    int64_t numDims;
    int32_t dtype;
};
struct TensorGroupInfo {
    const TensorInfo* tensors;
    int64_t numTensors;
};
#endif

#define BRX_SUBMISSION_STRINGIFY(x) #x
#define BRX_SUBMISSION_TOSTRING(x) BRX_SUBMISSION_STRINGIFY(x)
#ifndef BRX_SUBMISSION
#define BRX_SUBMISSION submission_v001.asc
#endif
#include BRX_SUBMISSION_TOSTRING(BRX_SUBMISSION)

#define ACL_CHECK(expr) do { \
    aclError r_ = (expr); \
    if (r_ != ACL_SUCCESS) { \
        std::fprintf(stderr, "ACL_FAIL %s=%d at %s:%d\n", #expr, (int)r_, __FILE__, __LINE__); \
        return false; \
    } \
} while (0)

namespace {

constexpr float kEpsilon = 1.0e-5f;

float InputValue(int64_t index, int mult, int off)
{
    return static_cast<float>((index * mult + off) % 997) / 498.0f - 1.0f;
}

uint16_t FloatToHalfBits(float v)
{
    aclFloat16 h = aclFloatToFloat16(v);
    uint16_t bits = 0;
    std::memcpy(&bits, &h, sizeof(bits));
    return bits;
}

float HalfBitsToFloat(uint16_t bits)
{
    aclFloat16 h;
    std::memcpy(&h, &bits, sizeof(bits));
    return aclFloat16ToFloat(h);
}

uint16_t FloatToBf16Bits(float v)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &v, sizeof(bits));
    bits += 0x7fffU + ((bits >> 16) & 1U);
    return static_cast<uint16_t>(bits >> 16);
}

float Bf16BitsToFloat(uint16_t bits)
{
    uint32_t wide = static_cast<uint32_t>(bits) << 16;
    float v = 0.0f;
    std::memcpy(&v, &wide, sizeof(v));
    return v;
}

void StoreValue(std::vector<uint8_t>& data, size_t idx, int dtype, float v)
{
    if (dtype == 0) {
        std::memcpy(data.data() + idx * sizeof(float), &v, sizeof(v));
        return;
    }
    uint16_t bits = dtype == 1 ? FloatToHalfBits(v) : FloatToBf16Bits(v);
    std::memcpy(data.data() + idx * sizeof(bits), &bits, sizeof(bits));
}

float LoadValue(const std::vector<uint8_t>& data, size_t idx, int dtype)
{
    if (dtype == 0) {
        float v = 0.0f;
        std::memcpy(&v, data.data() + idx * sizeof(float), sizeof(v));
        return v;
    }
    uint16_t bits = 0;
    std::memcpy(&bits, data.data() + idx * sizeof(bits), sizeof(bits));
    return dtype == 1 ? HalfBitsToFloat(bits) : Bf16BitsToFloat(bits);
}

void Golden(const std::vector<float>& x, const std::vector<float>& r,
            const std::vector<float>& g, const std::vector<float>& b,
            std::vector<float>& out, int64_t rows, int64_t d)
{
    out.assign(static_cast<size_t>(rows * d), 0.0f);
    for (int64_t i = 0; i < rows; ++i) {
        double sumSq = 0.0;
        for (int64_t j = 0; j < d; ++j) {
            const float u = x[i * d + j] + r[i * d + j];
            sumSq += static_cast<double>(u) * static_cast<double>(u);
        }
        const double mean = sumSq / static_cast<double>(d);
        const float rms = static_cast<float>(std::sqrt(mean + kEpsilon));
        const float inv = 1.0f / rms;
        for (int64_t j = 0; j < d; ++j) {
            const float u = x[i * d + j] + r[i * d + j];
            out[i * d + j] = u * inv * g[j] + b[j];
        }
    }
}

struct Shape {
    int64_t rows;
    int64_t d;
    const char* name;
};

bool RunCase(int deviceId, int dtype, int64_t rows, int64_t d, const char* name)
{
    const size_t n = static_cast<size_t>(rows * d);
    const size_t pn = static_cast<size_t>(d);
    const size_t xb = n * (dtype == 0 ? 4 : 2);
    const size_t pb = pn * (dtype == 0 ? 4 : 2);

    std::vector<float> xF(n), rF(n), gF(pn), bF(pn);
    for (size_t i = 0; i < n; ++i) {
        xF[i] = InputValue(static_cast<int64_t>(i), 3, 1);
        rF[i] = InputValue(static_cast<int64_t>(i), 5, 7);
    }
    for (size_t i = 0; i < pn; ++i) {
        gF[i] = InputValue(static_cast<int64_t>(i), 11, 3) * 0.1f + 1.0f;
        bF[i] = InputValue(static_cast<int64_t>(i), 13, 5) * 0.05f;
    }

    std::vector<uint8_t> xH(xb), rH(xb), gH(pb), bH(pb), oH(xb, 0);
    for (size_t i = 0; i < n; ++i) {
        StoreValue(xH, i, dtype, xF[i]);
        StoreValue(rH, i, dtype, rF[i]);
    }
    for (size_t i = 0; i < pn; ++i) {
        StoreValue(gH, i, dtype, gF[i]);
        StoreValue(bH, i, dtype, bF[i]);
    }

    std::vector<float> golden;
    Golden(xF, rF, gF, bF, golden, rows, d);

    void *xd = nullptr, *rd = nullptr, *gd = nullptr, *bd = nullptr, *od = nullptr;
    aclrtStream stream = nullptr;
    bool ok = false;

    if (!ACL_CHECK(aclInit(nullptr))) goto done;
    if (!ACL_CHECK(aclrtSetDevice(deviceId))) goto done;
    if (!ACL_CHECK(aclrtCreateStream(&stream))) goto done;
    if (!ACL_CHECK(aclrtMalloc(&xd, xb, ACL_MEM_MALLOC_HUGE_FIRST))) goto done;
    if (!ACL_CHECK(aclrtMalloc(&rd, xb, ACL_MEM_MALLOC_HUGE_FIRST))) goto done;
    if (!ACL_CHECK(aclrtMalloc(&gd, pb, ACL_MEM_MALLOC_HUGE_FIRST))) goto done;
    if (!ACL_CHECK(aclrtMalloc(&bd, pb, ACL_MEM_MALLOC_HUGE_FIRST))) goto done;
    if (!ACL_CHECK(aclrtMalloc(&od, xb, ACL_MEM_MALLOC_HUGE_FIRST))) goto done;
    if (!ACL_CHECK(aclrtMemcpy(xd, xb, xH.data(), xb, ACL_MEMCPY_HOST_TO_DEVICE))) goto done;
    if (!ACL_CHECK(aclrtMemcpy(rd, xb, rH.data(), xb, ACL_MEMCPY_HOST_TO_DEVICE))) goto done;
    if (!ACL_CHECK(aclrtMemcpy(gd, pb, gH.data(), pb, ACL_MEMCPY_HOST_TO_DEVICE))) goto done;
    if (!ACL_CHECK(aclrtMemcpy(bd, pb, bH.data(), pb, ACL_MEMCPY_HOST_TO_DEVICE))) goto done;

    {
        int64_t xShape[] = {rows, d};
        TensorInfo xT[] = {{xShape, 2, dtype}};
        TensorGroupInfo infoX = {xT, 1};
        int64_t rShape[] = {rows, d};
        TensorInfo rT[] = {{rShape, 2, dtype}};
        TensorGroupInfo infoR = {rT, 1};
        int64_t gShape[] = {d};
        TensorInfo gT[] = {{gShape, 1, dtype}};
        TensorGroupInfo infoG = {gT, 1};
        int64_t bShape[] = {d};
        TensorInfo bT[] = {{bShape, 1, dtype}};
        TensorGroupInfo infoB = {bT, 1};
        int64_t oShape[] = {rows, d};
        TensorInfo oT[] = {{oShape, 2, dtype}};
        TensorGroupInfo infoO = {oT, 1};

        int64_t cores = 0;
        ACL_CHECK(aclrtGetDeviceInfo(deviceId, ACL_DEV_ATTR_VECTOR_CORE_NUM, &cores));
        if (cores <= 0) cores = 40;

        run_kernel((GM_ADDR)xd, infoX,
                   (GM_ADDR)rd, infoR,
                   (GM_ADDR)gd, infoG,
                   (GM_ADDR)bd, infoB,
                   (GM_ADDR)od, infoO,
                   cores, stream, kEpsilon);
    }

    if (!ACL_CHECK(aclrtSynchronizeStream(stream))) goto done;
    if (!ACL_CHECK(aclrtMemcpy(oH.data(), xb, od, xb, ACL_MEMCPY_DEVICE_TO_HOST))) goto done;

    {
        double maxAbs = 0.0;
        size_t bad = 0;
        const float tol = dtype == 0 ? 1e-5f : (dtype == 1 ? 2e-3f : 2e-2f);
        for (size_t i = 0; i < n; ++i) {
            const float got = LoadValue(oH, i, dtype);
            const float exp = golden[i];
            const float diff = std::fabs(got - exp);
            if (!(diff <= tol * (1.0f + std::fabs(exp)))) {
                ++bad;
            }
            if (diff > maxAbs) maxAbs = diff;
        }
        const bool pass = bad == 0;
        std::printf("CASE dtype=%d rows=%lld D=%lld name=%s bad=%zu max_abs=%.6g %s\n",
                    dtype, (long long)rows, (long long)d, name, bad, maxAbs,
                    pass ? "PASS" : "FAIL");
        if (!pass) {
            size_t shown = 0;
            for (size_t i = 0; i < n && shown < 5; ++i) {
                const float got = LoadValue(oH, i, dtype);
                const float exp = golden[i];
                if (std::fabs(got - exp) > tol * (1.0f + std::fabs(exp))) {
                    std::printf("  mismatch i=%zu got=%g exp=%g\n", i, got, exp);
                    ++shown;
                }
            }
        }
        ok = pass;
    }

done:
    if (od) aclrtFree(od);
    if (bd) aclrtFree(bd);
    if (gd) aclrtFree(gd);
    if (rd) aclrtFree(rd);
    if (xd) aclrtFree(xd);
    if (stream) aclrtDestroyStream(stream);
    aclrtResetDevice(deviceId);
    aclFinalize();
    return ok;
}

}  // namespace

int main(int argc, char** argv)
{
    int deviceId = 4;
    if (argc >= 2) deviceId = std::atoi(argv[1]);

    const Shape shapes[] = {
        {2, 256, "fast_multi_aligned_small"},
        {8, 256, "fast_multi_aligned"},
        {5, 512, "fast_multi_aligned_mid"},
        {4, 1024, "fast_multi_1k"},
        {16, 128, "fast_multi_many_rows"},
        {3, 65, "fast_unaligned_D"},
        {1, 256, "fast_single_row"},
        {2, 4096, "fast_boundary_d"},
        {4, 8192, "resident_mid"},
        {2, 16384, "resident_wide"},
        {64, 64, "resident_many_small"},
    };

    int failures = 0;
    int total = 0;
    const int dtypes[] = {0, 1, 2};
    for (int dt : dtypes) {
        for (const Shape& s : shapes) {
            ++total;
            if (!RunCase(deviceId, dt, s.rows, s.d, s.name)) {
                ++failures;
            }
        }
    }
    std::printf("SUMMARY total=%d failures=%d %s\n", total, failures,
                failures == 0 ? "ALL_PASS" : "HAS_FAIL");
    return failures == 0 ? 0 : 1;
}
