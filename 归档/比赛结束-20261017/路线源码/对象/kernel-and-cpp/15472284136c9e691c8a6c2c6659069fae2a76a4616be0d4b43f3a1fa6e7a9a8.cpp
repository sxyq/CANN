#include <kernel_operator.h>

#define ASCENDC_CPU_DEBUG 0
#include KERNEL_SOURCE_HEADER
#undef ASCENDC_CPU_DEBUG

template __global__ __vector__ void add_rms_norm_bias_custom<float>(
    GM_ADDR, GM_ADDR, GM_ADDR, GM_ADDR, GM_ADDR,
    uint64_t, uint64_t, uint32_t, float, float);

template __global__ __vector__ void add_rms_norm_bias_custom<half>(
    GM_ADDR, GM_ADDR, GM_ADDR, GM_ADDR, GM_ADDR,
    uint64_t, uint64_t, uint32_t, float, float);

template __global__ __vector__ void add_rms_norm_bias_custom<bfloat16_t>(
    GM_ADDR, GM_ADDR, GM_ADDR, GM_ADDR, GM_ADDR,
    uint64_t, uint64_t, uint32_t, float, float);
