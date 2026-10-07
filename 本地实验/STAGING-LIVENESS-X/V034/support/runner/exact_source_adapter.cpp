#include <acl/acl.h>

#include <algorithm>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>

#include "aclrtlaunch_triple_chevrons_func.h"
#include "kernel_entry_abi.hpp"

using half = __fp16;
using bfloat16_t = __bf16;

namespace {

uint32_t LaunchFp32(KernelGmAddr x, KernelGmAddr residual, KernelGmAddr gamma,
                    KernelGmAddr bias, KernelGmAddr output, uint64_t rowCount,
                    uint64_t rowWidth, uint32_t blockCount, float invRowWidth,
                    float epsilon, aclrtStream stream)
{
    return aclrtlaunch_add_rms_norm_bias_custom<float>(
        blockCount, stream, x, residual, gamma, bias, output, rowCount, rowWidth,
        blockCount, invRowWidth, epsilon);
}

uint32_t LaunchFp16(KernelGmAddr x, KernelGmAddr residual, KernelGmAddr gamma,
                    KernelGmAddr bias, KernelGmAddr output, uint64_t rowCount,
                    uint64_t rowWidth, uint32_t blockCount, float invRowWidth,
                    float epsilon, aclrtStream stream)
{
    return aclrtlaunch_add_rms_norm_bias_custom<half>(
        blockCount, stream, x, residual, gamma, bias, output, rowCount, rowWidth,
        blockCount, invRowWidth, epsilon);
}

uint32_t LaunchBf16(KernelGmAddr x, KernelGmAddr residual, KernelGmAddr gamma,
                    KernelGmAddr bias, KernelGmAddr output, uint64_t rowCount,
                    uint64_t rowWidth, uint32_t blockCount, float invRowWidth,
                    float epsilon, aclrtStream stream)
{
    return aclrtlaunch_add_rms_norm_bias_custom<bfloat16_t>(
        blockCount, stream, x, residual, gamma, bias, output, rowCount, rowWidth,
        blockCount, invRowWidth, epsilon);
}

bool HasFirstTensor(const TensorGroupInfo& group)
{
    return group.tensors != nullptr && group.numTensors >= 1 &&
           group.tensors[0].shape != nullptr;
}

void CheckLaunch(uint32_t status)
{
    if (status != ACL_SUCCESS) {
        throw std::runtime_error("generated exact-kernel launch failed: " +
                                 std::to_string(status));
    }
}

}  // namespace

extern "C" void run_kernel(
    KernelGmAddr x, const TensorGroupInfo& infoX,
    KernelGmAddr residual, const TensorGroupInfo& infoResidual,
    KernelGmAddr gamma, const TensorGroupInfo& infoGamma,
    KernelGmAddr bias, const TensorGroupInfo& infoBias,
    KernelGmAddr output, const TensorGroupInfo& infoOutput,
    int64_t availableCoreNum, aclrtStream stream, float epsilon)
{
    if (x == nullptr || residual == nullptr || gamma == nullptr || bias == nullptr ||
        output == nullptr || !HasFirstTensor(infoX) || !HasFirstTensor(infoResidual) ||
        !HasFirstTensor(infoGamma) || !HasFirstTensor(infoBias) || !HasFirstTensor(infoOutput)) {
        return;
    }

    const TensorInfo& xInfo = infoX.tensors[0];
    const TensorInfo& residualInfo = infoResidual.tensors[0];
    const TensorInfo& gammaInfo = infoGamma.tensors[0];
    const TensorInfo& biasInfo = infoBias.tensors[0];
    const TensorInfo& outputInfo = infoOutput.tensors[0];
    if (xInfo.numDims < 1 || residualInfo.numDims != xInfo.numDims ||
        outputInfo.numDims != xInfo.numDims || gammaInfo.numDims != 1 ||
        biasInfo.numDims != 1 || xInfo.dtype != residualInfo.dtype ||
        xInfo.dtype != gammaInfo.dtype || xInfo.dtype != biasInfo.dtype ||
        xInfo.dtype != outputInfo.dtype) {
        return;
    }

    const int64_t widthSigned = xInfo.shape[xInfo.numDims - 1];
    if (widthSigned <= 0 || gammaInfo.shape[0] != widthSigned ||
        biasInfo.shape[0] != widthSigned) {
        return;
    }

    uint64_t rowCount = 1;
    for (int64_t dim = 0; dim < xInfo.numDims - 1; ++dim) {
        const int64_t extent = xInfo.shape[dim];
        if (extent <= 0 || residualInfo.shape[dim] != extent ||
            outputInfo.shape[dim] != extent ||
            rowCount > std::numeric_limits<uint64_t>::max() /
                           static_cast<uint64_t>(extent)) {
            return;
        }
        rowCount *= static_cast<uint64_t>(extent);
    }
    if (residualInfo.shape[xInfo.numDims - 1] != widthSigned ||
        outputInfo.shape[xInfo.numDims - 1] != widthSigned || rowCount == 0) {
        return;
    }

    const uint64_t rowWidth = static_cast<uint64_t>(widthSigned);
    if (rowCount > std::numeric_limits<uint64_t>::max() / rowWidth) return;

    uint64_t requestedBlocks = availableCoreNum > 0
                                   ? static_cast<uint64_t>(availableCoreNum)
                                   : 1;
    requestedBlocks = std::min(requestedBlocks, rowCount);
    requestedBlocks = std::min(requestedBlocks,
                               static_cast<uint64_t>(std::numeric_limits<uint32_t>::max()));
    const uint32_t blockCount = static_cast<uint32_t>(requestedBlocks);
    const float invRowWidth = 1.0f / static_cast<float>(widthSigned);

    uint32_t status = ACL_SUCCESS;
    switch (xInfo.dtype) {
        case 0:
            status = LaunchFp32(x, residual, gamma, bias, output, rowCount, rowWidth,
                                blockCount, invRowWidth, epsilon, stream);
            break;
        case 1:
            status = LaunchFp16(x, residual, gamma, bias, output, rowCount, rowWidth,
                                blockCount, invRowWidth, epsilon, stream);
            break;
        case 2:
            status = LaunchBf16(x, residual, gamma, bias, output, rowCount, rowWidth,
                                blockCount, invRowWidth, epsilon, stream);
            break;
        default:
            return;
    }
    CheckLaunch(status);
}
