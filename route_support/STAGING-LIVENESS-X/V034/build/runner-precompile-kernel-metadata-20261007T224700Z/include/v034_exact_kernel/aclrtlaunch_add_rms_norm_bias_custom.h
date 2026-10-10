#ifndef HEADER_ACLRTLAUNCH_ADD_RMS_NORM_BIAS_CUSTOM_H
#define HEADER_ACLRTLAUNCH_ADD_RMS_NORM_BIAS_CUSTOM_H
#include "acl/acl_base.h"

#ifndef ACLRT_LAUNCH_KERNEL
#define ACLRT_LAUNCH_KERNEL(kernel_func) aclrtlaunch_##kernel_func
#endif

template<typename T>
uint32_t aclrtlaunch_add_rms_norm_bias_custom(uint32_t blockDim, aclrtStream stream, void* x, void* residual, void* gamma, void* bias, void* output, uint64_t rowCount, uint64_t rowWidth, uint32_t blockCount, float invRowWidth, float epsilon);
#endif
