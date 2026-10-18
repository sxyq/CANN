/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <gtest/gtest.h>
#include "kernel_operator.h"

using namespace AscendC;

namespace {
template <typename T>
void MainC100VecReduce(
    __gm__ uint8_t* __restrict__ srcGm, __gm__ uint8_t* __restrict__ dstGm, __gm__ int32_t srcDataSize,
    __gm__ int32_t dstDataSize)
{
    TPipe tpipe;
    GlobalTensor<T> inputGlobal;
    GlobalTensor<T> outputGlobal;
    inputGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ T*>(srcGm), srcDataSize);
    outputGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ T*>(dstGm), dstDataSize);

    TBuf<TPosition::VECIN> inputTbuf;
    tpipe.InitBuffer(inputTbuf, srcDataSize * sizeof(T));
    LocalTensor<T> inputLocal = inputTbuf.Get<T>();

    TBuf<TPosition::VECOUT> outputTbuf;
    tpipe.InitBuffer(outputTbuf, dstDataSize * sizeof(T));
    LocalTensor<T> outputLocal = outputTbuf.Get<T>();

    TBuf<TPosition::VECCALC> workTbuf;
    tpipe.InitBuffer(workTbuf, AlignUp(srcDataSize * sizeof(T), ONE_BLK_SIZE));
    LocalTensor<T> workLocal = workTbuf.Get<T>();

    DataCopy(inputLocal, inputGlobal, srcDataSize);
    event_t eventIdMte2ToV = static_cast<event_t>(GetTPipePtr()->FetchEventID<HardEvent::MTE2_V>());
    SetFlag<HardEvent::MTE2_V>(eventIdMte2ToV);
    WaitFlag<HardEvent::MTE2_V>(eventIdMte2ToV);

    const int32_t oneRepeatCount = ONE_REPEAT_BYTE_SIZE / sizeof(T);
    ReduceMax<T>(outputLocal, inputLocal, workLocal, oneRepeatCount, false);
    ReduceMin<T>(outputLocal, inputLocal, workLocal, oneRepeatCount, false);
    ReduceSum<T>(outputLocal, inputLocal, workLocal, oneRepeatCount);

    const int32_t twoRepeatCount = oneRepeatCount * 2;
    ReduceMax<T>(outputLocal, inputLocal, workLocal, twoRepeatCount, false);
    ReduceMin<T>(outputLocal, inputLocal, workLocal, twoRepeatCount, false);
    ReduceSum<T>(outputLocal, inputLocal, workLocal, twoRepeatCount);

    const int32_t tailCount = oneRepeatCount + 1;
    ReduceSum<T>(outputLocal, inputLocal, workLocal, tailCount);
    ReduceMax<T>(outputLocal, inputLocal, workLocal, tailCount, true);
    ReduceMin<T>(outputLocal, inputLocal, workLocal, tailCount, true);

    const int32_t multiStageCount = oneRepeatCount * 130 + 1;
    ReduceMax<T>(outputLocal, inputLocal, workLocal, multiStageCount, true);
    ReduceMin<T>(outputLocal, inputLocal, workLocal, multiStageCount, true);

    event_t eventIdVToMte3 = static_cast<event_t>(GetTPipePtr()->FetchEventID<HardEvent::V_MTE3>());
    SetFlag<HardEvent::V_MTE3>(eventIdVToMte3);
    WaitFlag<HardEvent::V_MTE3>(eventIdVToMte3);
    DataCopy(outputGlobal, outputLocal, dstDataSize);
    PipeBarrier<PIPE_ALL>();
}

struct C100ReduceTestParams {
    void (*calFunc)(uint8_t*, uint8_t*, int32_t, int32_t);
    int32_t srcDataSize;
    int32_t dstDataSize;
    int32_t dtypeSize;
};

class C100ReduceTestSuite : public testing::Test, public testing::WithParamInterface<C100ReduceTestParams> {
protected:
    void SetUp() override { SetGCoreType(AIV_TYPE); }

    void TearDown() override
    {
        CheckSyncState();
        SetGCoreType(MIX_TYPE);
    }
};

INSTANTIATE_TEST_CASE_P(
    C100ReduceCases, C100ReduceTestSuite,
    ::testing::Values(C100ReduceTestParams{MainC100VecReduce<half>, 16800, 128, 2}));

TEST_P(C100ReduceTestSuite, ReduceMaxMinSumMultiRepeat)
{
    auto param = GetParam();
    uint8_t srcGm[param.srcDataSize * param.dtypeSize] = {0};
    uint8_t dstGm[param.dstDataSize * param.dtypeSize] = {0};

    param.calFunc(srcGm, dstGm, param.srcDataSize, param.dstDataSize);
    EXPECT_EQ(dstGm[0], 0x00);
}
} // namespace
