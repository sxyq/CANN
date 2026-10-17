#include <acl/acl.h>
#include <algorithm>
#include <array>
#include <chrono>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <string>
#include <vector>

#include "local_types.h"
#ifndef ACLRT_STREAM_DEFINED
#define ACLRT_STREAM_DEFINED
typedef void* aclrtStream;
#endif
#include UB_LIVENESS_SUBMISSION

#define ACL_CHECK(expr)                                                         \
    do {                                                                        \
        const aclError result = (expr);                                         \
        if (result != ACL_SUCCESS) {                                            \
            std::fprintf(stderr, "ACL failure %s=%d at %s:%d\n", #expr, result, \
                         __FILE__, __LINE__);                                   \
            return 1;                                                           \
        }                                                                       \
    } while (0)

namespace {
constexpr float kEpsilon = 1.0e-5f;

float InputValue(int64_t index, int multiplier, int offset)
{
    return static_cast<float>((index * multiplier + offset) % 997) / 498.0f - 1.0f;
}

uint16_t FloatToHalfBits(float value)
{
    const aclFloat16 halfValue = aclFloatToFloat16(value);
    uint16_t bits = 0;
    std::memcpy(&bits, &halfValue, sizeof(bits));
    return bits;
}

float HalfBitsToFloat(uint16_t bits)
{
    aclFloat16 halfValue;
    std::memcpy(&halfValue, &bits, sizeof(bits));
    return aclFloat16ToFloat(halfValue);
}

uint16_t FloatToBf16Bits(float value)
{
    uint32_t bits = 0;
    std::memcpy(&bits, &value, sizeof(bits));
    bits += 0x7fffU + ((bits >> 16) & 1U);
    return static_cast<uint16_t>(bits >> 16);
}

float Bf16BitsToFloat(uint16_t bits)
{
    const uint32_t wide = static_cast<uint32_t>(bits) << 16;
    float value = 0.0f;
    std::memcpy(&value, &wide, sizeof(value));
    return value;
}

void StoreValue(std::vector<uint8_t>& data, size_t index, int dtype, float value)
{
    if (dtype == 0) {
        std::memcpy(data.data() + index * sizeof(float), &value, sizeof(value));
        return;
    }
    const uint16_t bits = dtype == 1 ? FloatToHalfBits(value) : FloatToBf16Bits(value);
    std::memcpy(data.data() + index * sizeof(bits), &bits, sizeof(bits));
}

float LoadValue(const std::vector<uint8_t>& data, size_t index, int dtype)
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
}

// usage: device_id rows width dtype mode(correct|probe) prefix [rank]
// mode=correct runs once and reports bad count; mode=probe warmups+repeats timing only
int main(int argc, char** argv)
{
    if (argc < 6) {
        std::fprintf(stderr,
                     "usage: %s device_id rows width dtype correct|probe prefix [rank]\n",
                     argv[0]);
        return 2;
    }
    const int deviceId = std::atoi(argv[1]);
    const int64_t rows = std::strtoll(argv[2], nullptr, 10);
    const int64_t width = std::strtoll(argv[3], nullptr, 10);
    const int dtype = std::atoi(argv[4]);
    const std::string mode = argv[5];
    const char* prefix = argv[6] != nullptr ? argv[6] : "out";
    const int rank = argc > 7 ? std::atoi(argv[7]) : 2;
    if (rows < 1 || width < 64 || width > 32768 || dtype < 0 || dtype > 2) {
        std::fprintf(stderr, "unsupported shape\n");
        return 2;
    }

    const size_t elements = static_cast<size_t>(rows * width);
    const size_t elementBytes = dtype == 0 ? sizeof(float) : sizeof(uint16_t);
    const size_t dataBytes = elements * elementBytes;
    const size_t paramBytes = static_cast<size_t>(width) * elementBytes;

    int64_t availableCoreNum = 0;
    aclrtStream stream = nullptr;
    ACL_CHECK(aclInit(nullptr));
    ACL_CHECK(aclrtSetDevice(deviceId));
    ACL_CHECK(aclrtGetDeviceInfo(deviceId, ACL_DEV_ATTR_VECTOR_CORE_NUM, &availableCoreNum));
    ACL_CHECK(aclrtCreateStream(&stream));

    // Build shape as rank 2/3/4 with product(rows)*width
    int64_t shapeBuf[4] = {rows, width, 1, 1};
    if (rank == 3) {
        // rows = a*b
        int64_t a = rows;
        int64_t b = 1;
        for (int64_t d = 2; d * d <= rows; ++d) {
            if (rows % d == 0) {
                a = d;
                b = rows / d;
                break;
            }
        }
        shapeBuf[0] = a;
        shapeBuf[1] = b;
        shapeBuf[2] = width;
        shapeBuf[3] = 1;
    } else if (rank == 4) {
        int64_t a = rows;
        int64_t b = 1, c = 1;
        // rows = a*b*c with c>=1
        for (int64_t d = 2; d * d <= rows; ++d) {
            if (rows % d == 0) {
                a = d;
                b = rows / d;
                c = 1;
                break;
            }
        }
        shapeBuf[0] = a;
        shapeBuf[1] = b;
        shapeBuf[2] = c;
        shapeBuf[3] = width;
    }
    const int useRank = (rank < 2 || rank > 4) ? 2 : rank;

    int64_t xyShape[4];
    for (int i = 0; i < useRank; ++i) {
        xyShape[i] = shapeBuf[i];
    }
    int64_t paramShape[] = {width};
    TensorInfo xTensor[] = {{xyShape, useRank, dtype}};
    TensorInfo rTensor[] = {{xyShape, useRank, dtype}};
    TensorInfo gTensor[] = {{paramShape, 1, dtype}};
    TensorInfo bTensor[] = {{paramShape, 1, dtype}};
    TensorInfo oTensor[] = {{xyShape, useRank, dtype}};
    TensorGroupInfo infoX = {xTensor, 1}, infoR = {rTensor, 1};
    TensorGroupInfo infoG = {gTensor, 1}, infoB = {bTensor, 1}, infoO = {oTensor, 1};

    std::vector<uint8_t> hostX(dataBytes), hostR(dataBytes), hostG(paramBytes),
        hostB(paramBytes), hostOut(dataBytes, 0xA5);
    void *deviceX = nullptr, *deviceR = nullptr, *deviceG = nullptr, *deviceB = nullptr,
         *deviceOut = nullptr;
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceX), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceR), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceG), paramBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceB), paramBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceOut), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));

    for (size_t i = 0; i < elements; ++i) {
        StoreValue(hostX, i, dtype, InputValue(static_cast<int64_t>(i), 37, 11));
        StoreValue(hostR, i, dtype, InputValue(static_cast<int64_t>(i), 17, 3));
    }
    for (int64_t i = 0; i < width; ++i) {
        StoreValue(hostG, static_cast<size_t>(i), dtype,
                   0.75f + static_cast<float>((i * 13) % 100) / 200.0f);
        StoreValue(hostB, static_cast<size_t>(i), dtype,
                   static_cast<float>((i * 7) % 100) / 400.0f - 0.125f);
    }
    ACL_CHECK(aclrtMemcpy(deviceX, dataBytes, hostX.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceR, dataBytes, hostR.data(), dataBytes, ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceG, paramBytes, hostG.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE));
    ACL_CHECK(aclrtMemcpy(deviceB, paramBytes, hostB.data(), paramBytes, ACL_MEMCPY_HOST_TO_DEVICE));

    auto launch = [&]() {
        run_kernel((GM_ADDR)deviceX, infoX, (GM_ADDR)deviceR, infoR, (GM_ADDR)deviceG, infoG,
                   (GM_ADDR)deviceB, infoB, (GM_ADDR)deviceOut, infoO, availableCoreNum, stream,
                   kEpsilon);
    };

    const int warmups = (mode == "probe") ? 3 : 1;
    const int repeats = (mode == "probe") ? 11 : 1;
    for (int i = 0; i < warmups; ++i) {
        launch();
        ACL_CHECK(aclrtSynchronizeStream(stream));
    }
    std::array<double, 16> samples{};
    if (mode == "probe") {
        for (int i = 0; i < repeats && i < 16; ++i) {
            const auto start = std::chrono::steady_clock::now();
            launch();
            ACL_CHECK(aclrtSynchronizeStream(stream));
            const auto stop = std::chrono::steady_clock::now();
            samples[static_cast<size_t>(i)] =
                std::chrono::duration<double, std::micro>(stop - start).count();
        }
        std::sort(samples.begin(), samples.begin() + repeats);
    }
    ACL_CHECK(aclrtMemcpy(hostOut.data(), dataBytes, deviceOut, dataBytes, ACL_MEMCPY_DEVICE_TO_HOST));

    float maxAbs = 0.0f;
    int bad = 0;
    for (int64_t row = 0; row < rows; ++row) {
        float sum = 0.0f;
        for (int64_t col = 0; col < width; ++col) {
            const size_t i = static_cast<size_t>(row * width + col);
            float u = LoadValue(hostX, i, dtype) + LoadValue(hostR, i, dtype);
            if (dtype == 1) {
                u = HalfBitsToFloat(FloatToHalfBits(u));
            } else if (dtype == 2) {
                u = Bf16BitsToFloat(FloatToBf16Bits(u));
            }
            sum += u * u;
        }
        const float invRms = 1.0f / std::sqrt(sum / static_cast<float>(width) + kEpsilon);
        for (int64_t col = 0; col < width; ++col) {
            const size_t i = static_cast<size_t>(row * width + col);
            float u = LoadValue(hostX, i, dtype) + LoadValue(hostR, i, dtype);
            if (dtype == 1) {
                u = HalfBitsToFloat(FloatToHalfBits(u));
            } else if (dtype == 2) {
                u = Bf16BitsToFloat(FloatToBf16Bits(u));
            }
            float expected = u * invRms * LoadValue(hostG, static_cast<size_t>(col), dtype) +
                             LoadValue(hostB, static_cast<size_t>(col), dtype);
            if (dtype == 1) {
                expected = HalfBitsToFloat(FloatToHalfBits(expected));
            } else if (dtype == 2) {
                expected = Bf16BitsToFloat(FloatToBf16Bits(expected));
            }
            const float actual = LoadValue(hostOut, i, dtype);
            const float error = std::fabs(actual - expected);
            maxAbs = std::max(maxAbs, error);
            const float atol = dtype == 0 ? 1.0e-3f : (dtype == 1 ? 2.0e-2f : 5.0e-2f);
            const float rtol = dtype == 0 ? 1.0e-3f : (dtype == 1 ? 2.0e-2f : 5.0e-2f);
            if (!std::isfinite(actual) || error > atol + rtol * std::fabs(expected)) {
                if (bad < 5) {
                    std::fprintf(stderr,
                                 " mismatch r=%lld c=%lld actual=%g expected=%g err=%g\n",
                                 static_cast<long long>(row), static_cast<long long>(col), actual,
                                 expected, error);
                }
                ++bad;
            }
        }
    }

    char timingPath[512];
    std::snprintf(timingPath, sizeof(timingPath), "%s.tsv", prefix);
    std::ofstream timing(timingPath);
    timing << "device\tdtype\trows\twidth\trank\tmode\tmedian_us\tmax_abs\tbad\n"
           << deviceId << '\t' << dtype << '\t' << rows << '\t' << width << '\t' << useRank << '\t'
           << mode << '\t'
           << (mode == "probe" ? samples[repeats / 2] : 0.0) << '\t' << maxAbs << '\t' << bad
           << '\n';
    std::printf(
        "device=%d dtype=%d rows=%lld width=%lld rank=%d mode=%s median_us=%.3f max_abs=%.7g bad=%d\n",
        deviceId, dtype, static_cast<long long>(rows), static_cast<long long>(width), useRank,
        mode.c_str(), mode == "probe" ? samples[repeats / 2] : 0.0, maxAbs, bad);

    ACL_CHECK(aclrtFree(deviceX));
    ACL_CHECK(aclrtFree(deviceR));
    ACL_CHECK(aclrtFree(deviceG));
    ACL_CHECK(aclrtFree(deviceB));
    ACL_CHECK(aclrtFree(deviceOut));
    ACL_CHECK(aclrtDestroyStream(stream));
    ACL_CHECK(aclFinalize());
    return bad == 0 ? 0 : 3;
}
