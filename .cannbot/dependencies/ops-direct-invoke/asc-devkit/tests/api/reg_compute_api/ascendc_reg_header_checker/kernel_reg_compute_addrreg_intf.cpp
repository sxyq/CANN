/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#ifndef VERIFY_SINGLE_HEADER
#include "reg_compute/kernel_reg_compute_intf.h"
#else
#include "reg_compute/kernel_reg_compute_addrreg_intf.h"
#endif

extern "C" __simd_vf__ void addrreg_test()
{
    AscendC::Reg::AddrReg a, b, c;

    // template <typename T> __simd_callee__ inline AddrReg CreateAddrReg(uint16_t index0, uint32_t stride0);
    a = AscendC::Reg::CreateAddrReg<float>(0, 256);
    // template <typename T>
    // __simd_callee__ inline AddrReg CreateAddrReg(uint16_t index0, uint32_t stride0, uint16_t index1, uint32_t
    // stride1);
    a = AscendC::Reg::CreateAddrReg<float>(0, 256, 0, 256);
    // template <typename T>
    // __simd_callee__ inline AddrReg CreateAddrReg(uint16_t index0, uint32_t stride0, uint16_t index1, uint32_t
    // stride1,
    //                                              uint16_t index2, uint32_t stride2);
    a = AscendC::Reg::CreateAddrReg<float>(0, 256, 0, 256, 0, 256);
    // template <typename T>
    // __simd_callee__ inline AddrReg CreateAddrReg(uint16_t index0, uint32_t stride0, uint16_t index1, uint32_t
    // stride1,
    //                                              uint16_t index2, uint32_t stride2, uint16_t index3, uint32_t
    //                                              stride3);
    a = AscendC::Reg::CreateAddrReg<float>(0, 256, 0, 256, 0, 256, 0, 256);
}
