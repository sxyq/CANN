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
#include "reg_compute/kernel_reg_compute_vec_fused_intf.h"
#endif

extern "C" __simd_vf__ void vec_fused_test()
{
    AscendC::Reg::RegTensor<float> dstReg, srcReg0, srcReg1;
    AscendC::Reg::RegTensor<half> a, b;
    AscendC::Reg::MaskReg mask;

    // template <typename T0 = DefaultType, typename T1 = DefaultType, typename T2, RegLayout layout = RegLayout::ZERO,
    //           typename T3, typename T4>
    // __simd_callee__ inline void MulsCast(T3& dstReg, T4& srcReg, T2 scalarValue, MaskReg& mask);
    AscendC::Reg::MulsCast<half, float, float, AscendC::Reg::RegLayout::ZERO>(a, srcReg0, 2.0f, mask);

    // template <typename T = DefaultType, MaskMergeMode mode = MaskMergeMode::ZEROING, typename U>
    // __simd_callee__ inline void AbsSub(U& dstReg, U& srcReg0, U& srcReg1, MaskReg& mask);
    AscendC::Reg::AbsSub(dstReg, srcReg0, srcReg1, mask);

    // template <typename T = DefaultType, typename U = DefaultType, RegLayout layout = RegLayout::ZERO,
    //           MaskMergeMode mode = MaskMergeMode::ZEROING, typename S, typename V>
    // __simd_callee__ inline void ExpSub(S& dstReg, V& srcReg0, V& srcReg1, MaskReg& mask);
    AscendC::Reg::ExpSub<float, float, AscendC::Reg::RegLayout::ZERO, AscendC::Reg::MaskMergeMode::ZEROING>(
        dstReg, srcReg0, srcReg1, mask);

    // template <typename T = DefaultType, MaskMergeMode mode = MaskMergeMode::ZEROING, typename U>
    // __simd_callee__ inline void MulDstAdd(U& dstReg, U& srcReg0, U& srcReg1, MaskReg& mask);
    AscendC::Reg::MulDstAdd(dstReg, srcReg0, srcReg1, mask);
}
