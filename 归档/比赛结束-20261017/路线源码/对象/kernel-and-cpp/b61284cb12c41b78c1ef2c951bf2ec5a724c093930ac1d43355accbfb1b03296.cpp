#ifndef __EXACT_SOURCE_KERNEL__KERNEL_FUN_H__
#define __EXACT_SOURCE_KERNEL__KERNEL_FUN_H__

#undef __global__
#define __global__ inline
#include "/home/data4t2/lelinfeng/cann-staging-liveness-x/route_support/STAGING-LIVENESS-X/V034/exact_source_kernel.cpp"

#undef __global__
#if ASCENDC_CPU_DEBUG
#define __global__
#else
#define __global__ __attribute__((cce_kernel))
#endif

#ifndef ONE_CORE_DUMP_SIZE
#define ONE_CORE_DUMP_SIZE 1048576 * 1
#endif

#endif
