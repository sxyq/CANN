#include <kernel_operator.h>

#define ASCENDC_CPU_DEBUG 0
#include "submission.asc"
#undef ASCENDC_CPU_DEBUG

extern "C" __global__ __vector__ void route_v034_fp32(
    GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output,
    uint64_t rowCount, uint64_t rowWidth, uint32_t blockCount,
    float invRowWidth, float epsilon)
{
    AddRmsNormBiasKernel<float> op;
    op.Init(x, residual, gamma, bias, output, rowWidth, rowCount, blockCount);
    op.Process(rowCount, rowWidth, blockCount, invRowWidth, epsilon);
}

extern "C" __global__ __vector__ void route_v034_fp16(
    GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output,
    uint64_t rowCount, uint64_t rowWidth, uint32_t blockCount,
    float invRowWidth, float epsilon)
{
    AddRmsNormBiasKernel<half> op;
    op.Init(x, residual, gamma, bias, output, rowWidth, rowCount, blockCount);
    op.Process(rowCount, rowWidth, blockCount, invRowWidth, epsilon);
}

extern "C" __global__ __vector__ void route_v034_bf16(
    GM_ADDR x, GM_ADDR residual, GM_ADDR gamma, GM_ADDR bias, GM_ADDR output,
    uint64_t rowCount, uint64_t rowWidth, uint32_t blockCount,
    float invRowWidth, float epsilon)
{
    AddRmsNormBiasKernel<bfloat16_t> op;
    op.Init(x, residual, gamma, bias, output, rowWidth, rowCount, blockCount);
    op.Process(rowCount, rowWidth, blockCount, invRowWidth, epsilon);
}
