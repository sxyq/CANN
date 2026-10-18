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
#include "reg_compute/kernel_reg_compute_vec_binary_scalar_intf.h"
#endif

extern "C" __simd_vf__ void vec_binary_scalar_test()
{
    AscendC::Reg::RegTensor<float> dstReg, srcReg;
    AscendC::Reg::RegTensor<int16_t> a, b;
    AscendC::Reg::MaskReg mask;

    // template <typename T = DefaultType, typename U, MaskMergeMode mode = MaskMergeMode::ZEROING, typename S>
    // __simd_callee__ inline void Adds(S& dstReg, S& srcReg, U scalarValue, MaskReg& mask);
    AscendC::Reg::Adds<float, float>(dstReg, srcReg, 1.0f, mask);

    // template <typename T = DefaultType, typename U, MaskMergeMode mode = MaskMergeMode::ZEROING, typename S>
    // __simd_callee__ inline void Muls(S& dstReg, S& srcReg, U scalarValue, MaskReg& mask);
    AscendC::Reg::Muls<float, float>(dstReg, srcReg, 2.0f, mask);

    // template <typename T = DefaultType, typename U, MaskMergeMode mode = MaskMergeMode::ZEROING, typename S>
    // __simd_callee__ inline void Maxs(S& dstReg, S& srcReg, U scalarValue, MaskReg& mask);
    AscendC::Reg::Maxs<float, float>(dstReg, srcReg, 5.0f, mask);

    // template <typename T = DefaultType, typename U, MaskMergeMode mode = MaskMergeMode::ZEROING, typename S>
    // __simd_callee__ inline void Mins(S& dstReg, S& srcReg, U scalarValue, MaskReg& mask);
    AscendC::Reg::Mins<float, float>(dstReg, srcReg, 0.0f, mask);

    // template <typename T = DefaultType, typename U, MaskMergeMode mode = MaskMergeMode::ZEROING, typename S>
    // __simd_callee__ inline void ShiftLefts(S& dstReg, S& srcReg, U scalarValue, MaskReg& mask);
    AscendC::Reg::ShiftLefts<int16_t, int16_t>(a, b, 2, mask);

    // template <typename T = DefaultType, typename U, MaskMergeMode mode = MaskMergeMode::ZEROING, typename S>
    // __simd_callee__ inline void ShiftRights(S& dstReg, S& srcReg, U scalarValue, MaskReg& mask);
    AscendC::Reg::ShiftRights<int16_t, int16_t>(a, b, 1, mask);

    // template <typename T = DefaultType, typename U, MaskMergeMode mode = MaskMergeMode::ZEROING, typename S>
    // __simd_callee__ inline void LeakyRelu(S& dstReg, S& srcReg, U scalarValue, MaskReg& mask);
    AscendC::Reg::LeakyRelu<float, float>(dstReg, srcReg, 0.01f, mask);
}
