#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

namespace {
bool CheckAcl(aclError status, const char* expression)
{
    if (status == ACL_SUCCESS) return true;
    fprintf(stderr, "ACL failure %s status=%d\n", expression, (int)status);
    return false;
}

#define CHECK_ACL(expression) do { if (!CheckAcl((expression), #expression)) return 2; } while (0)

float InputX(size_t index)
{
    return (float)((int)(index % 101) - 50) * 0.01f;
}

float InputResidual(size_t index)
{
    return (float)((int)(index % 37) - 18) * 0.006f;
}

float InputGamma(size_t index)
{
    return 0.8f + (float)(index % 11) * 0.01f;
}

float InputBias(size_t index)
{
    return (float)((int)(index % 17) - 8) * 0.001f;
}

}  // namespace

int main(int argc, char** argv)
{
    if (argc != 7) {
        fprintf(stderr, "usage: %s DEVICE ROWS WIDTH BLOCKS WARMUP REPEATS\n", argv[0]);
        return 2;
    }
    const int32_t device = atoi(argv[1]);
    const uint64_t rows = strtoull(argv[2], nullptr, 10);
    const uint64_t width = strtoull(argv[3], nullptr, 10);
    const int64_t blocks = strtoll(argv[4], nullptr, 10);
    const int32_t warmup = atoi(argv[5]);
    const int32_t repeats = atoi(argv[6]);
    if (rows == 0 || width == 0 || blocks <= 0 || static_cast<uint64_t>(blocks) > rows ||
        warmup < 0 || repeats < 0) {
        fprintf(stderr, "invalid probe arguments\n");
        return 2;
    }

    CHECK_ACL(aclInit(nullptr));
    CHECK_ACL(aclrtSetDevice(device));
    aclrtStream stream = nullptr;
    CHECK_ACL(aclrtCreateStream(&stream));

    const size_t elementCount = (size_t)(rows * width);
    const size_t dataBytes = elementCount * sizeof(float);
    const size_t paramBytes = (size_t)width * sizeof(float);
    float* xHost = (float*)malloc(dataBytes);
    float* residualHost = (float*)malloc(dataBytes);
    float* gammaHost = (float*)malloc(paramBytes);
    float* biasHost = (float*)malloc(paramBytes);
    float* outputHost = (float*)malloc(dataBytes);
    if (xHost == nullptr || residualHost == nullptr || gammaHost == nullptr ||
        biasHost == nullptr || outputHost == nullptr) {
        fprintf(stderr, "host allocation failed\n");
        return 2;
    }
    for (size_t i = 0; i < elementCount; ++i) {
        xHost[i] = InputX(i);
        residualHost[i] = InputResidual(i);
    }
    for (size_t i = 0; i < (size_t)width; ++i) {
        gammaHost[i] = InputGamma(i);
        biasHost[i] = InputBias(i);
    }

    float* xDevice = nullptr;
    float* residualDevice = nullptr;
    float* gammaDevice = nullptr;
    float* biasDevice = nullptr;
    float* outputDevice = nullptr;
    CHECK_ACL(aclrtMalloc((void**)&xDevice, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    CHECK_ACL(aclrtMalloc((void**)&residualDevice, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    CHECK_ACL(aclrtMalloc((void**)&gammaDevice, paramBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    CHECK_ACL(aclrtMalloc((void**)&biasDevice, paramBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    CHECK_ACL(aclrtMalloc((void**)&outputDevice, dataBytes, ACL_MEM_MALLOC_HUGE_FIRST));
    CHECK_ACL(aclrtMemcpy(xDevice, dataBytes, xHost, dataBytes, ACL_MEMCPY_HOST_TO_DEVICE));
    CHECK_ACL(aclrtMemcpy(residualDevice, dataBytes, residualHost, dataBytes, ACL_MEMCPY_HOST_TO_DEVICE));
    CHECK_ACL(aclrtMemcpy(gammaDevice, paramBytes, gammaHost, paramBytes, ACL_MEMCPY_HOST_TO_DEVICE));
    CHECK_ACL(aclrtMemcpy(biasDevice, paramBytes, biasHost, paramBytes, ACL_MEMCPY_HOST_TO_DEVICE));

    const int64_t inputShape[] = {(int64_t)rows, (int64_t)width};
    const int64_t parameterShape[] = {(int64_t)width};
    TensorInfo xTensor[] = {{inputShape, 2, 0}};
    TensorInfo residualTensor[] = {{inputShape, 2, 0}};
    TensorInfo gammaTensor[] = {{parameterShape, 1, 0}};
    TensorInfo biasTensor[] = {{parameterShape, 1, 0}};
    TensorInfo outputTensor[] = {{inputShape, 2, 0}};
    const TensorGroupInfo infoX = {xTensor, 1};
    const TensorGroupInfo infoResidual = {residualTensor, 1};
    const TensorGroupInfo infoGamma = {gammaTensor, 1};
    const TensorGroupInfo infoBias = {biasTensor, 1};
    const TensorGroupInfo infoOutput = {outputTensor, 1};
    constexpr float epsilon = 1.0e-5f;
    const auto invoke = [&]() {
        run_kernel((GM_ADDR)xDevice, infoX,
            (GM_ADDR)residualDevice, infoResidual,
            (GM_ADDR)gammaDevice, infoGamma,
            (GM_ADDR)biasDevice, infoBias,
            (GM_ADDR)outputDevice, infoOutput, blocks, stream, epsilon);
    };

    invoke();
    CHECK_ACL(aclrtSynchronizeStream(stream));
    CHECK_ACL(aclrtMemcpy(outputHost, dataBytes, outputDevice, dataBytes, ACL_MEMCPY_DEVICE_TO_HOST));
    double maxAbsError = 0.0;
    for (uint64_t row = 0; row < rows; ++row) {
        double squareSum = 0.0;
        const size_t rowOffset = (size_t)(row * width);
        for (uint64_t col = 0; col < width; ++col) {
            const double value = static_cast<double>(xHost[rowOffset + col]) + residualHost[rowOffset + col];
            squareSum += value * value;
        }
        const double invRms = 1.0 / sqrt(squareSum / (double)width + epsilon);
        for (uint64_t col = 0; col < width; ++col) {
            const size_t index = rowOffset + (size_t)col;
            const double value = static_cast<double>(xHost[index]) + residualHost[index];
            const double expected = value * invRms * gammaHost[(size_t)col] +
                biasHost[(size_t)col];
            const double error = fabs(expected - outputHost[index]);
            if (error > maxAbsError) maxAbsError = error;
        }
    }
    if (!(maxAbsError < 1.0e-4)) {
        fprintf(stderr, "CORRECTNESS FAIL rows=%llu width=%llu max_abs=%.9g\n",
            (unsigned long long)rows, (unsigned long long)width, maxAbsError);
        return 3;
    }

    if (repeats == 0) {
        printf("CORRECTNESS PASS rows=%llu width=%llu blocks=%lld device=%d max_abs=%.9g\n",
            (unsigned long long)rows, (unsigned long long)width,
            (long long)blocks, device, maxAbsError);
        aclrtFree(xDevice);
        aclrtFree(residualDevice);
        aclrtFree(gammaDevice);
        aclrtFree(biasDevice);
        aclrtFree(outputDevice);
        aclrtDestroyStream(stream);
        aclrtResetDevice(device);
        aclFinalize();
        free(xHost);
        free(residualHost);
        free(gammaHost);
        free(biasHost);
        free(outputHost);
        return 0;
    }

    for (int32_t i = 0; i < warmup; ++i) {
        invoke();
    }
    CHECK_ACL(aclrtSynchronizeStream(stream));
    aclrtEvent start = nullptr;
    aclrtEvent stop = nullptr;
    CHECK_ACL(aclrtCreateEvent(&start));
    CHECK_ACL(aclrtCreateEvent(&stop));
    CHECK_ACL(aclrtRecordEvent(start, stream));
    for (int32_t i = 0; i < repeats; ++i) {
        invoke();
    }
    CHECK_ACL(aclrtRecordEvent(stop, stream));
    CHECK_ACL(aclrtSynchronizeEvent(stop));
    float deviceMs = 0.0f;
    CHECK_ACL(aclrtEventElapsedTime(&deviceMs, start, stop));
    printf("rows=%llu width=%llu blocks=%lld device=%d max_abs=%.9g device_us=%.6f\n",
        (unsigned long long)rows, (unsigned long long)width,
        (long long)blocks, device, maxAbsError,
        (double)deviceMs * 1000.0 / repeats);

    aclrtDestroyEvent(start);
    aclrtDestroyEvent(stop);
    aclrtFree(xDevice);
    aclrtFree(residualDevice);
    aclrtFree(gammaDevice);
    aclrtFree(biasDevice);
    aclrtFree(outputDevice);
    aclrtDestroyStream(stream);
    aclrtResetDevice(device);
    aclFinalize();
    free(xHost);
    free(residualHost);
    free(gammaHost);
    free(biasHost);
    free(outputHost);
    return 0;
}
