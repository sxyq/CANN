#include <acl/acl.h>

#include <algorithm>
#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <vector>

namespace {

constexpr uint64_t kRows = 1;
constexpr uint64_t kWidth = 1024;
constexpr float kEpsilon = 1.0e-5f;

struct RmsNormBiasTiling {
    uint64_t outer;
    uint64_t width;
    uint32_t blockNum;
    float epsilon;
    float invWidth;
};

void CheckAcl(aclError status, const char* expression)
{
    if (status != ACL_SUCCESS) {
        const char* detail = aclGetRecentErrMsg();
        std::cerr << expression << " failed: aclError=" << status;
        if (detail != nullptr) {
            std::cerr << " detail=" << detail;
        }
        std::cerr << std::endl;
        std::exit(3);
    }
}

float InputValue(size_t index, uint32_t stream)
{
    const int32_t centered = static_cast<int32_t>((index * (stream * 19u + 23u) + stream * 71u) % 2001u) - 1000;
    const float value = static_cast<float>(centered) / 2500.0f;
    if (stream == 2) {
        return 0.75f + value * 0.25f;
    }
    if (stream == 3) {
        return value * 0.15f;
    }
    return value * 0.5f;
}

}  // namespace

int main(int argc, char** argv)
{
    if (argc != 2) {
        std::cerr << "usage: correctness_runner <device_wrapper.alink>\n";
        return 2;
    }
    std::cout << "SMOKE dtype=FP32 rows=1 width=1024 blocks=1 warmup=0 repeats=0 kernel=rms_norm_bias_fp32_kernel"
              << " artifact=" << argv[1] << std::endl;

    CheckAcl(aclInit(nullptr), "aclInit");
    CheckAcl(aclrtSetDevice(0), "aclrtSetDevice(0)");
    aclrtStream stream = nullptr;
    CheckAcl(aclrtCreateStream(&stream), "aclrtCreateStream");
    aclrtBinHandle binary = nullptr;
    CheckAcl(aclrtBinaryLoadFromFile(argv[1], nullptr, &binary), "aclrtBinaryLoadFromFile");
    aclrtFuncHandle function = nullptr;
    CheckAcl(aclrtBinaryGetFunction(binary, "rms_norm_bias_fp32_kernel", &function), "aclrtBinaryGetFunction");

    const size_t elements = static_cast<size_t>(kRows * kWidth);
    const size_t bytes = elements * sizeof(float);
    std::vector<float> xHost(elements), residualHost(elements), gammaHost(kWidth), biasHost(kWidth), outputHost(elements, 0.0f);
    for (size_t i = 0; i < elements; ++i) {
        xHost[i] = InputValue(i, 0);
        residualHost[i] = InputValue(i, 1);
    }
    for (size_t i = 0; i < kWidth; ++i) {
        gammaHost[i] = InputValue(i, 2);
        biasHost[i] = InputValue(i, 3);
    }

    uint8_t* xDevice = nullptr;
    uint8_t* residualDevice = nullptr;
    uint8_t* gammaDevice = nullptr;
    uint8_t* biasDevice = nullptr;
    uint8_t* outputDevice = nullptr;
    uint8_t* tilingDevice = nullptr;
    CheckAcl(aclrtMalloc(reinterpret_cast<void**>(&xDevice), bytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(x)");
    CheckAcl(aclrtMalloc(reinterpret_cast<void**>(&residualDevice), bytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(residual)");
    CheckAcl(aclrtMalloc(reinterpret_cast<void**>(&gammaDevice), bytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(gamma)");
    CheckAcl(aclrtMalloc(reinterpret_cast<void**>(&biasDevice), bytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(bias)");
    CheckAcl(aclrtMalloc(reinterpret_cast<void**>(&outputDevice), bytes, ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(output)");
    RmsNormBiasTiling tiling{kRows, kWidth, 1, kEpsilon, 1.0f / static_cast<float>(kWidth)};
    CheckAcl(aclrtMalloc(reinterpret_cast<void**>(&tilingDevice), sizeof(tiling), ACL_MEM_MALLOC_HUGE_FIRST), "aclrtMalloc(tiling)");
    CheckAcl(aclrtMemcpy(xDevice, bytes, xHost.data(), bytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy x H2D");
    CheckAcl(aclrtMemcpy(residualDevice, bytes, residualHost.data(), bytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy residual H2D");
    CheckAcl(aclrtMemcpy(gammaDevice, bytes, gammaHost.data(), bytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy gamma H2D");
    CheckAcl(aclrtMemcpy(biasDevice, bytes, biasHost.data(), bytes, ACL_MEMCPY_HOST_TO_DEVICE), "copy bias H2D");
    CheckAcl(aclrtMemcpy(tilingDevice, sizeof(tiling), &tiling, sizeof(tiling), ACL_MEMCPY_HOST_TO_DEVICE), "copy tiling H2D");

    void* args[] = {&xDevice, &residualDevice, &gammaDevice, &biasDevice, &outputDevice, &tilingDevice};
    std::cout << "CALL aclrtLaunchKernel blockDim=1 argsSize=" << sizeof(args) << std::endl;
    CheckAcl(aclrtLaunchKernel(function, 1, args, sizeof(args), stream), "aclrtLaunchKernel");
    CheckAcl(aclrtSynchronizeStream(stream), "aclrtSynchronizeStream");
    CheckAcl(aclrtMemcpy(outputHost.data(), bytes, outputDevice, bytes, ACL_MEMCPY_DEVICE_TO_HOST), "copy output D2H");

    double sumSquares = 0.0;
    for (size_t i = 0; i < elements; ++i) {
        const double combined = static_cast<double>(xHost[i]) + residualHost[i];
        sumSquares += combined * combined;
    }
    const double inverseRms = 1.0 / std::sqrt(sumSquares / static_cast<double>(kWidth) + kEpsilon);
    uint64_t matched = 0;
    float maxAbs = 0.0f;
    for (size_t i = 0; i < elements; ++i) {
        const double golden = (static_cast<double>(xHost[i]) + residualHost[i]) * inverseRms * gammaHost[i] + biasHost[i];
        const float expected = static_cast<float>(golden);
        const float absError = std::abs(outputHost[i] - expected);
        maxAbs = std::max(maxAbs, absError);
        if (absError <= 1.0e-5f + 1.0e-3f * std::abs(expected)) {
            ++matched;
        }
    }
    const double ratio = static_cast<double>(matched) / static_cast<double>(elements);
    const bool pass = ratio >= 0.99 && maxAbs <= 1.0e-2f;
    std::cout << "RESULT matched_ratio=" << std::fixed << std::setprecision(8) << ratio
              << " max_abs_error=" << std::scientific << maxAbs
              << " status=" << (pass ? "PASS" : "FAIL") << std::endl;

    CheckAcl(aclrtFree(xDevice), "aclrtFree(x)");
    CheckAcl(aclrtFree(residualDevice), "aclrtFree(residual)");
    CheckAcl(aclrtFree(gammaDevice), "aclrtFree(gamma)");
    CheckAcl(aclrtFree(biasDevice), "aclrtFree(bias)");
    CheckAcl(aclrtFree(outputDevice), "aclrtFree(output)");
    CheckAcl(aclrtFree(tilingDevice), "aclrtFree(tiling)");
    CheckAcl(aclrtBinaryUnLoad(binary), "aclrtBinaryUnLoad");
    CheckAcl(aclrtDestroyStream(stream), "aclrtDestroyStream");
    CheckAcl(aclrtResetDevice(0), "aclrtResetDevice(0)");
    CheckAcl(aclFinalize(), "aclFinalize");
    return pass ? 0 : 1;
}
