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
#include "mockcpp/mockcpp.hpp"

using namespace AscendC;

template <typename SrcType>
class KernelDuplicate {
public:
    __aicore__ inline void Init(
        GM_ADDR srcGm, GM_ADDR dstGm, uint32_t stackSize, uint32_t dataSize, uint64_t maskCounter, uint64_t maskBitHigh,
        uint64_t maskBitLow, uint8_t repeatTimes, uint16_t dstBlockStride, uint8_t dstRepeatStride)
    {
        stackSize_ = stackSize;
        dataSize_ = dataSize;
        maskCounter_ = maskCounter;
        maskBit_[0] = maskBitHigh;
        maskBit_[1] = maskBitLow;
        repeatTimes_ = repeatTimes;
        dstBlockStride_ = dstBlockStride;
        dstRepeatStride_ = dstRepeatStride;

        srcGlobal_.SetGlobalBuffer(reinterpret_cast<__gm__ PrimT<SrcType>*>(srcGm), stackSize_);
        dstGlobal_.SetGlobalBuffer(reinterpret_cast<__gm__ PrimT<SrcType>*>(dstGm), stackSize_);
        pipe_.InitBuffer(inQueueSrc_, 1, stackSize_ * sizeof(PrimT<SrcType>));
        pipe_.InitBuffer(outQueue_, 1, stackSize_ * sizeof(PrimT<SrcType>));
    }

    __aicore__ inline void Process()
    {
        CopyIn();
        Compute();
        CopyOut();
    }

private:
    __aicore__ inline void CopyIn()
    {
        LocalTensor<SrcType> srcLocal = inQueueSrc_.AllocTensor<SrcType>();
        DataCopy(srcLocal, srcGlobal_, stackSize_);
        inQueueSrc_.EnQue(srcLocal);
    }

    __aicore__ inline void Compute()
    {
        LocalTensor<SrcType> dstLocal = outQueue_.AllocTensor<SrcType>();
        LocalTensor<SrcType> srcLocal = inQueueSrc_.DeQue<SrcType>();
        scalar_ = srcLocal.GetValue(0);

        event_t eventIdSToV = static_cast<event_t>(GetTPipePtr()->FetchEventID(HardEvent::S_V));
        SetFlag<HardEvent::S_V>(eventIdSToV);
        WaitFlag<HardEvent::S_V>(eventIdSToV);

        if constexpr (IsSameType<PrimT<SrcType>, SrcType>::value) {
            Duplicate(dstLocal, scalar_, maskBit_, repeatTimes_, dstBlockStride_, dstRepeatStride_);
            Duplicate(dstLocal, scalar_, maskCounter_, repeatTimes_, dstBlockStride_, dstRepeatStride_);
            Duplicate(dstLocal, scalar_, dataSize_);
        } else {
            Duplicate(dstLocal, srcLocal, dataSize_);
        }

        outQueue_.EnQue<SrcType>(dstLocal);
        inQueueSrc_.FreeTensor(srcLocal);
    }

    __aicore__ inline void CopyOut()
    {
        LocalTensor<SrcType> dstLocal = outQueue_.DeQue<SrcType>();
        DataCopy(dstGlobal_, dstLocal, stackSize_);
        outQueue_.FreeTensor(dstLocal);
    }

private:
    GlobalTensor<SrcType> srcGlobal_;
    GlobalTensor<SrcType> dstGlobal_;
    TPipe pipe_;
    TQue<TPosition::VECIN, 1> inQueueSrc_;
    TQue<TPosition::VECOUT, 1> outQueue_;
    uint64_t maskCounter_ = 0;
    uint64_t maskBit_[2] = {0, 0};
    uint8_t repeatTimes_ = 0;
    uint16_t dstBlockStride_ = 1;
    uint8_t dstRepeatStride_ = 8;
    uint32_t stackSize_ = 0;
    uint32_t dataSize_ = 0;
    PrimT<SrcType> scalar_ = 0;
};

template <typename SrcType>
__aicore__ void DuplicateTest(
    GM_ADDR srcGm, GM_ADDR dstGm, uint32_t stackSize, uint32_t dataSize, uint64_t maskCounter, uint64_t maskBitHigh,
    uint64_t maskBitLow, uint8_t repeatTimes, uint16_t dstBlockStride, uint8_t dstRepeatStride)
{
    KernelDuplicate<SrcType> op;
    op.Init(
        srcGm, dstGm, stackSize, dataSize, maskCounter, maskBitHigh, maskBitLow, repeatTimes, dstBlockStride,
        dstRepeatStride);
    op.Process();
}

struct InputParams {
    uint32_t stackSize;
    uint32_t dataSize;
    uint32_t dataTypeSize;
    uint64_t maskCounter;
    uint64_t maskBitHigh;
    uint64_t maskBitLow;
    uint8_t repeatTimes;
    uint16_t dstBlockStride;
    uint8_t dstRepeatStride;
};

class DuplicateTestsuite : public testing::Test {
protected:
    void TearDown() override { GlobalMockObject::verify(); }
};

#define DUPLICATE_NORMAL_TEST(testCaseName, dataType)                                                \
    TEST_F(DuplicateTestsuite, testCaseName)                                                         \
    {                                                                                                \
        InputParams params{256, 256, sizeof(PrimT<dataType>), 11, 22, 0, 3, 3, 3};                   \
        uint8_t srcGm[params.stackSize * params.dataTypeSize] = {0};                                 \
        uint8_t dstGm[params.stackSize * params.dataTypeSize] = {0};                                 \
        DuplicateTest<dataType>(                                                                     \
            srcGm, dstGm, params.stackSize, params.dataSize, params.maskCounter, params.maskBitHigh, \
            params.maskBitLow, params.repeatTimes, params.dstBlockStride, params.dstRepeatStride);   \
        EXPECT_EQ(dstGm[0], 0x00);                                                                   \
        EXPECT_EQ(dstGm[1], 0x00);                                                                   \
    }

DUPLICATE_NORMAL_TEST(Level0Float, float);
DUPLICATE_NORMAL_TEST(Level0Half, half);
DUPLICATE_NORMAL_TEST(Level0Bfloat16, bfloat16_t);
DUPLICATE_NORMAL_TEST(Level0Int16, int16_t);
DUPLICATE_NORMAL_TEST(Level0Uint16, uint16_t);
DUPLICATE_NORMAL_TEST(Level0Int32, int32_t);
DUPLICATE_NORMAL_TEST(Level0Uint32, uint32_t);

using TTFloat = TensorTrait<float>;
using TTUint64 = TensorTrait<uint64_t>;

DUPLICATE_NORMAL_TEST(TensorTraitNumOne, TTFloat);
DUPLICATE_NORMAL_TEST(TensorTraitNumTwo, TTUint64);

bool DuplicateImplCheckCalCount(const int32_t& calCount) { return calCount == 256; }

TEST_F(DuplicateTestsuite, ScalarOverloadsDispatch)
{
    InputParams params{256, 256, sizeof(float), 11, 22, 0, 3, 3, 3};
    MOCKER(DuplicateImpl, void (*)(__ubuf__ float*, const float&, const int32_t&))
        .times(1)
        .with(any(), any(), checkWith(DuplicateImplCheckCalCount));
    MOCKER(
        DuplicateImpl, void (*)(__ubuf__ float*, const float&, uint64_t*, const uint8_t, const uint16_t, const uint8_t))
        .times(1);
    MOCKER(
        DuplicateImpl, void (*)(__ubuf__ float*, const float&, uint64_t, const uint8_t, const uint16_t, const uint8_t))
        .times(1);

    uint8_t srcGm[params.stackSize * params.dataTypeSize] = {0};
    uint8_t dstGm[params.stackSize * params.dataTypeSize] = {0};
    DuplicateTest<float>(
        srcGm, dstGm, params.stackSize, params.dataSize, params.maskCounter, params.maskBitHigh, params.maskBitLow,
        params.repeatTimes, params.dstBlockStride, params.dstRepeatStride);
}
