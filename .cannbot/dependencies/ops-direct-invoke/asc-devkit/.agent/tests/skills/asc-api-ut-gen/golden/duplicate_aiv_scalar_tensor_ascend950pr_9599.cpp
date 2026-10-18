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
#include <vector>
#include "kernel_operator.h"

using namespace std;
using namespace AscendC;

template <typename T>
class KernelDuplicate {
public:
    __aicore__ inline void Init(__gm__ uint8_t* srcGm, __gm__ uint8_t* dstGm, uint32_t dataSize)
    {
        this->dataSize = dataSize;

        srcGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ PrimT<T>*>(srcGm), dataSize);
        dstGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ PrimT<T>*>(dstGm), dataSize);

        pipe.InitBuffer(inQueueSrc, 1, dataSize * sizeof(PrimT<T>));
        pipe.InitBuffer(outQueueDst, 1, dataSize * sizeof(PrimT<T>));
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
        LocalTensor<T> srcLocal = inQueueSrc.AllocTensor<T>();
        DataCopy(srcLocal, srcGlobal, dataSize);
        inQueueSrc.EnQue(srcLocal);
    }

    __aicore__ inline void Compute()
    {
        LocalTensor<T> srcLocal = inQueueSrc.DeQue<T>();
        LocalTensor<T> dstLocal = outQueueDst.AllocTensor<T>();
        PrimT<T> scalarValue = srcLocal.GetValue(0);

        event_t eventIdSToV = static_cast<event_t>(GetTPipePtr()->FetchEventID(HardEvent::S_V));
        SetFlag<HardEvent::S_V>(eventIdSToV);
        WaitFlag<HardEvent::S_V>(eventIdSToV);

        if constexpr (IsSameType<PrimT<T>, T>::value) {
            Duplicate(dstLocal, scalarValue, dataSize);
        } else {
            Duplicate(dstLocal, srcLocal, dataSize);
        }

        outQueueDst.EnQue<T>(dstLocal);
        inQueueSrc.FreeTensor(srcLocal);
    }

    __aicore__ inline void CopyOut()
    {
        LocalTensor<T> dstLocal = outQueueDst.DeQue<T>();
        DataCopy(dstGlobal, dstLocal, dataSize);
        outQueueDst.FreeTensor(dstLocal);
    }

    TPipe pipe;
    TQue<TPosition::VECIN, 1> inQueueSrc;
    TQue<TPosition::VECOUT, 1> outQueueDst;
    GlobalTensor<T> srcGlobal;
    GlobalTensor<T> dstGlobal;
    uint32_t dataSize;
};

template <typename T>
__aicore__ inline void main_Duplicate(uint8_t* srcGm, uint8_t* dstGm, uint32_t dataSize)
{
    KernelDuplicate<T> op;
    op.Init(srcGm, dstGm, dataSize);
    op.Process();
}

struct DuplicateTestParams {
    uint32_t data_size;
    uint32_t data_bit_size;
    void (*cal_func)(uint8_t*, uint8_t*, uint32_t);
};

class DuplicateTestsuite : public testing::Test, public testing::WithParamInterface<DuplicateTestParams> {
protected:
    void SetUp() { AscendC::SetGCoreType(2); }
    void TearDown() { AscendC::SetGCoreType(0); }
};

INSTANTIATE_TEST_CASE_P(
    TEST_DUPLICATE, DuplicateTestsuite,
    ::testing::Values(
        DuplicateTestParams{256, 4, main_Duplicate<float>},
        DuplicateTestParams{256, 4, main_Duplicate<TensorTrait<float>>}));

TEST_P(DuplicateTestsuite, DuplicateTestCase)
{
    auto param = GetParam();

    std::vector<uint8_t> srcGm(param.data_size * param.data_bit_size, 0);
    std::vector<uint8_t> dstGm(param.data_size * param.data_bit_size, 0);

    param.cal_func(srcGm.data(), dstGm.data(), param.data_size);

    EXPECT_EQ(dstGm[0], 0x00);
}
