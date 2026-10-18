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
#include "reg_compute/kernel_reg_compute_maskreg_intf.h"
#endif

extern "C" __simd_vf__ void maskreg_test()
{
    AscendC::Reg::MaskReg a, b, c, mask;
    AscendC::Reg::RegTensor<uint16_t> a1;

    // template <typename T, const RegTrait& regTrait = RegTraitNumOne>
    // __simd_callee__ inline MaskReg UpdateMask(uint32_t& scalarValue);
    uint32_t val = 128;
    a = AscendC::Reg::UpdateMask<float>(val);

    // template <typename T, MaskPattern mode = MaskPattern::ALL, const RegTrait& regTrait = RegTraitNumOne>
    // __simd_callee__ inline MaskReg CreateMask();
    a = AscendC::Reg::CreateMask<float, AscendC::Reg::MaskPattern::ALL>();

    // __simd_callee__ inline void Not(MaskReg& dst, MaskReg& src, MaskReg& mask);
    AscendC::Reg::Not(a, b, mask);

    // __simd_callee__ inline void And(MaskReg& dst, MaskReg& src0, MaskReg& src1, MaskReg& mask);
    AscendC::Reg::And(a, b, c, mask);

    // template <typename T = DefaultType, int16_t offset, typename U>
    // __simd_callee__ inline void MaskGenWithRegTensor(MaskReg& dst, U& srcReg);
    // (requires specific offset template parameter)
    AscendC::Reg::MaskGenWithRegTensor<uint16_t, 5>(a, a1);

    // __simd_callee__ inline void Or(MaskReg& dst, MaskReg& src0, MaskReg& src1, MaskReg& mask);
    AscendC::Reg::Or(a, b, c, mask);

    // __simd_callee__ inline void Xor(MaskReg& dst, MaskReg& src0, MaskReg& src1, MaskReg& mask);
    AscendC::Reg::Xor(a, b, c, mask);

    // __simd_callee__ inline void Move(MaskReg& dst, MaskReg& src, MaskReg& mask);
    AscendC::Reg::Move(a, b, mask);

    // __simd_callee__ inline void Move(MaskReg& dst, MaskReg& src);
    AscendC::Reg::Move(a, b);

    // template <typename T>
    // __simd_callee__ inline void Interleave(MaskReg& dst0, MaskReg& dst1, MaskReg& src0, MaskReg& src1);
    AscendC::Reg::Interleave<float>(a, b, c, mask);

    // template <typename T>
    // __simd_callee__ inline void DeInterleave(MaskReg& dst0, MaskReg& dst1, MaskReg& src0, MaskReg& src1);
    AscendC::Reg::DeInterleave<float>(a, b, c, mask);

    // __simd_callee__ inline void Select(MaskReg& dst, MaskReg& src0, MaskReg& src1, MaskReg& mask);
    AscendC::Reg::Select(a, b, c, mask);

    // template <HighLowPart part = HighLowPart::LOWEST>
    // __simd_callee__ inline void Pack(MaskReg& dst, MaskReg& src);
    AscendC::Reg::Pack<AscendC::Reg::HighLowPart::LOWEST>(a, b);

    // template <HighLowPart part = HighLowPart::LOWEST>
    // __simd_callee__ inline void UnPack(MaskReg& dst, MaskReg& src);
    AscendC::Reg::UnPack<AscendC::Reg::HighLowPart::LOWEST>(a, b);

    // template <typename T>
    // __simd_callee__ inline MaskReg MoveMask();
    a = AscendC::Reg::MoveMask<float>();
}
