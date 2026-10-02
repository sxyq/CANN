#include <acl/acl.h>
#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <vector>

#include "local_types.h"
#include STORE_V001_SOURCE

#define ACL_CHECK(expr) do { \
    const aclError result = (expr); \
    if (result != ACL_SUCCESS) { \
        std::fprintf(stderr, "ACL failure %s=%d at %s:%d\n", #expr, result, __FILE__, __LINE__); \
        return 1; \
    } \
} while (0)

namespace {
constexpr float kEpsilon = 1.0e-5f;
constexpr double kGrossRel = 0.05;

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
    std::memcpy(&bits, &value, sizeof(value));
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

int main(int argc, char** argv)
{
    if (argc != 7) {
        std::fprintf(stderr,
                     "usage: %s device_id rows width dtype output_path result_path\n",
                     argv[0]);
        return 2;
    }

    const int deviceId = std::atoi(argv[1]);
    const int64_t rows = std::strtoll(argv[2], nullptr, 10);
    const int64_t width = std::strtoll(argv[3], nullptr, 10);
    const int dtype = std::atoi(argv[4]);
    const char* outputPath = argv[5];
    const char* resultPath = argv[6];
    if (rows < 1 || width < 64 || width > 32768 || dtype < 0 || dtype > 2) {
        std::fprintf(stderr, "unsupported probe shape rows=%lld width=%lld dtype=%d\n",
                     static_cast<long long>(rows), static_cast<long long>(width), dtype);
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
    void *deviceX = nullptr, *deviceR = nullptr, *deviceG = nullptr,
         *deviceB = nullptr, *deviceOut = nullptr;
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceX), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceR), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceG), paramBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceB), paramBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMalloc(reinterpret_cast<void**>(&deviceOut), dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    ACL_CHECK(aclrtMemset(deviceOut, dataBytes, 0, dataBytes));

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

    run_kernel((GM_ADDR)deviceX, infoX, (GM_ADDR)deviceR, infoR,
               (GM_ADDR)deviceG, infoG, (GM_ADDR)deviceB, infoB,
               (GM_ADDR)deviceOut, infoO, availableCoreNum, stream, kEpsilon);
    ACL_CHECK(aclrtSynchronizeStream(stream));
    ACL_CHECK(aclrtMemcpy(hostOut.data(), dataBytes, deviceOut, dataBytes,
                          ACL_MEMCPY_DEVICE_TO_HOST));

    double atol = 1.0e-4, rtol = 1.0e-4;
    if (dtype == 1) { atol = 1.0e-3; rtol = 1.0e-3; }
    if (dtype == 2) { atol = 2.0e-2; rtol = 2.0e-2; }
    double maxAbs = 0.0;
    int bad = 0;
    int grossBad = 0;
    const double invWidth = 1.0 / static_cast<double>(width);
    for (int64_t row = 0; row < rows; ++row) {
        double sum = 0.0;
        for (int64_t col = 0; col < width; ++col) {
            const size_t i = static_cast<size_t>(row * width + col);
            const double x = LoadValue(hostX, i, dtype);
            const double r = LoadValue(hostR, i, dtype);
            sum += (x + r) * (x + r);
        }
        const double invRms = 1.0 / std::sqrt(sum * invWidth + kEpsilon);
        for (int64_t col = 0; col < width; ++col) {
            const size_t i = static_cast<size_t>(row * width + col);
            const double u = static_cast<double>(LoadValue(hostX, i, dtype)) +
                             static_cast<double>(LoadValue(hostR, i, dtype));
            const double expected = u * invRms *
                static_cast<double>(LoadValue(hostG, static_cast<size_t>(col), dtype)) +
                static_cast<double>(LoadValue(hostB, static_cast<size_t>(col), dtype));
            const double actual = LoadValue(hostOut, i, dtype);
            const double error = std::fabs(actual - expected);
            maxAbs = std::max(maxAbs, error);
            if (!std::isfinite(actual) || error > atol + rtol * std::fabs(expected)) {
                ++bad;
            }
            if (!std::isfinite(actual) ||
                error > kGrossRel * (std::fabs(expected) + 1.0e-3)) {
                ++grossBad;
            }
        }
    }

    std::ofstream output(outputPath, std::ios::binary);
    output.write(reinterpret_cast<const char*>(hostOut.data()), dataBytes);
    output.close();
    if (!output.good()) {
        std::fprintf(stderr, "cannot write %s\n", outputPath);
        return 1;
    }
    std::ofstream result(resultPath);
    result << "variant\tdevice\trows\twidth\tdtype\tmax_abs\tbad\tgross_bad\telements\n"
           << STORE_V001_VARIANT << '\t' << deviceId << '\t' << rows << '\t'
           << width << '\t' << dtype << '\t' << maxAbs << '\t' << bad << '\t'
           << grossBad << '\t' << elements << '\n';
    result.close();
    if (!result.good()) {
        std::fprintf(stderr, "cannot write %s\n", resultPath);
        return 1;
    }

    std::printf("variant=%s device=%d dtype=%d rows=%lld width=%lld max_abs=%.9g bad=%d gross_bad=%d\n",
                STORE_V001_VARIANT, deviceId, dtype, static_cast<long long>(rows),
                static_cast<long long>(width), maxAbs, bad, grossBad);

    ACL_CHECK(aclrtFree(deviceX)); ACL_CHECK(aclrtFree(deviceR));
    ACL_CHECK(aclrtFree(deviceG)); ACL_CHECK(aclrtFree(deviceB));
    ACL_CHECK(aclrtFree(deviceOut));
    ACL_CHECK(aclrtDestroyStream(stream));
    ACL_CHECK(aclFinalize());
    return (bad == 0 && grossBad == 0) ? 0 : 3;
}
