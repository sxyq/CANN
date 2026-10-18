/**
 * Copyright (c) 2025 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <gtest/gtest.h>
#include <mockcpp/mockcpp.hpp>
#include "kernel_operator.h"
// #include "api_check/kernel_cpu_check.h"

using namespace std;
using namespace AscendC;
namespace {
int32_t RaiseStub(int32_t i) { return 0; }
} // namespace

class TestLoaddataCheck : public testing::Test {
protected:
    static void SetUpTestCase() {}
    static void TearDownTestCase() {}
    virtual void SetUp() {}
    virtual void TearDown() {}
};

struct TestLoaddata2dApiCheckParams {
    uint64_t srcSizeEle;
    uint64_t dstSizeEle;
    TPosition srcpos;
    TPosition dstpos;
    uint8_t repeat;
    uint16_t startIndex;
    uint16_t srcStride;
    uint16_t dstGap;
    bool expect;
};

struct TestLoaddata3dv1ApiCheckParams {
    uint64_t srcSizeEle;
    uint64_t dstSizeEle;
    TPosition srcpos;
    TPosition dstpos;
    uint8_t repeatMode;
    uint8_t repeat;
    uint8_t jumpStride;
    bool expect;
};

struct TestLoaddata3dv2ApiCheckParams {
    uint64_t srcSizeEle;
    uint64_t dstSizeEle;
    TPosition srcpos;
    TPosition dstpos;
    uint16_t channelSize;
    uint16_t l1H;
    uint16_t l1W;
    uint16_t kExtension;
    uint16_t mExtension;
    bool expect;
};

class TestLoaddata2dApiCheckSuite : public testing::Test,
                                    public testing::WithParamInterface<TestLoaddata2dApiCheckParams> {
protected:
    void SetUp() {}
    void TearDown() {}
};

class TestLoaddata3dv1ApiCheckSuite : public testing::Test,
                                      public testing::WithParamInterface<TestLoaddata3dv1ApiCheckParams> {
protected:
    void SetUp() {}
    void TearDown() {}
};

class TestLoaddata3dv2ApiCheckSuite : public testing::Test,
                                      public testing::WithParamInterface<TestLoaddata3dv2ApiCheckParams> {
protected:
    void SetUp() {}
    void TearDown() {}
};

INSTANTIATE_TEST_CASE_P(
    TestLoaddata2dApiCheck, TestLoaddata2dApiCheckSuite,
    ::testing::Values(
        TestLoaddata2dApiCheckParams{512, 512, TPosition::A1, TPosition::A2, 1, 0, 1, 0, true},
        TestLoaddata2dApiCheckParams{512, 512, TPosition::A2, TPosition::A1, 1, 0, 1, 0, false},
        TestLoaddata2dApiCheckParams{512, 512, TPosition::A1, TPosition::C1, 1, 0, 1, 0, false},
        TestLoaddata2dApiCheckParams{54289, 512, TPosition::A1, TPosition::A2, 1, 0, 1, 0, true},
        TestLoaddata2dApiCheckParams{255, 512, TPosition::A1, TPosition::A2, 1, 0, 1, 0, true},
        TestLoaddata2dApiCheckParams{512, 255, TPosition::B1, TPosition::B2, 1, 0, 1, 0, true}));

INSTANTIATE_TEST_CASE_P(
    TestLoaddata3dv1ApiCheck, TestLoaddata3dv1ApiCheckSuite,
    ::testing::Values(
        TestLoaddata3dv1ApiCheckParams{3584, 3584, TPosition::A1, TPosition::A2, 1, 1, 1, false},
        TestLoaddata3dv1ApiCheckParams{3584, 3584, TPosition::A1, TPosition::A2, 0, 1, 1, false},
        TestLoaddata3dv1ApiCheckParams{18577, 3584, TPosition::A1, TPosition::A2, 1, 1, 1, false},
        TestLoaddata3dv1ApiCheckParams{3584, 256, TPosition::A1, TPosition::A2, 1, 1, 1, false}));

INSTANTIATE_TEST_CASE_P(
    TestLoaddata3dv2ApiCheck, TestLoaddata3dv2ApiCheckSuite,
    ::testing::Values(
        TestLoaddata3dv2ApiCheckParams{6144, 6144, TPosition::A1, TPosition::A2, 32, 12, 16, 32, 192, true},
        TestLoaddata3dv2ApiCheckParams{6144, 6144, TPosition::A2, TPosition::A2, 32, 12, 16, 32, 192, false},
        TestLoaddata3dv2ApiCheckParams{6144, 6144, TPosition::A1, TPosition::C1, 32, 12, 16, 32, 192, false},
        TestLoaddata3dv2ApiCheckParams{26145, 6144, TPosition::A1, TPosition::A2, 32, 12, 16, 32, 192, true},
        TestLoaddata3dv2ApiCheckParams{6144, 16000, TPosition::A1, TPosition::A2, 32, 12, 16, 32, 192, true},
        TestLoaddata3dv2ApiCheckParams{6143, 6144, TPosition::A1, TPosition::A2, 32, 12, 16, 32, 192, true},
        TestLoaddata3dv2ApiCheckParams{6144, 6143, TPosition::B1, TPosition::B2, 32, 12, 16, 32, 192, true}));

TEST_P(TestLoaddata2dApiCheckSuite, Loaddata2dApiCheckAllHighLevel)
{
    TPipe tpipe;
    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    auto param = GetParam();
    uint64_t srcSizeEle = param.srcSizeEle;
    uint64_t dstSizeEle = param.dstSizeEle;

    LocalTensor<half> input;
    if (param.srcpos == TPosition::A1) {
        TBuf<TPosition::A1> tbuf;
        tpipe.InitBuffer(tbuf, AlignUp(srcSizeEle * sizeof(half), ONE_BLK_SIZE));
        input = tbuf.Get<half>();
    } else if (param.srcpos == TPosition::A2) {
        TBuf<TPosition::A2> tbuf;
        tpipe.InitBuffer(tbuf, AlignUp(srcSizeEle * sizeof(half), ONE_BLK_SIZE));
        input = tbuf.Get<half>();
    } else {
        TBuf<TPosition::B1> tbuf;
        tpipe.InitBuffer(tbuf, AlignUp(srcSizeEle * sizeof(half), ONE_BLK_SIZE));
        input = tbuf.Get<half>();
    }

    LocalTensor<half> output;
    if (param.dstpos == TPosition::A1) {
        TBuf<TPosition::A1> tbuf1;
        tpipe.InitBuffer(tbuf1, AlignUp(dstSizeEle * sizeof(half), ONE_BLK_SIZE));
        output = tbuf1.Get<half>();
    } else if (param.dstpos == TPosition::A2) {
        TBuf<TPosition::A2> tbuf1;
        tpipe.InitBuffer(tbuf1, AlignUp(dstSizeEle * sizeof(half), ONE_BLK_SIZE));
        output = tbuf1.Get<half>();
    } else if (param.dstpos == TPosition::C1) {
        TBuf<TPosition::C1> tbuf1;
        tpipe.InitBuffer(tbuf1, AlignUp(dstSizeEle * sizeof(half), ONE_BLK_SIZE));
        output = tbuf1.Get<half>();
    } else {
        TBuf<TPosition::B2> tbuf1;
        tpipe.InitBuffer(tbuf1, AlignUp(dstSizeEle * sizeof(half), ONE_BLK_SIZE));
        output = tbuf1.Get<half>();
    }

    uint8_t repeatTimes = param.repeat;
    uint16_t startIndex = param.startIndex;
    uint16_t srcStride = param.srcStride;
    uint16_t dstGap = param.dstGap;

    bool ifTranspose = false;
    uint8_t addrMode = 0;

    check::LoadData2dApiParams chkParams{
        reinterpret_cast<uint64_t>(output.GetPhyAddr()),
        reinterpret_cast<uint64_t>(input.GetPhyAddr()),
        startIndex,
        repeatTimes,
        srcStride,
        static_cast<uint8_t>(startIndex),
        dstGap,
        static_cast<bool>(ifTranspose),
        addrMode,
        static_cast<uint32_t>(sizeof(half)),
        static_cast<uint32_t>(sizeof(half)),
        static_cast<uint64_t>(output.GetLength()),
        static_cast<uint64_t>(input.GetLength()),
        static_cast<uint8_t>(output.GetPosition()),
        static_cast<uint8_t>(input.GetPosition())};
    // check::TikcppLoaddata2dCheck chkIns { "loaddata2d", chkParams };
    // bool flag = CheckFuncLoadData2dImpl(chkParams, "loaddata2d");

    // EXPECT_EQ(flag, param.expect);
}

TEST_P(TestLoaddata3dv1ApiCheckSuite, Loaddata3dv1ApiCheckAllHighLevel)
{
    TPipe tpipe;
    auto param = GetParam();
    uint64_t srcSizeEle = param.srcSizeEle;
    uint64_t dstSizeEle = param.dstSizeEle;

    TBuf<TPosition::A1> tbuf;
    tpipe.InitBuffer(tbuf, AlignUp(srcSizeEle * sizeof(uint8_t), ONE_BLK_SIZE));
    LocalTensor<uint8_t> input = tbuf.Get<uint8_t>();

    TBuf<TPosition::A2> tbuf1;
    tpipe.InitBuffer(tbuf1, AlignUp(dstSizeEle * sizeof(uint8_t), ONE_BLK_SIZE));
    LocalTensor<uint8_t> output = tbuf1.Get<uint8_t>();

    uint8_t repeatMode = param.repeatMode;
    uint8_t repeatTimes = param.repeat;
    uint8_t jumpStride = param.jumpStride;

    uint8_t padList[4] = {7, 7, 7, 7};
    uint16_t l1H = 2;
    uint16_t l1W = 56;
    uint16_t c1Index = 0;
    uint8_t fetchFilterW = 0;
    uint8_t fetchFilterH = 0;
    int16_t leftTopW = 0;
    int16_t leftTopH = 0;
    uint8_t strideW = 1;
    uint8_t strideH = 1;
    uint8_t filterW = 1;
    uint8_t filterH = 1;
    uint8_t dilationFilterW = 1;
    uint8_t dilationFilterH = 1;
    uint8_t cSize = 1;
    uint8_t padValue = 7;

    check::LoadData3dv1ApiParams chkParams{
        reinterpret_cast<uint64_t>(output.GetPhyAddr()),
        reinterpret_cast<uint64_t>(input.GetPhyAddr()),
        padList,
        l1H,
        l1W,
        c1Index,
        fetchFilterW,
        fetchFilterH,
        leftTopW,
        leftTopH,
        strideW,
        strideH,
        filterW,
        filterH,
        dilationFilterW,
        dilationFilterH,
        jumpStride,
        repeatMode,
        repeatTimes,
        cSize,
        static_cast<uint32_t>(sizeof(uint8_t)),
        static_cast<uint32_t>(sizeof(uint8_t)),
        static_cast<uint64_t>(output.GetLength()),
        static_cast<uint64_t>(input.GetLength()),
        static_cast<uint8_t>(output.GetPosition()),
        static_cast<uint8_t>(input.GetPosition())};
    // check::TikcppLoaddata3dv1Check chkIns { "loaddata3dv1", chkParams };
    // bool flag = CheckFuncLoadData3dv1Impl(chkParams, "loaddata3dv1");

    // EXPECT_EQ(flag, param.expect);
}

TEST_P(TestLoaddata3dv2ApiCheckSuite, Loaddata3dv2ApiCheckAllHighLevel)
{
    TPipe tpipe;
    auto param = GetParam();
    uint64_t srcSizeEle = param.srcSizeEle;
    uint64_t dstSizeEle = param.dstSizeEle;

    LocalTensor<float> input;
    if (param.srcpos == TPosition::A1) {
        TBuf<TPosition::A1> tbuf;
        tpipe.InitBuffer(tbuf, AlignUp(srcSizeEle * sizeof(float), ONE_BLK_SIZE));
        input = tbuf.Get<float>();
    } else if (param.srcpos == TPosition::A2) {
        TBuf<TPosition::A2> tbuf;
        tpipe.InitBuffer(tbuf, AlignUp(srcSizeEle * sizeof(float), ONE_BLK_SIZE));
        input = tbuf.Get<float>();
    } else {
        TBuf<TPosition::B1> tbuf;
        tpipe.InitBuffer(tbuf, AlignUp(srcSizeEle * sizeof(float), ONE_BLK_SIZE));
        input = tbuf.Get<float>();
    }

    LocalTensor<float> output;
    if (param.dstpos == TPosition::A2) {
        TBuf<TPosition::A2> tbuf1;
        tpipe.InitBuffer(tbuf1, AlignUp(dstSizeEle * sizeof(float), ONE_BLK_SIZE));
        output = tbuf1.Get<float>();
    } else if (param.dstpos == TPosition::C1) {
        TBuf<TPosition::C1> tbuf1;
        tpipe.InitBuffer(tbuf1, AlignUp(dstSizeEle * sizeof(float), ONE_BLK_SIZE));
        output = tbuf1.Get<float>();
    } else {
        TBuf<TPosition::B2> tbuf1;
        tpipe.InitBuffer(tbuf1, AlignUp(dstSizeEle * sizeof(float), ONE_BLK_SIZE));
        output = tbuf1.Get<float>();
    }

    uint16_t channelSize = param.channelSize;
    uint16_t l1H = param.l1H;
    uint16_t l1W = param.l1W;
    uint16_t kExtension = param.kExtension;
    uint16_t mExtension = param.mExtension;

    uint8_t padList[4] = {0, 0, 0, 0};
    uint16_t kStartPt = 0;
    uint16_t mStartPt = 0;
    uint8_t strideW = 1;
    uint8_t strideH = 1;
    uint8_t filterW = 1;
    uint8_t filterH = 1;
    uint8_t dilationFilterW = 1;
    uint8_t dilationFilterH = 1;
    bool enTranspose = false;
    bool enSmallK = false;
    uint8_t padValue = 0;

    check::LoadData3dv2ApiParams chkParams{
        reinterpret_cast<uint64_t>(output.GetPhyAddr()),
        reinterpret_cast<uint64_t>(input.GetPhyAddr()),
        padList,
        l1H,
        l1W,
        channelSize,
        kExtension,
        mExtension,
        kStartPt,
        mStartPt,
        strideW,
        strideH,
        filterW,
        filterH,
        dilationFilterW,
        dilationFilterH,
        enTranspose,
        enSmallK,
        static_cast<uint32_t>(sizeof(float)),
        static_cast<uint32_t>(sizeof(float)),
        static_cast<uint64_t>(output.GetLength()),
        static_cast<uint64_t>(input.GetLength()),
        static_cast<uint8_t>(output.GetPosition()),
        static_cast<uint8_t>(input.GetPosition())};
    // check::TikcppLoaddata3dv2Check chkIns { "loaddata3dv2", chkParams };
    // bool flag = CheckFuncLoadData3dv2Impl(chkParams, "loaddata3dv2");

    // EXPECT_EQ(flag, param.expect);
}

// ============================================================
// LoadData3DParamsV1 NPU Debug validation - negative test cases
// Validation: dtype (uint8_t/int8_t/half), src position (L1), src alignment (32B),
//             dst alignment (512B if L0A/L0B, 32B if UB),
//             parameter ranges: l1H, l1W, c1Index, fetchFilterW/H, leftTopW/H,
//             strideW/H, filterW/H, dilationFilterW/H, jumpStride, repeatMode, cSize, repeatTime
// ============================================================

class TestLoadData3dv1NpuDebug : public testing::Test {
protected:
    void SetUp() { AscendC::SetGCoreType(2); }
    void TearDown()
    {
        AscendC::CheckSyncState();
        AscendC::SetGCoreType(0);
        GlobalMockObject::verify();
    }
};

// src position not L1 (using A2 instead)
TEST_F(TestLoadData3dv1NpuDebug, SrcPositionNotL1)
{
    TPipe tpipe;
    TBuf<TPosition::A2> srcBuf;
    tpipe.InitBuffer(srcBuf, 4096 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 4096 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV1<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 2;
    params.l1W = 56;
    params.c1Index = 0;
    params.fetchFilterW = 0;
    params.fetchFilterH = 0;
    params.leftTopW = 0;
    params.leftTopH = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.jumpStride = 1;
    params.repeatMode = 0;
    params.repeatTime = 1;
    params.cSize = 0;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// repeatTimes=0 (below MIN_LOAD3D_REPEAT_TIMES=1)
TEST_F(TestLoadData3dv1NpuDebug, RepeatTimesUnderflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 4096 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 4096 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV1<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 2;
    params.l1W = 56;
    params.c1Index = 0;
    params.fetchFilterW = 0;
    params.fetchFilterH = 0;
    params.leftTopW = 0;
    params.leftTopH = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.jumpStride = 1;
    params.repeatMode = 0;
    params.repeatTime = 0; // below min=1
    params.cSize = 0;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// l1H=0 (below MIN_LOAD3D_L1=1)
TEST_F(TestLoadData3dv1NpuDebug, L1HUnderflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 4096 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 4096 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV1<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 0; // below min=1
    params.l1W = 56;
    params.c1Index = 0;
    params.fetchFilterW = 0;
    params.fetchFilterH = 0;
    params.leftTopW = 0;
    params.leftTopH = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.jumpStride = 1;
    params.repeatMode = 0;
    params.repeatTime = 1;
    params.cSize = 0;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// repeatMode=2 (exceeds [0, 1])
TEST_F(TestLoadData3dv1NpuDebug, RepeatModeOverflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 4096 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 4096 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV1<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 2;
    params.l1W = 56;
    params.c1Index = 0;
    params.fetchFilterW = 0;
    params.fetchFilterH = 0;
    params.leftTopW = 0;
    params.leftTopH = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.jumpStride = 1;
    params.repeatMode = 2; // exceeds [0, 1]
    params.repeatTime = 1;
    params.cSize = 0;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// strideW=0 (below MIN_LOAD3D_STRIDE=1)
TEST_F(TestLoadData3dv1NpuDebug, StrideWUnderflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 4096 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 4096 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV1<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 2;
    params.l1W = 56;
    params.c1Index = 0;
    params.fetchFilterW = 0;
    params.fetchFilterH = 0;
    params.leftTopW = 0;
    params.leftTopH = 0;
    params.strideW = 0; // below min=1
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.jumpStride = 1;
    params.repeatMode = 0;
    params.repeatTime = 1;
    params.cSize = 0;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// l1W=0 (below MIN_LOAD3D_L1=1)
TEST_F(TestLoadData3dv1NpuDebug, L1WUnderflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 4096 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 4096 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV1<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 2;
    params.l1W = 0; // below min=1
    params.c1Index = 0;
    params.fetchFilterW = 0;
    params.fetchFilterH = 0;
    params.leftTopW = 0;
    params.leftTopH = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.jumpStride = 1;
    params.repeatMode = 0;
    params.repeatTime = 1;
    params.cSize = 0;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// c1Index=4096 (exceeds MAX_LOAD3D_C1_IDX=4095)
TEST_F(TestLoadData3dv1NpuDebug, C1IndexOverflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 4096 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 4096 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV1<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 2;
    params.l1W = 56;
    params.c1Index = 4096; // exceeds max=4095
    params.fetchFilterW = 0;
    params.fetchFilterH = 0;
    params.leftTopW = 0;
    params.leftTopH = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.jumpStride = 1;
    params.repeatMode = 0;
    params.repeatTime = 1;
    params.cSize = 0;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// filterW=0 (below MIN_LOAD3D_FILTER=1)
TEST_F(TestLoadData3dv1NpuDebug, FilterWUnderflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 4096 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 4096 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV1<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 2;
    params.l1W = 56;
    params.c1Index = 0;
    params.fetchFilterW = 0;
    params.fetchFilterH = 0;
    params.leftTopW = 0;
    params.leftTopH = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 0; // below min=1
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.jumpStride = 1;
    params.repeatMode = 0;
    params.repeatTime = 1;
    params.cSize = 0;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// dilationFilterW=0 (below MIN_LOAD3D_DILATION_FILTER=1)
TEST_F(TestLoadData3dv1NpuDebug, DilationFilterWUnderflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 4096 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 4096 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV1<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 2;
    params.l1W = 56;
    params.c1Index = 0;
    params.fetchFilterW = 0;
    params.fetchFilterH = 0;
    params.leftTopW = 0;
    params.leftTopH = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 0; // below min=1
    params.dilationFilterH = 1;
    params.jumpStride = 1;
    params.repeatMode = 0;
    params.repeatTime = 1;
    params.cSize = 0;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// jumpStride=0 (below MIN_LOAD3D_JUMP_STRIDE=1)
TEST_F(TestLoadData3dv1NpuDebug, JumpStrideUnderflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 4096 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 4096 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV1<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 2;
    params.l1W = 56;
    params.c1Index = 0;
    params.fetchFilterW = 0;
    params.fetchFilterH = 0;
    params.leftTopW = 0;
    params.leftTopH = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.jumpStride = 0; // below min=1
    params.repeatMode = 0;
    params.repeatTime = 1;
    params.cSize = 0;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// cSize=2 (exceeds [0, 1])
TEST_F(TestLoadData3dv1NpuDebug, CSizeOverflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 4096 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 4096 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV1<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 2;
    params.l1W = 56;
    params.c1Index = 0;
    params.fetchFilterW = 0;
    params.fetchFilterH = 0;
    params.leftTopW = 0;
    params.leftTopH = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.jumpStride = 1;
    params.repeatMode = 0;
    params.repeatTime = 1;
    params.cSize = 2; // exceeds [0, 1]
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// ============================================================
// LoadData3DParamsV2 NPU Debug validation - negative test cases
// Validation: src position (L1), src alignment (32B), dst alignment (512B if L0A/L0B, 32B if UB),
//             dtype based on dst position (A2 vs B2),
//             parameter ranges: l1H, l1W, kExtension, mExtension, kStartPt, mStartPt,
//             strideW/H, filterW/H, dilationFilterW/H, channelSize, enTranspose, enSmallK
// ============================================================

class TestLoadData3dv2NpuDebug : public testing::Test {
protected:
    void SetUp() { AscendC::SetGCoreType(2); }
    void TearDown()
    {
        AscendC::CheckSyncState();
        AscendC::SetGCoreType(0);
        GlobalMockObject::verify();
    }
};

// src position not L1 (using A2 instead)
TEST_F(TestLoadData3dv2NpuDebug, SrcPositionNotL1)
{
    TPipe tpipe;
    TBuf<TPosition::A2> srcBuf;
    tpipe.InitBuffer(srcBuf, 8192 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 8192 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV2<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 12;
    params.l1W = 16;
    params.channelSize = 32;
    params.kExtension = 32;
    params.mExtension = 192;
    params.kStartPt = 0;
    params.mStartPt = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.enTranspose = false;
    params.enSmallK = false;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// l1H=0 (below MIN_LOAD3D_L1=1)
TEST_F(TestLoadData3dv2NpuDebug, L1HUnderflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 8192 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 8192 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV2<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 0; // below min=1
    params.l1W = 16;
    params.channelSize = 32;
    params.kExtension = 32;
    params.mExtension = 192;
    params.kStartPt = 0;
    params.mStartPt = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.enTranspose = false;
    params.enSmallK = false;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// kExtension=0 (below MIN_LOAD3D_EXTENSION=1)
TEST_F(TestLoadData3dv2NpuDebug, KExtensionUnderflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 8192 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 8192 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV2<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 12;
    params.l1W = 16;
    params.channelSize = 32;
    params.kExtension = 0; // below min=1
    params.mExtension = 192;
    params.kStartPt = 0;
    params.mStartPt = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.enTranspose = false;
    params.enSmallK = false;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// strideW=0 (below MIN_LOAD3D_STRIDE=1)
TEST_F(TestLoadData3dv2NpuDebug, StrideWUnderflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 8192 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 8192 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV2<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 12;
    params.l1W = 16;
    params.channelSize = 32;
    params.kExtension = 32;
    params.mExtension = 192;
    params.kStartPt = 0;
    params.mStartPt = 0;
    params.strideW = 0; // below min=1
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.enTranspose = false;
    params.enSmallK = false;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// dst on B2 with int8_t type (B2 only supports half/bfloat16_t/float/uint32_t/int32_t, not int8_t)
TEST_F(TestLoadData3dv2NpuDebug, DstB2InvalidDtypeInt8)
{
    TPipe tpipe;
    TBuf<TPosition::B1> srcBuf;
    tpipe.InitBuffer(srcBuf, 8192 * sizeof(int8_t));
    LocalTensor<int8_t> srcLocal = srcBuf.Get<int8_t>();

    TBuf<TPosition::B2> dstBuf;
    tpipe.InitBuffer(dstBuf, 8192 * sizeof(int8_t));
    LocalTensor<int8_t> dstLocal = dstBuf.Get<int8_t>();

    LoadData3DParamsV2<int8_t> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 12;
    params.l1W = 16;
    params.channelSize = 32;
    params.kExtension = 32;
    params.mExtension = 192;
    params.kStartPt = 0;
    params.mStartPt = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.enTranspose = false;
    params.enSmallK = false;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<int8_t>(dstLocal, srcLocal, params);
}

// l1W=0 (below MIN_LOAD3D_L1=1)
TEST_F(TestLoadData3dv2NpuDebug, L1WUnderflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 8192 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 8192 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV2<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 12;
    params.l1W = 0; // below min=1
    params.channelSize = 32;
    params.kExtension = 32;
    params.mExtension = 192;
    params.kStartPt = 0;
    params.mStartPt = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.enTranspose = false;
    params.enSmallK = false;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// mExtension=0 (below MIN_LOAD3D_EXTENSION=1)
TEST_F(TestLoadData3dv2NpuDebug, MExtensionUnderflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 8192 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 8192 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV2<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 12;
    params.l1W = 16;
    params.channelSize = 32;
    params.kExtension = 32;
    params.mExtension = 0; // below min=1
    params.kStartPt = 0;
    params.mStartPt = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.enTranspose = false;
    params.enSmallK = false;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// filterW=0 (below MIN_LOAD3D_FILTER=1)
TEST_F(TestLoadData3dv2NpuDebug, FilterWUnderflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 8192 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 8192 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV2<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 12;
    params.l1W = 16;
    params.channelSize = 32;
    params.kExtension = 32;
    params.mExtension = 192;
    params.kStartPt = 0;
    params.mStartPt = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 0; // below min=1
    params.filterH = 1;
    params.dilationFilterW = 1;
    params.dilationFilterH = 1;
    params.enTranspose = false;
    params.enSmallK = false;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}

// dilationFilterW=0 (below MIN_LOAD3D_DILATION_FILTER=1)
TEST_F(TestLoadData3dv2NpuDebug, DilationFilterWUnderflow)
{
    TPipe tpipe;
    TBuf<TPosition::A1> srcBuf;
    tpipe.InitBuffer(srcBuf, 8192 * sizeof(half));
    LocalTensor<half> srcLocal = srcBuf.Get<half>();

    TBuf<TPosition::A2> dstBuf;
    tpipe.InitBuffer(dstBuf, 8192 * sizeof(half));
    LocalTensor<half> dstLocal = dstBuf.Get<half>();

    LoadData3DParamsV2<half> params;
    params.padList[0] = 0;
    params.padList[1] = 0;
    params.padList[2] = 0;
    params.padList[3] = 0;
    params.l1H = 12;
    params.l1W = 16;
    params.channelSize = 32;
    params.kExtension = 32;
    params.mExtension = 192;
    params.kStartPt = 0;
    params.mStartPt = 0;
    params.strideW = 1;
    params.strideH = 1;
    params.filterW = 1;
    params.filterH = 1;
    params.dilationFilterW = 0; // below min=1
    params.dilationFilterH = 1;
    params.enTranspose = false;
    params.enSmallK = false;
    params.padValue = 0;

    MOCKER(raise, int32_t(*)(int32_t)).stubs().will(invoke(RaiseStub));
    LoadData<half>(dstLocal, srcLocal, params);
}
