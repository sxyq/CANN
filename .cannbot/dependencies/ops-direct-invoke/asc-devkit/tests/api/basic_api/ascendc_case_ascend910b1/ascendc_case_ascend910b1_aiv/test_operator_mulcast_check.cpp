/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <cstdint>
#include <gtest/gtest.h>
#include "kernel_operator.h"

using namespace AscendC;

namespace {
constexpr uint32_t DATA_SIZE = 256;
constexpr uint8_t REPEAT_TIME = 1;

template <typename Func>
void WithMulCastTensors(Func func)
{
    TPipe tpipe;
    TBuf<TPosition::VECCALC> src0Buf;
    TBuf<TPosition::VECCALC> src1Buf;
    TBuf<TPosition::VECCALC> dstBuf;
    tpipe.InitBuffer(src0Buf, DATA_SIZE * sizeof(half));
    tpipe.InitBuffer(src1Buf, DATA_SIZE * sizeof(half));
    tpipe.InitBuffer(dstBuf, DATA_SIZE * sizeof(int8_t));

    LocalTensor<half> src0 = src0Buf.Get<half>();
    LocalTensor<half> src1 = src1Buf.Get<half>();
    LocalTensor<int8_t> dst = dstBuf.Get<int8_t>();
    func(dst, src0, src1);
}

void RunMulCastInvalidMaskValue()
{
    WithMulCastTensors([](LocalTensor<int8_t>& dst, LocalTensor<half>& src0, LocalTensor<half>& src1) {
        SetMaskNorm();
        BinaryRepeatParams repeatParams;
        MulCast(dst, src0, src1, static_cast<uint64_t>(129), REPEAT_TIME, repeatParams);
    });
}

void RunMulCastNullMaskArray()
{
    WithMulCastTensors([](LocalTensor<int8_t>& dst, LocalTensor<half>& src0, LocalTensor<half>& src1) {
        uint64_t* mask = nullptr;
        BinaryRepeatParams repeatParams;
        MulCast(dst, src0, src1, mask, REPEAT_TIME, repeatParams);
    });
}

void RunMulCastInvalidCount()
{
    WithMulCastTensors([](LocalTensor<int8_t>& dst, LocalTensor<half>& src0, LocalTensor<half>& src1) {
        uint32_t count = static_cast<uint32_t>(INT32_MAX) + 1U;
        MulCast(dst, src0, src1, count);
    });
}

void RunMulCastInvalidTensorPosition()
{
    TPipe tpipe;
    TBuf<TPosition::VECCALC> src0Buf;
    TBuf<TPosition::VECCALC> src1Buf;
    tpipe.InitBuffer(src0Buf, DATA_SIZE * sizeof(half));
    tpipe.InitBuffer(src1Buf, DATA_SIZE * sizeof(half));

    LocalTensor<half> src0 = src0Buf.Get<half>();
    LocalTensor<half> src1 = src1Buf.Get<half>();
    LocalTensor<int8_t> dst;
    TBuffAddr dstAddr;
    dstAddr.logicPos = static_cast<uint8_t>(TPosition::GM);
    dst.SetAddr(dstAddr);
    dst.InitBuffer(0, DATA_SIZE);

    BinaryRepeatParams repeatParams;
    MulCast(dst, src0, src1, static_cast<uint64_t>(128), REPEAT_TIME, repeatParams);
}
} // namespace

class MulCastApiCheckTest : public testing::Test {
protected:
    void SetUp() override { AscendC::SetGCoreType(2); }

    void TearDown() override { AscendC::SetGCoreType(0); }
};

TEST_F(MulCastApiCheckTest, MulCastInvalidMaskValue) { EXPECT_DEATH(RunMulCastInvalidMaskValue(), ""); }

TEST_F(MulCastApiCheckTest, MulCastNullMaskArray) { EXPECT_DEATH(RunMulCastNullMaskArray(), ""); }

TEST_F(MulCastApiCheckTest, MulCastInvalidCount) { EXPECT_DEATH(RunMulCastInvalidCount(), ""); }

TEST_F(MulCastApiCheckTest, MulCastInvalidTensorPosition) { EXPECT_DEATH(RunMulCastInvalidTensorPosition(), ""); }
