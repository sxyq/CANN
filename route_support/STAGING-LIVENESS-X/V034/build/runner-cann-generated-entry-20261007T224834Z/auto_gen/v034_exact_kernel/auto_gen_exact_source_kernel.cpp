#ifndef __EXACT_SOURCE_KERNEL__KERNEL_FUN_H__
#define __EXACT_SOURCE_KERNEL__KERNEL_FUN_H__

#undef __global__
#define __global__ inline
#define add_rms_norm_bias_custom add_rms_norm_bias_custom_origin
#include "/home/data4t2/lelinfeng/cann-staging-liveness-x/route_support/STAGING-LIVENESS-X/V034/exact_source_kernel.cpp"

#undef add_rms_norm_bias_custom
#undef __global__
#if ASCENDC_CPU_DEBUG
#define __global__
#else
#define __global__ __attribute__((cce_kernel))
#endif

#ifndef ONE_CORE_DUMP_SIZE
#define ONE_CORE_DUMP_SIZE 1048576 * 1
#endif

extern "C" __global__ [aicore] void auto_gen_add_rms_norm_bias_custom_template_0_kernel(
__attribute__((cce_global)) uint8_t* x, __attribute__((cce_global)) uint8_t* residual, __attribute__((cce_global)) uint8_t* gamma, __attribute__((cce_global)) uint8_t* bias, __attribute__((cce_global)) uint8_t* output, unsigned long rowCount, unsigned long rowWidth, unsigned int blockCount, float invRowWidth, float epsilon, GM_ADDR overflow_status) {
#if defined(HAVE_WORKSPACE)
    GM_ADDR workspace_param;
    GM_ADDR workspace_usr;
#if defined(HAVE_TILING)
    workspace_param = invRowWidth;
#else
    workspace_param = epsilon;
#endif
    AscendC::SetSysWorkspaceForce(workspace_param);
    workspace_usr = AscendC::GetUserWorkspace(workspace_param);
#if defined(HAVE_TILING)
    invRowWidth = workspace_usr;
#else
    epsilon = workspace_usr;
#endif
#endif
    add_rms_norm_bias_custom_origin<float>(x, residual, gamma, bias, output, rowCount, rowWidth, blockCount, invRowWidth, epsilon);
#if defined(ASCENDC_DUMP) && defined(ASCENDC_DEBUG)
    AscendC::WriteBackOverflow(overflow_status);
#endif
#if defined(__DAV_C310__) || defined(__DAV_310R6__)
    pipe_barrier(PIPE_ALL);
    dsb(mem_dsb_t::DSB_ALL);
    dci();
#endif
}

static const struct FunLevelKType add_rms_norm_bias_custom_section __attribute__ ((used, section (".ascend.meta.add_rms_norm_bias_custom_0"))) = { { {F_TYPE_KTYPE, sizeof(unsigned int)}, K_TYPE_AIV} };
extern "C" __global__ [aicore] void auto_gen_add_rms_norm_bias_custom_template_1_kernel(
__attribute__((cce_global)) uint8_t* x, __attribute__((cce_global)) uint8_t* residual, __attribute__((cce_global)) uint8_t* gamma, __attribute__((cce_global)) uint8_t* bias, __attribute__((cce_global)) uint8_t* output, unsigned long rowCount, unsigned long rowWidth, unsigned int blockCount, float invRowWidth, float epsilon, GM_ADDR overflow_status) {
#if defined(HAVE_WORKSPACE)
    GM_ADDR workspace_param;
    GM_ADDR workspace_usr;
#if defined(HAVE_TILING)
    workspace_param = invRowWidth;
#else
    workspace_param = epsilon;
#endif
    AscendC::SetSysWorkspaceForce(workspace_param);
    workspace_usr = AscendC::GetUserWorkspace(workspace_param);
#if defined(HAVE_TILING)
    invRowWidth = workspace_usr;
#else
    epsilon = workspace_usr;
#endif
#endif
    add_rms_norm_bias_custom_origin<half>(x, residual, gamma, bias, output, rowCount, rowWidth, blockCount, invRowWidth, epsilon);
#if defined(ASCENDC_DUMP) && defined(ASCENDC_DEBUG)
    AscendC::WriteBackOverflow(overflow_status);
#endif
#if defined(__DAV_C310__) || defined(__DAV_310R6__)
    pipe_barrier(PIPE_ALL);
    dsb(mem_dsb_t::DSB_ALL);
    dci();
#endif
}

extern "C" __global__ [aicore] void auto_gen_add_rms_norm_bias_custom_template_2_kernel(
__attribute__((cce_global)) uint8_t* x, __attribute__((cce_global)) uint8_t* residual, __attribute__((cce_global)) uint8_t* gamma, __attribute__((cce_global)) uint8_t* bias, __attribute__((cce_global)) uint8_t* output, unsigned long rowCount, unsigned long rowWidth, unsigned int blockCount, float invRowWidth, float epsilon, GM_ADDR overflow_status) {
#if defined(HAVE_WORKSPACE)
    GM_ADDR workspace_param;
    GM_ADDR workspace_usr;
#if defined(HAVE_TILING)
    workspace_param = invRowWidth;
#else
    workspace_param = epsilon;
#endif
    AscendC::SetSysWorkspaceForce(workspace_param);
    workspace_usr = AscendC::GetUserWorkspace(workspace_param);
#if defined(HAVE_TILING)
    invRowWidth = workspace_usr;
#else
    epsilon = workspace_usr;
#endif
#endif
    add_rms_norm_bias_custom_origin<__bf16>(x, residual, gamma, bias, output, rowCount, rowWidth, blockCount, invRowWidth, epsilon);
#if defined(ASCENDC_DUMP) && defined(ASCENDC_DEBUG)
    AscendC::WriteBackOverflow(overflow_status);
#endif
#if defined(__DAV_C310__) || defined(__DAV_310R6__)
    pipe_barrier(PIPE_ALL);
    dsb(mem_dsb_t::DSB_ALL);
    dci();
#endif
}

#endif
