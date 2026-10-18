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
#include "reg_compute/kernel_reg_compute_vec_reduce_intf.h"
#endif

extern "C" __simd_vf__ void vec_reduce_test()
{
    AscendC::Reg::RegTensor<float> dstReg, srcReg;
    AscendC::Reg::MaskReg mask;

    // template <ReduceType type = ReduceType::SUM, typename T = DefaultType, typename U = DefaultType,
    //           MaskMergeMode mode = MaskMergeMode::ZEROING, typename S, typename V>
    // __simd_callee__ inline void Reduce(S& dstReg, V srcReg, MaskReg mask);
    AscendC::Reg::Reduce<AscendC::Reg::ReduceType::SUM>(dstReg, srcReg, mask);
    AscendC::Reg::Reduce<AscendC::Reg::ReduceType::MAX>(dstReg, srcReg, mask);
    AscendC::Reg::Reduce<AscendC::Reg::ReduceType::MIN>(dstReg, srcReg, mask);

    // template <ReduceType type = ReduceType::SUM, typename T = DefaultType,
    //           MaskMergeMode mode = MaskMergeMode::ZEROING, typename U>
    // __simd_callee__ inline void ReduceDataBlock(U& dstReg, U srcReg, MaskReg mask);
    AscendC::Reg::ReduceDataBlock<AscendC::Reg::ReduceType::SUM>(dstReg, srcReg, mask);

    // template <PairReduce type = PairReduce::SUM, typename T = DefaultType,
    //           MaskMergeMode mode = MaskMergeMode::ZEROING, typename U>
    // __simd_callee__ inline void PairReduceElem(U& dstReg, U srcReg, MaskReg mask);
    AscendC::Reg::PairReduceElem<AscendC::Reg::PairReduce::SUM>(dstReg, srcReg, mask);
}
