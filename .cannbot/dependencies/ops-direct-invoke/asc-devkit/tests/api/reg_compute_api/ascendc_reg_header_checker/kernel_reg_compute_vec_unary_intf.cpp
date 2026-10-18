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
#include "reg_compute/kernel_reg_compute_vec_unary_intf.h"
#endif

extern "C" __simd_vf__ void vec_unary_test()
{
    AscendC::Reg::RegTensor<float> dstReg, srcReg, srcReg2;
    AscendC::Reg::RegTensor<int32_t> srcRegInt;
    AscendC::Reg::RegTensor<complex64> a, b, c;
    AscendC::Reg::MaskReg mask;

    // template <typename T = DefaultType, MaskMergeMode mode = MaskMergeMode::ZEROING, typename U>
    // __simd_callee__ inline void Abs(U& dstReg, U& srcReg, MaskReg& mask);
    AscendC::Reg::Abs(dstReg, srcReg, mask);

    // template <typename T = DefaultType, typename U = DefaultType, MaskMergeMode mode = MaskMergeMode::ZEROING,
    //           typename S, typename V>
    // __simd_callee__ inline void Abs(S& dstReg, V& srcReg, MaskReg& mask);
    AscendC::Reg::Abs(dstReg, a, mask);

    // template <typename T = DefaultType, MaskMergeMode mode = MaskMergeMode::ZEROING, typename U>
    // __simd_callee__ inline void Relu(U& dstReg, U& srcReg, MaskReg& mask);
    AscendC::Reg::Relu(dstReg, srcReg, mask);

    // template <typename T = DefaultType, auto mode = MaskMergeMode::ZEROING, typename U>
    // __simd_callee__ inline void Exp(U& dstReg, U& srcReg, MaskReg& mask);
    AscendC::Reg::Exp(dstReg, srcReg, mask);

    // template <typename T = DefaultType, auto mode = MaskMergeMode::ZEROING, typename U>
    // __simd_callee__ inline void Sqrt(U& dstReg, U& srcReg, MaskReg& mask);
    AscendC::Reg::Sqrt(dstReg, srcReg, mask);

    // template <typename T = DefaultType, auto mode = MaskMergeMode::ZEROING, typename U>
    // __simd_callee__ inline void Ln(U& dstReg, U& srcReg, MaskReg& mask);
    AscendC::Reg::Ln(dstReg, srcReg, mask);

    // template <typename T = DefaultType, auto mode = MaskMergeMode::ZEROING, typename U>
    // __simd_callee__ inline void Log(U& dstReg, U& srcReg, MaskReg& mask);
    AscendC::Reg::Log(dstReg, srcReg, mask);

    // template <typename T = DefaultType, auto mode = MaskMergeMode::ZEROING, typename U>
    // __simd_callee__ inline void Log2(U& dstReg, U& srcReg, MaskReg& mask);
    AscendC::Reg::Log2(dstReg, srcReg, mask);

    // template <typename T = DefaultType, auto mode = MaskMergeMode::ZEROING, typename U>
    // __simd_callee__ inline void Log10(U& dstReg, U& srcReg, MaskReg& mask);
    AscendC::Reg::Log10(dstReg, srcReg, mask);

    // template <typename T = DefaultType, MaskMergeMode mode = MaskMergeMode::ZEROING, typename U>
    // __simd_callee__ inline void Neg(U& dstReg, U& srcReg, MaskReg& mask);
    AscendC::Reg::Neg(dstReg, srcReg, mask);

    // template <typename T = DefaultType, MaskMergeMode mode = MaskMergeMode::ZEROING, typename U>
    // __simd_callee__ inline void Not(U& dstReg, U& srcReg, MaskReg& mask);
    AscendC::Reg::Not(dstReg, srcReg, mask);
}
