/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under
 * the terms and conditions of CANN Open Software License Agreement Version 2.0
 * (the "License"). Please refer to the License for details. You may not use
 * this file except in compliance with the License. THIS SOFTWARE IS PROVIDED ON
 * AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS
 * FOR A PARTICULAR PURPOSE. See LICENSE in the root of the software repository
 * for the full text of the License.
 */

#ifndef BANK_CONFLICT_VF_FUNC
#define BANK_CONFLICT_VF_FUNC

#include "kernel_operator.h"
#include "config.h"

constexpr uint16_t oneRepSize = 256 / sizeof(float);
constexpr uint16_t repeatTimes = TOTAL_LENGTH / oneRepSize;

template <typename T>
__simd_vf__ inline void AddWithUnRollVF(__ubuf__ T* src0, __ubuf__ T* src1, __ubuf__ T* dst)
{
    AscendC::Reg::MaskReg mask = AscendC::Reg::CreateMask<T, AscendC::Reg::MaskPattern::ALL>();
    AscendC::Reg::RegTensor<T> reg0, reg1, reg2, reg3, reg4, reg5;
    constexpr uint16_t mainRepeatTimes = repeatTimes / 2;
    for (uint16_t i = 0; i < mainRepeatTimes; ++i) {
        AscendC::Reg::LoadAlign(reg0, src0 + oneRepSize * i);
        AscendC::Reg::LoadAlign(reg1, src1 + oneRepSize * i);
        AscendC::Reg::LoadAlign(reg3, src0 + oneRepSize * (mainRepeatTimes + i));
        AscendC::Reg::LoadAlign(reg4, src1 + oneRepSize * (mainRepeatTimes + i));
        AscendC::Reg::Add(reg2, reg0, reg1, mask);
        AscendC::Reg::Add(reg5, reg3, reg4, mask);
        AscendC::Reg::StoreAlign(dst + oneRepSize * i, reg2, mask);
        AscendC::Reg::StoreAlign(dst + oneRepSize * (mainRepeatTimes + i), reg5, mask);
    }
}

// Single-instruction version (no loop unrolling), for comparison with AddWithUnRollVF
template <typename T>
__simd_vf__ inline void AddWithoutUnRollVF(__ubuf__ T* src0, __ubuf__ T* src1, __ubuf__ T* dst)
{
    AscendC::Reg::MaskReg mask = AscendC::Reg::CreateMask<T, AscendC::Reg::MaskPattern::ALL>();
    AscendC::Reg::RegTensor<T> reg0, reg1, reg2;
    for (uint16_t i = 0; i < repeatTimes; ++i) {
        AscendC::Reg::LoadAlign(reg0, src0 + oneRepSize * i);
        AscendC::Reg::LoadAlign(reg1, src1 + oneRepSize * i);
        AscendC::Reg::Add(reg2, reg0, reg1, mask);
        AscendC::Reg::StoreAlign(dst + oneRepSize * i, reg2, mask);
    }
}

template <typename T>
__simd_vf__ inline void ComputeBoundVF(__ubuf__ T* src0, __ubuf__ T* src1, __ubuf__ T* dst)
{
    AscendC::Reg::MaskReg mask = AscendC::Reg::CreateMask<T, AscendC::Reg::MaskPattern::ALL>();
    AscendC::Reg::RegTensor<T> reg0, reg1, reg2;
    for (uint16_t i = 0; i < repeatTimes; ++i) {
        AscendC::Reg::LoadAlign(reg0, src0 + oneRepSize * i);
        AscendC::Reg::LoadAlign(reg1, src1 + oneRepSize * i);
        for (uint16_t j = 0; j < 1000; ++j) {
            AscendC::Reg::Adds(reg0, reg0, (T)1.0, mask);
        }
        AscendC::Reg::Add(reg2, reg0, reg1, mask);
        AscendC::Reg::StoreAlign(dst + oneRepSize * i, reg2, mask);
    }
}

template <typename T>
__simd_vf__ inline void AddsByRowVF(__ubuf__ T* src0, __ubuf__ T* src1, __ubuf__ T* dst)
{
    // 横着搬运512个数: 8轮×64元素
    AscendC::Reg::MaskReg mask = AscendC::Reg::CreateMask<T, AscendC::Reg::MaskPattern::ALL>();
    AscendC::Reg::RegTensor<T> reg0, reg1;
    for (uint16_t i = 0; i < 8; ++i) {
        AscendC::Reg::LoadAlign<T, AscendC::Reg::DataCopyMode::DATA_BLOCK_COPY>(reg0, src0 + oneRepSize * i, 1, mask);
        AscendC::Reg::Adds(reg1, reg0, (T)0, mask);
        AscendC::Reg::StoreAlign<T, AscendC::Reg::DataCopyMode::DATA_BLOCK_COPY>(dst + oneRepSize * i, reg1, 1, mask);
    }
}

template <typename T>
__simd_vf__ inline void AddsByColVF(__ubuf__ T* src0, __ubuf__ T* src1, __ubuf__ T* dst)
{
    // stride=8, 竖着搬运: 分8块×64列, 每次加载1个DataBlock(8元素), 64轮拼满一行
    AscendC::Reg::MaskReg mask = AscendC::Reg::CreateMask<T, AscendC::Reg::MaskPattern::ALL>();
    AscendC::Reg::RegTensor<T> reg0, reg1;
    for (uint16_t i = 0; i < 8; ++i) {
        AscendC::Reg::LoadAlign<T, AscendC::Reg::DataCopyMode::DATA_BLOCK_COPY>(reg0, src0 + i * 8, 8, mask);
        AscendC::Reg::Adds(reg1, reg0, (T)0, mask);
        AscendC::Reg::StoreAlign<T, AscendC::Reg::DataCopyMode::DATA_BLOCK_COPY>(dst + i * 8, reg1, 8, mask);
    }
}

template <typename T>
__simd_vf__ inline void AddsWithBlockOverlap(__ubuf__ T* src0, __ubuf__ T* src1, __ubuf__ T* dst)
{
    // 测试BlockStride为0
    AscendC::Reg::MaskReg mask = AscendC::Reg::CreateMask<T, AscendC::Reg::MaskPattern::ALL>();
    AscendC::Reg::RegTensor<T> reg0, reg1;
    for (uint16_t i = 0; i < 8; ++i) {
        AscendC::Reg::LoadAlign<T, AscendC::Reg::DataCopyMode::DATA_BLOCK_COPY>(reg0, src0 + oneRepSize * i, 0, mask);
        AscendC::Reg::Adds(reg1, reg0, (T)0, mask);
        AscendC::Reg::StoreAlign<T, AscendC::Reg::DataCopyMode::DATA_BLOCK_COPY>(dst + oneRepSize * i, reg1, 1, mask);
    }
}

template <typename T>
__simd_vf__ inline void AddsWithRepeatOverlap(__ubuf__ T* src0, __ubuf__ T* src1, __ubuf__ T* dst)
{
    // 测试srcRepeatStride为0
    AscendC::Reg::MaskReg mask = AscendC::Reg::CreateMask<T, AscendC::Reg::MaskPattern::ALL>();
    AscendC::Reg::RegTensor<T> reg0, reg1;
    for (uint16_t i = 0; i < 8; ++i) {
        AscendC::Reg::LoadAlign<T, AscendC::Reg::DataCopyMode::DATA_BLOCK_COPY>(reg0, src0, 1, mask);
        AscendC::Reg::Adds(reg1, reg0, (T)0, mask);
        AscendC::Reg::StoreAlign<T, AscendC::Reg::DataCopyMode::DATA_BLOCK_COPY>(dst + oneRepSize * i, reg1, 1, mask);
    }
}

template <typename T1, typename T2, typename T3>
__simd_vf__ inline void GatherAndScatterVF(__ubuf__ T1* src0, __ubuf__ T2* src1, __ubuf__ T3* dst)
{
    AscendC::Reg::MaskReg mask = AscendC::Reg::CreateMask<T2, AscendC::Reg::MaskPattern::ALL>();
    AscendC::Reg::RegTensor<T1> indexReg;
    AscendC::Reg::RegTensor<T2> reg0;
    AscendC::Reg::RegTensor<T3> reg1;
    for (uint16_t i = 0; i < repeatTimes; ++i) {
        // load index
        AscendC::Reg::LoadAlign(indexReg, src0 + oneRepSize * i);
        AscendC::Reg::Gather(reg0, src1, indexReg, mask);
        AscendC::Reg::Adds(reg1, reg0, (T2)1.0f, mask);
        AscendC::Reg::Scatter(dst, reg1, indexReg, mask);
    }
}

#endif // !BANK_CONFLICT_VF_FUNC
