/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

/* !
 * \file bank_conflict_nd2nz_3510.h
 * \brief Ascend 950 ND2NZ Regbase implementation for the bank conflict sample
 */

#ifndef BANK_CONFLICT_ND2NZ_3510_H
#define BANK_CONFLICT_ND2NZ_3510_H

#include "kernel_operator.h"

namespace Nd2Nz3510 {
// half 类型下 1 个 DataBlock/C0 固定为 32B，即 16 个 half 元素；该值不是可调 tiling 参数。
constexpr uint32_t C0_ELEMS = 16;

template <uint32_t tileH, uint32_t tileW, uint32_t vecLenHalf>
__simd_vf__ inline void RegLoadStoreVF(__ubuf__ half* dstAddr, __ubuf__ half* srcAddr, uint32_t actualTileH)
{
    constexpr uint32_t dstNzC0Stride = (SCENARIO == 1) ? tileH : (tileH + 1);
    constexpr uint32_t vecsPerRow = tileW / vecLenHalf;
    constexpr uint32_t vecLenDbs = vecLenHalf / C0_ELEMS;

    AscendC::Reg::RegTensor<half> reg;
    AscendC::Reg::MaskReg mask = AscendC::Reg::CreateMask<half, AscendC::Reg::MaskPattern::ALL>();

    for (uint16_t k = 0; k < vecsPerRow; ++k) {
        const uint32_t dstColGroupOffset = k * vecLenDbs * dstNzC0Stride * C0_ELEMS;

        for (uint16_t r = 0; r < actualTileH; ++r) {
            AscendC::Reg::LoadAlign<half>(reg, srcAddr + r * tileW + k * vecLenHalf);

            __ubuf__ half* dstRowAddr = dstAddr + dstColGroupOffset + r * C0_ELEMS;
            AscendC::Reg::StoreAlign<half, AscendC::Reg::DataCopyMode::DATA_BLOCK_COPY>(
                dstRowAddr, reg, dstNzC0Stride, mask);
        }
    }
}

template <uint32_t tileH, uint32_t tileW, uint32_t vecLenHalf>
__aicore__ inline void ComputeTileWithRegVF(
    AscendC::LocalTensor<half>& ndBuf, AscendC::LocalTensor<half>& nzBuf, uint32_t actualTileH)
{
    __ubuf__ half* dstAddr = reinterpret_cast<__ubuf__ half*>(nzBuf.GetPhyAddr());
    __ubuf__ half* srcAddr = reinterpret_cast<__ubuf__ half*>(ndBuf.GetPhyAddr());
    asc_vf_call<RegLoadStoreVF<tileH, tileW, vecLenHalf>>(dstAddr, srcAddr, actualTileH);
}
} // namespace Nd2Nz3510

#endif // BANK_CONFLICT_ND2NZ_3510_H
