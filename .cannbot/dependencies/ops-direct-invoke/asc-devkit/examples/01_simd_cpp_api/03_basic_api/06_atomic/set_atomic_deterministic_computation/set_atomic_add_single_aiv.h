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
 * \file set_atomic_add_single_aiv.h
 * \brief 单个AIV内，多次搬运指令串行做原子累加
 */

#include "acl/acl.h"
#include "data_utils.h"
#include "kernel_operator.h"

class KernelSetAtomicAddSingleAIV {
public:
    // 数据长度常量（32字节对齐）
    static constexpr int32_t DATA_LENGTH = 8;
    __aicore__ inline KernelSetAtomicAddSingleAIV() {}
    __aicore__ inline void Init(
        __gm__ uint8_t* src0Gm, __gm__ uint8_t* src1Gm, __gm__ uint8_t* src2Gm, __gm__ uint8_t* src3Gm,
        __gm__ uint8_t* dstGm)
    {
        src0Global.SetGlobalBuffer((__gm__ float*)src0Gm);
        src1Global.SetGlobalBuffer((__gm__ float*)src1Gm);
        src2Global.SetGlobalBuffer((__gm__ float*)src2Gm);
        src3Global.SetGlobalBuffer((__gm__ float*)src3Gm);
        dstGlobal.SetGlobalBuffer((__gm__ float*)dstGm);
    }
    __aicore__ inline void Process()
    {
        // 使用LocalMemAllocator进行内存分配
        AscendC::LocalMemAllocator<AscendC::Hardware::UB> ubAllocator;
        AscendC::LocalTensor<float> src0Local = ubAllocator.Alloc<float, DATA_LENGTH>();
        AscendC::LocalTensor<float> src1Local = ubAllocator.Alloc<float, DATA_LENGTH>();
        AscendC::LocalTensor<float> src2Local = ubAllocator.Alloc<float, DATA_LENGTH>();
        AscendC::LocalTensor<float> src3Local = ubAllocator.Alloc<float, DATA_LENGTH>();

        AscendC::DisableDmaAtomic();
        AscendC::DataCopy(src0Local, src0Global, DATA_LENGTH);
        AscendC::DataCopy(src1Local, src1Global, DATA_LENGTH);
        AscendC::DataCopy(src2Local, src2Global, DATA_LENGTH);
        AscendC::DataCopy(src3Local, src3Global, DATA_LENGTH);

        AscendC::SetFlag<AscendC::HardEvent::MTE2_MTE3>(0);
        AscendC::WaitFlag<AscendC::HardEvent::MTE2_MTE3>(0);

        // 在原子累加之前，以src0Local初始化GM
        AscendC::DataCopy(dstGlobal, src0Local, DATA_LENGTH);
        // 确保在原子累加之前，gm的初始化完成
        AscendC::PipeBarrier<PIPE_MTE3>();

        // 开启原子累加
        // 预期累加执行的顺序是：src2Local + src3Local + src1Local
        AscendC::SetAtomicAdd<float>();
        AscendC::DataCopy(dstGlobal, src2Local, DATA_LENGTH);
        // 两条搬运指令之间需要调用PipeBarrier<PIPE_MTE3>()，确保src2Local和src3Local的搬运顺序正确
        AscendC::PipeBarrier<PIPE_MTE3>();
        AscendC::DataCopy(dstGlobal, src3Local, DATA_LENGTH);
        AscendC::PipeBarrier<PIPE_MTE3>();
        AscendC::DataCopy(dstGlobal, src1Local, DATA_LENGTH);

        AscendC::DisableDmaAtomic();
    }

private:
    AscendC::GlobalTensor<float> src0Global, src1Global, src2Global, src3Global, dstGlobal;
};
