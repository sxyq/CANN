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
 * \file set_atomic_add_multi_aiv.h
 * \brief 多个AIV串行做原子累加
 */

#include "acl/acl.h"
#include "data_utils.h"
#include "kernel_operator.h"

class KernelSetAtomicAddMultiAIV {
public:
    // 数据长度常量（32字节对齐）
    static constexpr int32_t DATA_LENGTH = 8;
    // gmWorkspace申请的空间最少要求为：3 * 32Bytes + 32Bytes
    static constexpr int32_t SYNC_SPACE_LENGTH = 256;
    __aicore__ inline KernelSetAtomicAddMultiAIV() {}
    __aicore__ inline void Init(
        __gm__ uint8_t* src0Gm, __gm__ uint8_t* src1Gm, __gm__ uint8_t* src2Gm, __gm__ uint8_t* src3Gm,
        __gm__ uint8_t* src4Gm, __gm__ uint8_t* dstGm, AscendC::TPipe* pipeIn)
    {
        pipe = pipeIn;
        blockIdx = AscendC::GetBlockIdx();
        src0Global.SetGlobalBuffer((__gm__ float*)src0Gm);
        src1Global.SetGlobalBuffer((__gm__ float*)src1Gm);
        src2Global.SetGlobalBuffer((__gm__ float*)src2Gm);
        src3Global.SetGlobalBuffer((__gm__ float*)src3Gm);
        src4Global.SetGlobalBuffer((__gm__ int32_t*)src4Gm);
        dstGlobal.SetGlobalBuffer((__gm__ float*)dstGm);
        pipe->InitBuffer(vecIn, 1, DATA_LENGTH * sizeof(int32_t));
        pipe->InitBuffer(inQueueSrc0, 1, DATA_LENGTH * sizeof(float));
        pipe->InitBuffer(inQueueSrc1, 1, DATA_LENGTH * sizeof(float));
        pipe->InitBuffer(inQueueSrc2, 1, DATA_LENGTH * sizeof(float));
        pipe->InitBuffer(inQueueSrc3, 1, DATA_LENGTH * sizeof(float));
        pipe->InitBuffer(inQueueSrc4, 1, SYNC_SPACE_LENGTH * sizeof(int32_t));
        pipe->InitBuffer(outQueueDst, 1, DATA_LENGTH * sizeof(float));
    }
    __aicore__ inline void Process()
    {
        // 预期的4个核执行的顺序是核0-->核2-->核3-->核1
        // 核0负责在原子累加之前，以src0Local初始化GM；核2、3、1负责按顺序做原子累加

        if (blockIdx == 0) {
            AscendC::LocalTensor<float> src0Local = inQueueSrc0.AllocTensor<float>();
            AscendC::DataCopy(src0Local, src0Global, DATA_LENGTH);
            AscendC::SetFlag<AscendC::HardEvent::MTE2_MTE3>(0);
            AscendC::WaitFlag<AscendC::HardEvent::MTE2_MTE3>(0);

            // 在原子累加之前，以src0Local初始化GM
            AscendC::DataCopy(dstGlobal, src0Local, DATA_LENGTH);

            // 核0通知核2，核0的src0Local初始化已完成
            auto sync_buf = vecIn.AllocTensor<int32_t>();
            AscendC::IBSet(src4Global, sync_buf, 0, 0); // 最后两个参数：int32_t blockIdx, int32_t eventID
            inQueueSrc0.FreeTensor(src0Local);
            vecIn.FreeTensor(sync_buf);
            AscendC::printf("核的下标%d\n", blockIdx);
        } else if (blockIdx == 1) {
            AscendC::LocalTensor<float> src1Local = inQueueSrc1.AllocTensor<float>();
            // 核1等待核3，直到核3数据已完成累加
            auto sync_buf = vecIn.AllocTensor<int32_t>();
            AscendC::IBWait(src4Global, sync_buf, 3, 0); // 最后两个参数：int32_t blockIdx, int32_t eventID

            AscendC::DataCopy(src1Local, src1Global, DATA_LENGTH);
            AscendC::SetFlag<AscendC::HardEvent::MTE2_MTE3>(0);
            AscendC::WaitFlag<AscendC::HardEvent::MTE2_MTE3>(0);

            AscendC::DisableDmaAtomic();
            AscendC::SetAtomicAdd<float>();
            AscendC::DataCopy(dstGlobal, src1Local, DATA_LENGTH);
            AscendC::DisableDmaAtomic();
            vecIn.FreeTensor(sync_buf);
            inQueueSrc1.FreeTensor(src1Local);
            AscendC::printf("核的下标%d\n", blockIdx);
        } else if (blockIdx == 2) {
            AscendC::LocalTensor<float> src2Local = inQueueSrc2.AllocTensor<float>();
            // 核2等待核0，直到核0数据已完成累加
            auto sync_buf = vecIn.AllocTensor<int32_t>();
            AscendC::IBWait(src4Global, sync_buf, 0, 0); // 最后两个参数：int32_t blockIdx, int32_t eventID

            AscendC::DataCopy(src2Local, src2Global, DATA_LENGTH);
            AscendC::SetFlag<AscendC::HardEvent::MTE2_MTE3>(0);
            AscendC::WaitFlag<AscendC::HardEvent::MTE2_MTE3>(0);

            AscendC::DisableDmaAtomic();
            AscendC::SetAtomicAdd<float>();
            AscendC::DataCopy(dstGlobal, src2Local, DATA_LENGTH);
            AscendC::DisableDmaAtomic();

            // 核2通知核3，核2数据已经累加完成
            AscendC::IBSet(src4Global, sync_buf, 2, 0);
            vecIn.FreeTensor(sync_buf);
            inQueueSrc2.FreeTensor(src2Local);
            AscendC::printf("核的下标%d\n", blockIdx);
        } else if (blockIdx == 3) {
            // 核3等待核2，直到核2数据已完成累加
            auto sync_buf = vecIn.AllocTensor<int32_t>();
            AscendC::IBWait(src4Global, sync_buf, 2, 0); // 最后两个参数：int32_t blockIdx, int32_t eventID

            AscendC::LocalTensor<float> src3Local = inQueueSrc3.AllocTensor<float>();
            AscendC::DataCopy(src3Local, src3Global, DATA_LENGTH);
            //  AscendC::DumpTensor(src3Local,0,8);
            AscendC::SetFlag<AscendC::HardEvent::MTE2_MTE3>(0);
            AscendC::WaitFlag<AscendC::HardEvent::MTE2_MTE3>(0);

            AscendC::DisableDmaAtomic();
            AscendC::SetAtomicAdd<float>();
            AscendC::DataCopy(dstGlobal, src3Local, DATA_LENGTH);
            AscendC::DisableDmaAtomic();

            // 核3通知核1，核3数据已经累加完成
            AscendC::IBSet(src4Global, sync_buf, 3, 0);
            vecIn.FreeTensor(sync_buf);
            inQueueSrc3.FreeTensor(src3Local);
            AscendC::printf("核的下标%d\n", blockIdx);
        }
    }

private:
    AscendC::TPipe* pipe;
    AscendC::TQue<AscendC::TPosition::VECIN, 1> inQueueSrc0;
    AscendC::TQue<AscendC::TPosition::VECIN, 1> inQueueSrc1;
    AscendC::TQue<AscendC::TPosition::VECIN, 1> inQueueSrc2;
    AscendC::TQue<AscendC::TPosition::VECIN, 1> inQueueSrc3;
    AscendC::TQue<AscendC::TPosition::VECIN, 1> inQueueSrc4;
    AscendC::TQue<AscendC::TPosition::VECIN, 1> vecIn;
    AscendC::TQue<AscendC::TPosition::VECOUT, 1> outQueueDst;
    AscendC::GlobalTensor<float> src0Global, src1Global, src2Global, src3Global, dstGlobal;
    AscendC::GlobalTensor<int32_t> src4Global;
    AscendC::LocalTensor<float> src0Local, src1Local, src2Local, src3Local;
    AscendC::LocalTensor<int32_t> src4Local;
    int32_t blockIdx = 0;
};
