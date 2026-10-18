/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#include "kernel_operator.h"
#include "leaky_relu_custom_tiling.h"

constexpr int32_t BUFFER_NUM = 2;
constexpr uint32_t STATIC_TILE_LENGTH = 64;

template <uint32_t tileLength>
class KernelLeakyRelu {
public:
    __aicore__ inline KernelLeakyRelu() {}
    __aicore__ inline void Init(
        __gm__ uint8_t* x, __gm__ uint8_t* y, uint32_t totalLength, uint32_t tileNum, float negativeSlope)
    {
        ascendc_assert(tileNum != 0, "tileNum can not be zero.\n");
        this->blockLength = totalLength / AscendC::GetBlockNum();
        this->tileNum = tileNum;
        this->negativeSlope = static_cast<float>(negativeSlope);
        uint32_t runtimeTileLength = this->blockLength / tileNum / BUFFER_NUM;
        ascendc_assert(runtimeTileLength == tileLength, "runtime tileLength must match template tileLength.\n");

        xGm.SetGlobalBuffer((__gm__ float*)x + this->blockLength * AscendC::GetBlockIdx(), this->blockLength);
        yGm.SetGlobalBuffer((__gm__ float*)y + this->blockLength * AscendC::GetBlockIdx(), this->blockLength);
    }
    __aicore__ inline void Process()
    {
        int32_t loopCount = this->tileNum * BUFFER_NUM;

        AscendC::LocalMemAllocator<AscendC::Hardware::UB> ubAllocator;
        AscendC::LocalTensor<float> xLocal = ubAllocator.Alloc<float, tileLength>();
        AscendC::LocalTensor<float> yLocal = ubAllocator.Alloc<float, tileLength>();
        AscendC::LocalTensor<float> tmpTensor1 = ubAllocator.Alloc<float, tileLength>();
        AscendC::LocalTensor<float> tmpTensor2 = ubAllocator.Alloc<float, tileLength>();

        float inputVal = 0.0f;

        AscendC::SetFlag<AscendC::HardEvent::V_MTE2>(EVENT_ID0);
        AscendC::SetFlag<AscendC::HardEvent::MTE3_V>(EVENT_ID0);

        for (int32_t i = 0; i < loopCount; i++) {
            AscendC::WaitFlag<AscendC::HardEvent::V_MTE2>(EVENT_ID0);

            AscendC::DataCopy(xLocal, xGm[i * tileLength], tileLength);

            AscendC::SetFlag<AscendC::HardEvent::MTE2_V>(EVENT_ID0);
            AscendC::WaitFlag<AscendC::HardEvent::MTE2_V>(EVENT_ID0);

            AscendC::WaitFlag<AscendC::HardEvent::MTE3_V>(EVENT_ID0);

            AscendC::Maxs(tmpTensor1, xLocal, inputVal, tileLength);
            AscendC::Mins(tmpTensor2, xLocal, inputVal, tileLength);
            AscendC::Muls(tmpTensor2, tmpTensor2, this->negativeSlope, tileLength);
            AscendC::Add(yLocal, tmpTensor1, tmpTensor2, tileLength);

            AscendC::SetFlag<AscendC::HardEvent::V_MTE2>(EVENT_ID0);
            AscendC::SetFlag<AscendC::HardEvent::V_MTE3>(EVENT_ID0);
            AscendC::WaitFlag<AscendC::HardEvent::V_MTE3>(EVENT_ID0);

            AscendC::DataCopy(yGm[i * tileLength], yLocal, tileLength);

            AscendC::SetFlag<AscendC::HardEvent::MTE3_V>(EVENT_ID0);
        }

        AscendC::WaitFlag<AscendC::HardEvent::V_MTE2>(EVENT_ID0);
        AscendC::WaitFlag<AscendC::HardEvent::MTE3_V>(EVENT_ID0);
    }

private:
    AscendC::GlobalTensor<float> xGm;
    AscendC::GlobalTensor<float> yGm;
    uint32_t blockLength;
    uint32_t tileNum;
    float negativeSlope;
};

extern "C" __global__ __aicore__ void leaky_relu_custom(
    __gm__ uint8_t* x, __gm__ uint8_t* y, __gm__ uint8_t* workspace, __gm__ uint8_t* tiling)
{
    AscendC::InitSocState();

    REGISTER_TILING_DEFAULT(LeakyReluCustomTilingData);
    GET_TILING_DATA(tiling_data, tiling);

    KernelLeakyRelu<STATIC_TILE_LENGTH> op;
    op.Init(x, y, tiling_data.totalLength, tiling_data.tileNum, tiling_data.negativeSlope);
    op.Process();

    AscendC::PipeBarrier<PIPE_ALL>();
}
