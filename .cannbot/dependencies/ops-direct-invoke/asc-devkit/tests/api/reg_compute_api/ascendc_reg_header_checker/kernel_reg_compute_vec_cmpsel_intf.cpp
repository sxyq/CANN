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
#include "reg_compute/kernel_reg_compute_vec_cmpsel_intf.h"
#endif

extern "C" __simd_vf__ void vec_cmpsel_test()
{
    AscendC::Reg::RegTensor<float> dstReg, srcReg0, srcReg1;
    AscendC::Reg::MaskReg mask;

    // template <typename T = DefaultType, CMPMODE mode = CMPMODE::EQ, typename U>
    // __simd_callee__ inline void Compare(MaskReg& dst, U& srcReg0, U& srcReg1, MaskReg& mask);
    AscendC::Reg::Compare<float, AscendC::CMPMODE::EQ>(mask, srcReg0, srcReg1, mask);
    AscendC::Reg::Compare<float, AscendC::CMPMODE::NE>(mask, srcReg0, srcReg1, mask);
    AscendC::Reg::Compare<float, AscendC::CMPMODE::GT>(mask, srcReg0, srcReg1, mask);
    AscendC::Reg::Compare<float, AscendC::CMPMODE::GE>(mask, srcReg0, srcReg1, mask);
    AscendC::Reg::Compare<float, AscendC::CMPMODE::LT>(mask, srcReg0, srcReg1, mask);
    AscendC::Reg::Compare<float, AscendC::CMPMODE::LE>(mask, srcReg0, srcReg1, mask);

    // template <typename T = DefaultType, CMPMODE mode = CMPMODE::EQ, typename U, typename S>
    // __simd_callee__ inline void Compares(MaskReg& dst, U& srcReg, S scalarValue, MaskReg& mask);
    AscendC::Reg::Compares<float, AscendC::CMPMODE::EQ>(mask, srcReg0, 0.0f, mask);

    // template <typename T = DefaultType, typename U>
    // __simd_callee__ inline void Select(U& dstReg, U& srcReg0, U& srcReg1, MaskReg& mask);
    AscendC::Reg::Select(dstReg, srcReg0, srcReg1, mask);
}
