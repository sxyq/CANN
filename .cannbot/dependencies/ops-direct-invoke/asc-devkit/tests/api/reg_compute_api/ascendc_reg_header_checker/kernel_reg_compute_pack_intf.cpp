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
#include "reg_compute/kernel_reg_compute_pack_intf.h"
#endif

extern "C" __simd_vf__ void pack_test()
{
    AscendC::Reg::RegTensor<uint8_t> a;
    AscendC::Reg::RegTensor<uint16_t> b;
    AscendC::Reg::MaskReg mask;

    // template <typename T = DefaultType, typename U = DefaultType, HighLowPart part = HighLowPart::LOWEST, typename S,
    //           typename V>
    // __simd_callee__ inline void Pack(S& dstReg, V& srcReg);
    AscendC::Reg::Pack<uint8_t, uint16_t, AscendC::Reg::HighLowPart::LOWEST>(a, b);

    // template <typename T = DefaultType, typename U = DefaultType, HighLowPart part = HighLowPart::LOWEST, typename S,
    //           typename V>
    // __simd_callee__ inline void UnPack(S& dstReg, V& srcReg);
    AscendC::Reg::UnPack<uint16_t, uint8_t, AscendC::Reg::HighLowPart::LOWEST>(b, a);
}
