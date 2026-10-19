#include "/home/data4t2/lelinfeng/cann-staging-liveness-x/本地实验/STAGING-LIVENESS-X/V034/support/runner/kernel_entry_abi.hpp"

#ifndef KERNEL_SOURCE_HEADER
#define KERNEL_SOURCE_HEADER \
    "/home/data4t2/lelinfeng/cann-staging-liveness-x/本地实验/STAGING-LIVENESS-X/V034/src/submission.asc"
#endif

#include <kernel_operator.h>
using aclrtStream = void*;

#if defined(__CHECK_FEATURE_AT_PRECOMPILE)
int __enable_feature_for_compile_default = KERNEL_TYPE_AIV_ONLY;
#undef __vector__
#define __vector__ [aicore]
#endif

#if defined(__NPU_DEVICE__) && !defined(ASCENDC_CPU_DEBUG)
#define ASCENDC_CPU_DEBUG 1
#define V034_SUPPORT_DEFINED_CPU_DEBUG 1
#endif
#include KERNEL_SOURCE_HEADER
#ifdef V034_SUPPORT_DEFINED_CPU_DEBUG
#undef ASCENDC_CPU_DEBUG
#undef V034_SUPPORT_DEFINED_CPU_DEBUG
#endif

template __global__ __vector__ void add_rms_norm_bias_custom<float>(
    GM_ADDR, GM_ADDR, GM_ADDR, GM_ADDR, GM_ADDR,
    uint64_t, uint64_t, uint32_t, float, float);

template __global__ __vector__ void add_rms_norm_bias_custom<half>(
    GM_ADDR, GM_ADDR, GM_ADDR, GM_ADDR, GM_ADDR,
    uint64_t, uint64_t, uint32_t, float, float);

template __global__ __vector__ void add_rms_norm_bias_custom<bfloat16_t>(
    GM_ADDR, GM_ADDR, GM_ADDR, GM_ADDR, GM_ADDR,
    uint64_t, uint64_t, uint32_t, float, float);
