// SHAPE-TILING-CHAMPION-X — segmented timing harness (instrumentation only).
// Measures the unmodified parent (R31B-V011) against a Pass1-only copy so the
// Pass2 (normalize + affine + store) share can be obtained by subtraction.
// This is measurement instrumentation; it is NOT a candidate revision and the
// recorded parent/candidate sources are unchanged.

#include "paired_runner_abi.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {

using KernelFn = void (*)(GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
                          GM_ADDR, const TensorGroupInfo&, GM_ADDR, const TensorGroupInfo&,
                          GM_ADDR, const TensorGroupInfo&, int64_t, aclrtStream, float);

constexpr int kWarmup = 45;
constexpr int kSamples = 31;
constexpr int kCoreCount = 8;
constexpr float kEpsilon = 1.0e-5f;

struct CaseSpec {
    const char* name;
    int dtype;
    int rows;
    int width;
};

struct Buf {
    void* x = nullptr;
    void* residual = nullptr;
    void* gamma = nullptr;
    void* bias = nullptr;
    void* output = nullptr;
};

struct Ev {
    aclrtEvent start = nullptr;
    aclrtEvent stop = nullptr;
    bool recorded = false;
};

void CheckAcl(aclError s, const char* op)
{
    if (s != ACL_SUCCESS) {
        std::fprintf(stderr, "%s failed: %d\n", op, s);
        std::exit(2);
    }
}

float Decode(const void* base, size_t i, int dtype)
{
    if (dtype == 0) {
        return static_cast<const float*>(base)[i];
    }
    return static_cast<const uint16_t*>(base)[i];  // values unused in this harness
}

void Encode(void* base, size_t i, float v, int dtype)
{
    if (dtype == 0) {
        static_cast<float*>(base)[i] = v;
    } else {
        static_cast<uint16_t*>(base)[i] = static_cast<uint16_t>(0);
    }
}

double Median(std::vector<double> v)
{
    if (v.empty()) {
        return 0.0;
    }
    std::sort(v.begin(), v.end());
    return v[v.size() / 2];
}

double Measure(KernelFn fn, void* out, const Buf& b, const TensorGroupInfo& dg,
               const TensorGroupInfo& pg, const CaseSpec& s, aclrtStream stream, Ev& ev)
{
    if (ev.recorded) {
        CheckAcl(aclrtResetEvent(ev.start, stream), "reset start");
        CheckAcl(aclrtResetEvent(ev.stop, stream), "reset stop");
    }
    CheckAcl(aclrtRecordEvent(ev.start, stream), "record start");
    fn((GM_ADDR)b.x, dg, (GM_ADDR)b.residual, dg, (GM_ADDR)b.gamma, pg, (GM_ADDR)b.bias, pg,
       (GM_ADDR)out, dg, kCoreCount, stream, kEpsilon);
    CheckAcl(aclrtRecordEvent(ev.stop, stream), "record stop");
    CheckAcl(aclrtSynchronizeEvent(ev.stop), "sync stop");
    ev.recorded = true;
    float ms = 0.0f;
    CheckAcl(aclrtEventElapsedTime(&ms, ev.start, ev.stop), "elapsed");
    return static_cast<double>(ms) * 1000.0;
}

void RunCase(const CaseSpec& s, aclrtStream stream, Ev& ev)
{
    const size_t rows = s.rows, width = s.width;
    const size_t n = rows * width;
    const size_t eb = s.dtype == 0 ? 4 : 2;
    const size_t dataBytes = n * eb, paramBytes = width * eb;
    std::vector<uint8_t> x(dataBytes), res(dataBytes), gamma(paramBytes), bias(paramBytes);
    std::vector<uint8_t> outA(dataBytes), outB(dataBytes);
    for (size_t i = 0; i < n; ++i) {
        Encode(x.data(), i, 0.19f * std::sin((float)((i * 17) % 997) * 0.013f), s.dtype);
        Encode(res.data(), i, 0.11f * std::cos((float)((i * 29) % 991) * 0.017f), s.dtype);
    }
    for (size_t c = 0; c < width; ++c) {
        Encode(gamma.data(), c, 0.85f + (float)(c % 23) * 0.002f, s.dtype);
        Encode(bias.data(), c, -0.025f + (float)(c % 19) * 0.001f, s.dtype);
    }
    Buf b;
    CheckAcl(aclrtMalloc(&b.x, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "mx");
    CheckAcl(aclrtMalloc(&b.residual, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "mr");
    CheckAcl(aclrtMalloc(&b.gamma, paramBytes, ACL_MEM_MALLOC_HUGE_FIRST), "mg");
    CheckAcl(aclrtMalloc(&b.bias, paramBytes, ACL_MEM_MALLOC_HUGE_FIRST), "mb");
    CheckAcl(aclrtMalloc(&b.output, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST), "mo");
    CheckAcl(aclrtMemcpy(b.x, dataBytes, x.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "cx");
    CheckAcl(aclrtMemcpy(b.residual, dataBytes, res.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE), "cr");
    CheckAcl(aclrtMemcpy(b.gamma, paramBytes, gamma.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "cg");
    CheckAcl(aclrtMemcpy(b.bias, paramBytes, bias.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE), "cb");

    const int64_t ds[2] = {s.rows, s.width};
    const int64_t ps[1] = {s.width};
    const TensorInfo di{ds, 2, s.dtype}, pi{ps, 1, s.dtype};
    const TensorGroupInfo dg{&di, 1}, pg{&pi, 1};

    // Warm both variants together so neither is cold.
    for (int i = 0; i < kWarmup; ++i) {
        run_kernel_full((GM_ADDR)b.x, dg, (GM_ADDR)b.residual, dg, (GM_ADDR)b.gamma, pg,
                        (GM_ADDR)b.bias, pg, (GM_ADDR)b.output, dg, kCoreCount, stream, kEpsilon);
        CheckAcl(aclrtSynchronizeStream(stream), "warm full");
        run_kernel_p1only((GM_ADDR)b.x, dg, (GM_ADDR)b.residual, dg, (GM_ADDR)b.gamma, pg,
                          (GM_ADDR)b.bias, pg, (GM_ADDR)b.output, dg, kCoreCount, stream, kEpsilon);
        CheckAcl(aclrtSynchronizeStream(stream), "warm p1");
    }

    // Interleave: full, p1only alternating so drift cancels.
    std::vector<double> fullT, p1T, diffT;
    for (int i = 0; i < kSamples; ++i) {
        const bool fullFirst = (i % 2) == 0;
        double tf, tp;
        if (fullFirst) {
            tf = Measure(run_kernel_full, b.output, b, dg, pg, s, stream, ev);
            tp = Measure(run_kernel_p1only, b.output, b, dg, pg, s, stream, ev);
        } else {
            tp = Measure(run_kernel_p1only, b.output, b, dg, pg, s, stream, ev);
            tf = Measure(run_kernel_full, b.output, b, dg, pg, s, stream, ev);
        }
        fullT.push_back(tf);
        p1T.push_back(tp);
        diffT.push_back(tf - tp);
        std::printf("SEG_SAMPLE case=%s i=%02d order=%s full_us=%.3f p1only_us=%.3f pass2_us=%.3f\n",
                    s.name, i, fullFirst ? "F,P" : "P,F", tf, tp, tf - tp);
    }
    const double fmed = Median(fullT), pmed = Median(p1T), dmed = Median(diffT);
    const double p1share = fmed > 0 ? pmed / fmed : 0.0;
    const double p2share = fmed > 0 ? dmed / fmed : 0.0;
    std::printf("SEG_RESULT case=%s rows=%d width=%d full_median_us=%.3f p1only_median_us=%.3f pass2_median_us=%.3f p1_share=%.4f p2_share=%.4f samples=%d\n",
                s.name, s.rows, s.width, fmed, pmed, dmed, p1share, p2share, kSamples);

    CheckAcl(aclrtFree(b.output), "fo");
    CheckAcl(aclrtFree(b.bias), "fb");
    CheckAcl(aclrtFree(b.gamma), "fg");
    CheckAcl(aclrtFree(b.residual), "fr");
    CheckAcl(aclrtFree(b.x), "fx");
}

}  // namespace

int main(int argc, char** argv)
{
    const int device = argc > 1 ? std::atoi(argv[1]) : 4;
    CheckAcl(aclInit(nullptr), "aclInit");
    CheckAcl(aclrtSetDevice(device), "setDevice");
    aclrtStream stream = nullptr;
    CheckAcl(aclrtCreateStream(&stream), "createStream");
    Ev ev;
    CheckAcl(aclrtCreateEvent(&ev.start), "evStart");
    CheckAcl(aclrtCreateEvent(&ev.stop), "evStop");

    const CaseSpec cases[] = {
        {"seg-2x32768", 0, 2, 32768},
        {"seg-8x32768", 0, 8, 32768},
        {"seg-16x32768", 0, 16, 32768},
        {"seg-8x16384", 0, 8, 16384},
        {"seg-2x16384", 0, 2, 16384},
    };
    for (const CaseSpec& s : cases) {
        RunCase(s, stream, ev);
    }

    CheckAcl(aclrtDestroyEvent(ev.stop), "dStop");
    CheckAcl(aclrtDestroyEvent(ev.start), "dStart");
    CheckAcl(aclrtDestroyStream(stream), "dStream");
    CheckAcl(aclrtResetDevice(device), "resetDevice");
    CheckAcl(aclFinalize(), "aclFinalize");
    std::printf("SEG_RUNNER_COMPLETE device=%d cases=5 warmup=%d samples=%d method=DEVICE_EVENT subtraction=full_minus_p1only\n",
                device, kWarmup, kSamples);
    return 0;
}
