#pragma once
// Unified reference measurement layer for BATCH-RESIDENT-X (MAIN-2).
// Ported from SCHED-ROWGROUP-X support/runner_ref.inc (unified protocol).
// Host measurement only — does not alter kernel Candidate sources.
// Primary: device-side ACL events. Secondary: host steady_clock wall.
// Lifecycle: aclInit / setDevice / stream / malloc / H2D ONCE, then warmup,
// then measurement blocks in the SAME process, then D2H / cleanup once.
//
// usage: device_id rows width dtype result_prefix warmup samples blocks gap_sec [batch_n]
// argc contract: exactly 10 args, or 11 with batch_n (default 1).

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
#include <ctime>

#include "local_types.h"
#include SRX_SUBMISSION

#define ACL_CHECK(expr) do { \
    const aclError result = (expr); \
    if (result != ACL_SUCCESS) { \
        std::fprintf(stderr, "ACL failure %s=%d at %s:%d\n", #expr, result, __FILE__, __LINE__); \
        return 1; \
    } \
} while (0)

namespace refharness {

constexpr float kEpsilon = 1.0e-5f;

struct SamplePair {
    double device_us;
    double wall_us;
};

inline double MedianOf(std::vector<double> v)
{
    if (v.empty()) return 0.0;
    std::sort(v.begin(), v.end());
    const size_t n = v.size();
    if (n % 2 == 1) return v[n / 2];
    return 0.5 * (v[n / 2 - 1] + v[n / 2]);
}

inline double PercentileSorted(const std::vector<double>& s, double p)
{
    if (s.empty()) return 0.0;
    if (s.size() == 1) return s[0];
    const double idx = p * static_cast<double>(s.size() - 1);
    const size_t lo = static_cast<size_t>(idx);
    const size_t hi = lo + 1 < s.size() ? lo + 1 : lo;
    const double frac = idx - static_cast<double>(lo);
    return s[lo] * (1.0 - frac) + s[hi] * frac;
}

struct Stats {
    double median = 0, mean = 0, stdev = 0, mad = 0;
    double p10 = 0, p90 = 0, min_us = 0, max_us = 0;
    double abs_spread = 0, cv = 0, max_min = 0;
};

inline Stats ComputeStats(std::vector<double> vals)
{
    Stats st;
    if (vals.empty()) return st;
    const size_t n = vals.size();
    st.min_us = vals[0];
    st.max_us = vals[0];
    double sum = 0.0;
    for (double v : vals) {
        sum += v;
        if (v < st.min_us) st.min_us = v;
        if (v > st.max_us) st.max_us = v;
    }
    st.mean = sum / static_cast<double>(n);
    double var = 0.0;
    for (double v : vals) var += (v - st.mean) * (v - st.mean);
    var /= static_cast<double>(n);
    st.stdev = std::sqrt(var);
    st.cv = st.mean > 0 ? st.stdev / st.mean : 0.0;
    st.median = MedianOf(vals);
    std::vector<double> absdev;
    absdev.reserve(n);
    for (double v : vals) absdev.push_back(std::fabs(v - st.median));
    st.mad = MedianOf(absdev);
    std::sort(vals.begin(), vals.end());
    st.p10 = PercentileSorted(vals, 0.10);
    st.p90 = PercentileSorted(vals, 0.90);
    st.abs_spread = st.max_us - st.min_us;
    st.max_min = st.min_us > 0 ? st.max_us / st.min_us : 0.0;
    return st;
}

inline void WriteStats(std::ofstream& out, const char* tag, const Stats& st)
{
    out << tag << "\tmedian_us\t" << st.median << "\n"
        << tag << "\tmean_us\t" << st.mean << "\n"
        << tag << "\tstdev_us\t" << st.stdev << "\n"
        << tag << "\tMAD_us\t" << st.mad << "\n"
        << tag << "\tp10_us\t" << st.p10 << "\n"
        << tag << "\tp90_us\t" << st.p90 << "\n"
        << tag << "\tmin_us\t" << st.min_us << "\n"
        << tag << "\tmax_us\t" << st.max_us << "\n"
        << tag << "\tabs_spread_us\t" << st.abs_spread << "\n"
        << tag << "\tCV\t" << st.cv << "\n"
        << tag << "\tmax_min\t" << st.max_min << "\n"
        << tag << "\tn\t" << (st.mean == 0 && st.median == 0 ? 0 : 1) << "\n";
}

// Input generation and golden model: same formulas as npu_correctness.cpp
// (route's own correctness probe), so the post-timing D2H check matches it.
inline float InputValue(int64_t index, int multiplier, int offset)
{
    return static_cast<float>((index * multiplier + offset) % 997) / 498.0f - 1.0f;
}

inline uint16_t FloatToHalfBits(float value)
{
    const aclFloat16 halfValue = aclFloatToFloat16(value);
    uint16_t bits = 0;
    std::memcpy(&bits, &halfValue, sizeof(bits));
    return bits;
}

inline float HalfBitsToFloat(uint16_t bits)
{
    aclFloat16 halfValue;
    std::memcpy(&halfValue, &bits, sizeof(bits));
    return aclFloat16ToFloat(halfValue);
}

inline uint16_t FloatToBf16Bits(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(value));
    bits += 0x7fffU + ((bits >> 16) & 1U);
    return static_cast<uint16_t>(bits >> 16);
}

inline float Bf16BitsToFloat(uint16_t bits)
{
    const uint32_t wide = static_cast<uint32_t>(bits) << 16;
    float value = 0.0f;
    std::memcpy(&value, &wide, sizeof(value));
    return value;
}

inline void StoreValue(std::vector<uint8_t>& data, size_t index, int dtype, float value)
{
    if (dtype == 0) {
        std::memcpy(data.data() + index * sizeof(float), &value, sizeof(value));
        return;
    }
    const uint16_t bits = dtype == 1 ? FloatToHalfBits(value) : FloatToBf16Bits(value);
    std::memcpy(data.data() + index * sizeof(bits), &bits, sizeof(bits));
}

inline float LoadValue(const std::vector<uint8_t>& data, size_t index, int dtype)
{
    if (dtype == 0) {
        float value = 0.0f;
        std::memcpy(&value, data.data() + index * sizeof(value), sizeof(value));
        return value;
    }
    uint16_t bits = 0;
    std::memcpy(&bits, data.data() + index * sizeof(bits), sizeof(bits));
    return dtype == 1 ? HalfBitsToFloat(bits) : Bf16BitsToFloat(bits);
}

} // namespace refharness

int main(int argc, char** argv)
{
    using namespace refharness;
    if (argc != 10 && argc != 11) {
        std::fprintf(stderr,
            "usage: %s device_id rows width dtype result_prefix warmup samples blocks gap_sec [batch_n]\n",
            argv[0]);
        return 2;
    }
    const int deviceId = std::atoi(argv[1]);
    const int64_t rows = std::strtoll(argv[2], nullptr, 10);
    const int64_t width = std::strtoll(argv[3], nullptr, 10);
    const int dtype = std::atoi(argv[4]);
    const char* prefix = argv[5];
    const int warmup = std::atoi(argv[6]);
    const int samples = std::atoi(argv[7]);
    const int blocks = std::atoi(argv[8]);
    const int gapSec = std::atoi(argv[9]);
    const int batchN = argc >= 11 ? std::atoi(argv[10]) : 1;
    if (rows < 1 || width < 64 || width > 32768 || dtype < 0 || dtype > 2 ||
        warmup < 0 || samples < 1 || blocks < 1 || gapSec < 0 || batchN < 1) {
        std::fprintf(stderr, "invalid args\n");
        return 2;
    }

    const size_t elements = static_cast<size_t>(rows * width);
    const size_t elementBytes = dtype == 0 ? sizeof(float) : sizeof(uint16_t);
    const size_t dataBytes = elements * elementBytes;
    const size_t paramBytes = static_cast<size_t>(width) * elementBytes;

    // ---- init once ----
    ACL_CHECK(aclInit(nullptr));
    ACL_CHECK(aclrtSetDevice(deviceId));
    int64_t availableCoreNum = 0;
    ACL_CHECK(aclrtGetDeviceInfo(deviceId, ACL_DEV_ATTR_VECTOR_CORE_NUM, &availableCoreNum));
    if (availableCoreNum <= 0) availableCoreNum = 40;
    aclrtStream stream = nullptr;
    ACL_CHECK(aclrtCreateStream(&stream));

    int64_t xyShape[] = {rows, width};
    int64_t paramShape[] = {width};
    TensorInfo xTensor[] = {{xyShape, 2, dtype}};
    TensorInfo rTensor[] = {{xyShape, 2, dtype}};
    TensorInfo gTensor[] = {{paramShape, 1, dtype}};
    TensorInfo bTensor[] = {{paramShape, 1, dtype}};
    TensorInfo oTensor[] = {{xyShape, 2, dtype}};
    TensorGroupInfo infoX = {xTensor, 1}, infoR = {rTensor, 1};
    TensorGroupInfo infoG = {gTensor, 1}, infoB = {bTensor, 1}, infoO = {oTensor, 1};

    std::vector<uint8_t> hostX(dataBytes), hostR(dataBytes), hostG(paramBytes),
        hostB(paramBytes), hostOut(dataBytes);
    std::vector<float> refX(elements), refR(elements), refG(static_cast<size_t>(width)),
        refB(static_cast<size_t>(width)), refGolden(elements, 0.0f);
    for (size_t i = 0; i < elements; ++i) {
        refX[i] = InputValue(static_cast<int64_t>(i), 3, 1);
        refR[i] = InputValue(static_cast<int64_t>(i), 5, 7);
        StoreValue(hostX, i, dtype, refX[i]);
        StoreValue(hostR, i, dtype, refR[i]);
    }
    for (int64_t i = 0; i < width; ++i) {
        refG[static_cast<size_t>(i)] = InputValue(i, 11, 3) * 0.1f + 1.0f;
        refB[static_cast<size_t>(i)] = InputValue(i, 13, 5) * 0.05f;
        StoreValue(hostG, static_cast<size_t>(i), dtype, refG[static_cast<size_t>(i)]);
        StoreValue(hostB, static_cast<size_t>(i), dtype, refB[static_cast<size_t>(i)]);
    }

    void *deviceX = nullptr, *deviceR = nullptr, *deviceG = nullptr,
         *deviceB = nullptr, *deviceOut = nullptr;
    // malloc once
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceX), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceR), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceG), paramBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceB), paramBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceOut), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));

    // H2D once (before warmup, outside timed loop)
    ACL_CHECK(aclrtMemcpy(deviceX, dataBytes, hostX.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceR, dataBytes, hostR.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceG, paramBytes, hostG.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceB, paramBytes, hostB.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE));

    auto launch = [&]() {
        run_kernel((GM_ADDR)deviceX, infoX, (GM_ADDR)deviceR, infoR,
                   (GM_ADDR)deviceG, infoG, (GM_ADDR)deviceB, infoB,
                   (GM_ADDR)deviceOut, infoO, availableCoreNum, stream, kEpsilon);
    };

    // warmup once per process (shared across blocks)
    for (int i = 0; i < warmup; ++i) {
        launch();
        ACL_CHECK(aclrtSynchronizeStream(stream));
    }

    aclrtEvent evStart = nullptr, evStop = nullptr;
    ACL_CHECK(aclrtCreateEvent(&evStart));
    ACL_CHECK(aclrtCreateEvent(&evStop));

    std::string rawPath = std::string(prefix) + "-raw.tsv";
    std::ofstream raw(rawPath);
    raw << "block\trep\tdevice_us\thost_wall_us\n";

    std::vector<std::vector<SamplePair>> blockSamples(static_cast<size_t>(blocks));

    for (int b = 0; b < blocks; ++b) {
        if (b > 0 && gapSec > 0) {
            struct timespec ts = {gapSec, 0};
            nanosleep(&ts, nullptr);
        }
        ACL_CHECK(aclrtSynchronizeStream(stream));
        for (int i = 0; i < samples; ++i) {
            const auto wall0 = std::chrono::steady_clock::now();
            ACL_CHECK(aclrtRecordEvent(evStart, stream));
            for (int bi = 0; bi < batchN; ++bi) {
                launch();
            }
            ACL_CHECK(aclrtRecordEvent(evStop, stream));
            ACL_CHECK(aclrtSynchronizeEvent(evStop));
            float deviceMs = 0.0f;
            ACL_CHECK(aclrtEventElapsedTime(&deviceMs, evStart, evStop));
            ACL_CHECK(aclrtSynchronizeStream(stream));
            const auto wall1 = std::chrono::steady_clock::now();
            SamplePair sp;
            sp.device_us = static_cast<double>(deviceMs) * 1000.0 / static_cast<double>(batchN);
            sp.wall_us = std::chrono::duration<double, std::micro>(wall1 - wall0).count() /
                         static_cast<double>(batchN);
            blockSamples[static_cast<size_t>(b)].push_back(sp);
            raw << (b + 1) << '\t' << (i + 1) << '\t'
                << sp.device_us << '\t' << sp.wall_us << '\n';
        }
    }
    raw.close();

    // D2H once after all measurement (correctness never interleaved with timing)
    ACL_CHECK(aclrtMemcpy(hostOut.data(), dataBytes, deviceOut, dataBytes, ACL_MEMCPY_DEVICE_TO_HOST));

    // Golden model identical to npu_correctness.cpp (AddRMSNormBias semantics).
    for (int64_t r = 0; r < rows; ++r) {
        double sumSq = 0.0;
        for (int64_t c = 0; c < width; ++c) {
            const size_t idx = static_cast<size_t>(r * width + c);
            const float u = refX[idx] + refR[idx];
            sumSq += static_cast<double>(u) * static_cast<double>(u);
        }
        const double mean = sumSq / static_cast<double>(width);
        const float inv = 1.0f / static_cast<float>(std::sqrt(mean + kEpsilon));
        for (int64_t c = 0; c < width; ++c) {
            const size_t idx = static_cast<size_t>(r * width + c);
            const float u = refX[idx] + refR[idx];
            refGolden[idx] = u * inv * refG[static_cast<size_t>(c)] + refB[static_cast<size_t>(c)];
        }
    }

    float maxAbs = 0.0f;
    int bad = 0;
    const float tol = dtype == 0 ? 1.0e-5f : (dtype == 1 ? 2.0e-3f : 2.0e-2f);
    for (size_t i = 0; i < elements; ++i) {
        const float actual = LoadValue(hostOut, i, dtype);
        const float expected = refGolden[i];
        const float error = std::fabs(actual - expected);
        maxAbs = std::max(maxAbs, error);
        if (!std::isfinite(actual) || !(error <= tol * (1.0f + std::fabs(expected)))) ++bad;
    }

    std::string statsPath = std::string(prefix) + "-stats.txt";
    std::ofstream stats(statsPath);
    stats << "method\tDEVICE_EVENT_PRIMARY+HOST_WALL_SECONDARY\n"
          << "route\tBATCH-RESIDENT-X\n"
          << "device\t" << deviceId << "\n"
          << "dtype\t" << dtype << "\n"
          << "rows\t" << rows << "\n"
          << "width\t" << width << "\n"
          << "warmup\t" << warmup << "\n"
          << "samples_per_block\t" << samples << "\n"
          << "blocks\t" << blocks << "\n"
          << "gap_sec\t" << gapSec << "\n"
          << "batch_n\t" << batchN << "\n"
          << "bad\t" << bad << "\n"
          << "max_abs\t" << maxAbs << "\n";

    for (size_t b = 0; b < blockSamples.size(); ++b) {
        std::vector<double> dev, wall;
        for (const auto& s : blockSamples[b]) {
            dev.push_back(s.device_us);
            wall.push_back(s.wall_us);
        }
        char tag[32];
        std::snprintf(tag, sizeof(tag), "B%zu_DEVICE", b + 1);
        WriteStats(stats, tag, ComputeStats(dev));
        std::snprintf(tag, sizeof(tag), "B%zu_WALL", b + 1);
        WriteStats(stats, tag, ComputeStats(wall));
    }
    {
        std::vector<double> all;
        for (const auto& blk : blockSamples)
            for (const auto& s : blk) all.push_back(s.device_us);
        WriteStats(stats, "ALL_DEVICE", ComputeStats(all));
        std::vector<double> allw;
        for (const auto& blk : blockSamples)
            for (const auto& s : blk) allw.push_back(s.wall_us);
        WriteStats(stats, "ALL_WALL", ComputeStats(allw));
    }
    stats.close();

    std::printf("REF_HARNESS route=BATCH-RESIDENT-X device=%d shape=%lldx%d dtype=%d warmup=%d samples=%d blocks=%d batch_n=%d bad=%d\n",
                deviceId, static_cast<long long>(rows), width, dtype, warmup, samples, blocks, batchN, bad);

    ACL_CHECK(aclrtDestroyEvent(evStart));
    ACL_CHECK(aclrtDestroyEvent(evStop));
    ACL_CHECK(aclrtFree(deviceX));
    ACL_CHECK(aclrtFree(deviceR));
    ACL_CHECK(aclrtFree(deviceG));
    ACL_CHECK(aclrtFree(deviceB));
    ACL_CHECK(aclrtFree(deviceOut));
    ACL_CHECK(aclrtDestroyStream(stream));
    ACL_CHECK(aclFinalize());
    return bad == 0 ? 0 : 3;
}
