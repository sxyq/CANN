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
#include "reg_compute/kernel_reg_compute_vec_vconv_intf.h"
#endif

extern "C" __simd_vf__ void vec_vconv_test()
{
    AscendC::Reg::RegTensor<float> a;
    AscendC::Reg::RegTensor<int16_t> b;
    AscendC::Reg::MaskReg mask;
    static constexpr AscendC::Reg::CastTrait castTrait = {
        AscendC::Reg::RegLayout::ZERO, AscendC::Reg::SatMode::NO_SAT, AscendC::Reg::MaskMergeMode::ZEROING,
        AscendC::RoundMode::CAST_RINT};

    // template <typename T = DefaultType, typename U = DefaultType, const CastTrait& trait = castTrait, typename S,
    // typename V>
    // __simd_callee__ inline void Cast(S& dstReg, V& srcReg, MaskReg& mask);
    AscendC::Reg::Cast<int16_t, float, castTrait>(b, a, mask);

    // template <typename T = DefaultType, RoundMode roundMode = RoundMode::CAST_NONE,
    //           MaskMergeMode mode = MaskMergeMode::ZEROING, typename S>
    // __simd_callee__ inline void Truncate(S& dstReg, S& srcReg, MaskReg& mask);
    AscendC::Reg::Truncate<float, AscendC::RoundMode::CAST_NONE, AscendC::Reg::MaskMergeMode::ZEROING>(a, a, mask);
}
