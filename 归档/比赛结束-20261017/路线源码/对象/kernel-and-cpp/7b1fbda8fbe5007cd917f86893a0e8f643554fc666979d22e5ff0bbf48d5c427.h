#ifndef R31B_V016_PAIRED_RUNNER_HOST_ABI_H
#define R31B_V016_PAIRED_RUNNER_HOST_ABI_H

#include "paired_runner_abi.h"

using R31BRunKernel = void (*)(uint8_t*, const TensorGroupInfo&,
                               uint8_t*, const TensorGroupInfo&,
                               uint8_t*, const TensorGroupInfo&,
                               uint8_t*, const TensorGroupInfo&,
                               uint8_t*, const TensorGroupInfo&,
                               int64_t, aclrtStream, float);

extern "C" void run_kernel_v011(uint8_t*, const TensorGroupInfo&,
                                 uint8_t*, const TensorGroupInfo&,
                                 uint8_t*, const TensorGroupInfo&,
                                 uint8_t*, const TensorGroupInfo&,
                                 uint8_t*, const TensorGroupInfo&,
                                 int64_t, aclrtStream, float);

extern "C" void run_kernel_v016(uint8_t*, const TensorGroupInfo&,
                                 uint8_t*, const TensorGroupInfo&,
                                 uint8_t*, const TensorGroupInfo&,
                                 uint8_t*, const TensorGroupInfo&,
                                 uint8_t*, const TensorGroupInfo&,
                                 int64_t, aclrtStream, float);

#endif
