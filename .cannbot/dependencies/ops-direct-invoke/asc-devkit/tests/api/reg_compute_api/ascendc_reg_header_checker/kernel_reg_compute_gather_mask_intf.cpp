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
#include "reg_compute/kernel_reg_compute_gather_mask_intf.h"
#endif

extern "C" __simd_vf__ void gather_mask_test()
{
    AscendC::Reg::RegTensor<uint8_t> dstReg, srcReg, indexReg;
    AscendC::Reg::MaskReg mask;

    // template <typename T = DefaultType, GatherMaskMode store = GatherMaskMode::NO_STORE_REG, typename U>
    // __simd_callee__ inline void Squeeze(U& dstReg, U& srcReg, MaskReg& mask);
    AscendC::Reg::Squeeze(dstReg, srcReg, mask);

    // template <typename T = DefaultType, typename U>
    // __simd_callee__ inline void Unsqueeze(U& dstReg, MaskReg& mask);
    AscendC::Reg::Unsqueeze(dstReg, mask);

    // template <SpecialPurposeReg spr>
    // __simd_callee__ inline void ClearSpr();
    AscendC::Reg::ClearSpr<AscendC::SpecialPurposeReg::AR>();

    // template <typename T = DefaultType, typename U = DefaultType, typename S, typename V>
    // __simd_callee__ inline void Gather(S& dstReg, S& srcReg, V& indexReg);
    AscendC::Reg::Gather(dstReg, srcReg, indexReg);
}

extern "C" __global__ __aicore__ void gather_mask_test_2()
{
    // template <SpecialPurposeReg spr>
    // __aicore__ inline int64_t GetSpr();
    // (GetSpr cannot be tested in __simd_vf__ context)
    auto res = AscendC::Reg::GetSpr<AscendC::SpecialPurposeReg::AR>();
}
