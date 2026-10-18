/**
 * Copyright (c) 2025 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#include "kernel_operator.h"
#include "add_custom_tiling_sink_tiling_struct.h"

template <uint32_t tileLength>
class KernelAdd {
public:
    __aicore__ inline void Init(
        __gm__ uint8_t* x, __gm__ uint8_t* y, __gm__ uint8_t* z, uint32_t totalLength, uint32_t tileNum)
    {
        ascendc_assert(tileNum != 0, "tileNum can not be zero.\n");
        this->blockLength = totalLength / AscendC::GetBlockNum();
        this->tileNum = tileNum;
        uint32_t runtimeTileLength = this->blockLength / tileNum;
        ascendc_assert(runtimeTileLength == tileLength, "runtime tileLength must match template tileLength.\n");

        xGm.SetGlobalBuffer((__gm__ DTYPE_X*)x + this->blockLength * AscendC::GetBlockIdx(), this->blockLength);
        yGm.SetGlobalBuffer((__gm__ DTYPE_Y*)y + this->blockLength * AscendC::GetBlockIdx(), this->blockLength);
        zGm.SetGlobalBuffer((__gm__ DTYPE_Z*)z + this->blockLength * AscendC::GetBlockIdx(), this->blockLength);
    }
    __aicore__ inline void Process()
    {
        AscendC::LocalMemAllocator<AscendC::Hardware::UB> ubAllocator;
        AscendC::LocalTensor<DTYPE_X> xLocal = ubAllocator.Alloc<DTYPE_X, tileLength>();
        AscendC::LocalTensor<DTYPE_Y> yLocal = ubAllocator.Alloc<DTYPE_Y, tileLength>();
        AscendC::LocalTensor<DTYPE_Z> zLocal = ubAllocator.Alloc<DTYPE_Z, tileLength>();

        for (int32_t i = 0; i < this->tileNum; i++) {
            if (i != 0) {
                AscendC::WaitFlag<AscendC::HardEvent::V_MTE2>(EVENT_ID0);
            }

            AscendC::DataCopy(xLocal, xGm[i * tileLength], tileLength);
            AscendC::DataCopy(yLocal, yGm[i * tileLength], tileLength);

            AscendC::SetFlag<AscendC::HardEvent::MTE2_V>(EVENT_ID0);
            AscendC::WaitFlag<AscendC::HardEvent::MTE2_V>(EVENT_ID0);

            if (i != 0) {
                AscendC::WaitFlag<AscendC::HardEvent::MTE3_V>(EVENT_ID0);
            }

            AscendC::Add(zLocal, xLocal, yLocal, tileLength);

            if (i != this->tileNum - 1) {
                AscendC::SetFlag<AscendC::HardEvent::V_MTE2>(EVENT_ID0);
            }

            AscendC::SetFlag<AscendC::HardEvent::V_MTE3>(EVENT_ID0);
            AscendC::WaitFlag<AscendC::HardEvent::V_MTE3>(EVENT_ID0);

            AscendC::DataCopy(zGm[i * tileLength], zLocal, tileLength);

            if (i != this->tileNum - 1) {
                AscendC::SetFlag<AscendC::HardEvent::MTE3_V>(EVENT_ID0);
            }
        }
    }

private:
    AscendC::GlobalTensor<DTYPE_X> xGm;
    AscendC::GlobalTensor<DTYPE_Y> yGm;
    AscendC::GlobalTensor<DTYPE_Z> zGm;
    uint32_t blockLength;
    uint32_t tileNum;
};

extern "C" __global__ __aicore__ void add_custom_tiling_sink(
    __gm__ uint8_t* x, __gm__ uint8_t* y, __gm__ uint8_t* z, __gm__ uint8_t* workspace, __gm__ uint8_t* tiling)
{
    constexpr uint32_t STATIC_TILE_LENGTH = 8;
    REGISTER_TILING_DEFAULT(TilingSinkTilingData);
    GET_TILING_DATA(tiling_data, tiling);
    KERNEL_TASK_TYPE_DEFAULT(KERNEL_TYPE_AIV_ONLY);

    AscendC::InitSocState();
    KernelAdd<STATIC_TILE_LENGTH> op;
    op.Init(x, y, z, tiling_data.totalLength, tiling_data.tileNum);
    op.Process();
    AscendC::PipeBarrier<PIPE_ALL>();
}
