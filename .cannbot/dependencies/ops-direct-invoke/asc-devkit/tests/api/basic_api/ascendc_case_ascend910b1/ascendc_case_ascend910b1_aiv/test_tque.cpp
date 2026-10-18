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
#include <fstream>
#include <iostream>
#include "kernel_operator.h"
#include "mockcpp/mockcpp.hpp"

using namespace std;
using namespace AscendC;

class TEST_TSCM : public testing::Test {
protected:
    void SetUp() { AscendC::SetGCoreType(AIV_TYPE); }
    void TearDown()
    {
        AscendC::CheckSyncState();
        AscendC::SetGCoreType(0);
        GlobalMockObject::verify();
    }
};

void AbortStub1() {}
TEST_F(TEST_TSCM, TEST_ALLOC_AND_FREE_BUFFER)
{
    static constexpr TPosition tpTscm[8] = {TPosition::MAX, TPosition::MAX, TPosition::MAX, TPosition::MAX,
                                            TPosition::MAX, TPosition::MAX, TPosition::MAX, TPosition::MAX};
    static constexpr TQueConfig confTscm = GetTQueConfig(false, false, false, 0, 0, 0, tpTscm, false, true);
    int32_t tmpCore = g_coreType;
    g_coreType = AscendC::AIV_TYPE;
    TPipe pipe;
    TSCM<TPosition::VECIN, 1, &confTscm> scmUb;
    TSCM<TPosition::GM, 1, &confTscm> scmGm;
    pipe.InitBuffer(scmUb, 1, 1024);
    pipe.InitBuffer(scmGm, 1, 1024);
    auto tmp = scmUb.AllocTensor<float>();
    auto tmp1 = scmGm.AllocTensor<float>();
    EXPECT_EQ(scmUb.bufUsedCount, 1);
    EXPECT_EQ(scmGm.bufUsedCount, 1);
    scmUb.FreeTensor(tmp);
    scmGm.FreeTensor(tmp1);
    g_coreType = tmpCore;
}

TEST_F(TEST_TSCM, TEST_ALLOC_TOO_MUCH_BUFFER_IN_ONE_INITBUFFER)
{
    g_coreType = AscendC::AIV_TYPE;
    TPipe pipe;
    TQue<TPosition::VECIN, 1> tque1;
    MOCKER(raise, int (*)(int)).times(2).will(returnValue(0));

    static int32_t count = 0;
    std::string fileName = "print_ut_aiv_init_buffer" + std::to_string(getpid()) + "_" + std::to_string(count) + ".txt";
    freopen(fileName.c_str(), "w", stdout);

    pipe.InitBuffer(tque1, 65, 1024);

    // 恢复printf
    fclose(stdout);
    freopen("/dev/tty", "w", stdout);
    freopen("/dev/tty", "r", stdin);

    // 校验真值
    std::ifstream resultFile(fileName, std::ios::in);
    std::stringstream streambuffer;
    streambuffer << resultFile.rdbuf();
    std::string resultString(streambuffer.str());
    std::string goldenStr = "Failed to check num value in InitBuffer, its valid range is 1 ~ 64, current value is 65.";
    resultFile.close();
    std::cout << "resultString is " << resultString << std::endl;
    std::cout << "goldenStr is " << goldenStr << std::endl;
    EXPECT_TRUE(resultString.find(goldenStr) != std::string::npos);
    EXPECT_EQ(remove(fileName.c_str()), 0);
}

// 预计分配到65会挂
TEST_F(TEST_TSCM, TEST_ALLOC_MANY_BUFFER_IN_MULTIPLE_INITBUFFER)
{
    g_coreType = AscendC::AIV_TYPE;
    TPipe pipe;
    TQue<TPosition::VECIN, 1> tque1;
    TQue<TPosition::VECIN, 1> tque2;
    pipe.InitBuffer(tque1, 32, 1024);
    pipe.InitBuffer(tque2, 32, 1024);
}

TEST_F(TEST_TSCM, TEST_LOOP_QUEUE_ALLOC_FREE_SAME_EVT)
{
    static constexpr TPosition tp[8] = {TPosition::MAX, TPosition::MAX, TPosition::MAX, TPosition::MAX,
                                        TPosition::MAX, TPosition::MAX, TPosition::MAX, TPosition::MAX};
    static constexpr TQueConfig confLoopQueue = GetTQueConfig(false, false, false, 0, 0, 0, tp, false, true);
    g_coreType = AscendC::AIV_TYPE;
    TPipe pipe;
    // VECIN: src=GM, dst=VECIN, freeBufEvt=V_MTE2, depth=2
    TQueBind<TPosition::GM, TPosition::VECIN, 2, &confLoopQueue> tque;
    pipe.InitBuffer(tque, 2, 1024);

    // Allocate both buffers and free them, so that FreeBuffer sets freeBufEvt/freeBufEvtID
    auto tensor1 = tque.AllocTensor<float>();
    auto tensor2 = tque.AllocTensor<float>();
    EXPECT_EQ(tque.bufUsedCount, 2);

    tque.EnQue(tensor1);
    auto dequeTensor1 = tque.DeQue<float>();
    tque.FreeTensor(dequeTensor1);

    tque.EnQue(tensor2);
    auto dequeTensor2 = tque.DeQue<float>();
    tque.FreeTensor(dequeTensor2);
    EXPECT_EQ(tque.bufUsedCount, 0);

    auto tensor3 = tque.AllocTensor<float>();
    EXPECT_EQ(tque.bufUsedCount, 1);
    tque.EnQue(tensor3);
    auto dequeTensor3 = tque.DeQue<float>();
    tque.FreeTensor(dequeTensor3);
}

// Cover kernel_tquebind_impl.h line 588-595:
// enableLoopQueue=true, freeBufEvt(V_MTE2) != ret->freeBufEvt(MTE3_V)
// This simulates the case where a buffer freed by a VECOUT queue (freeBufEvt=MTE3_V)
// is re-allocated by a VECIN queue (freeBufEvt=V_MTE2)
TEST_F(TEST_TSCM, TEST_LOOP_QUEUE_ALLOC_FREE_DIFF_EVT_V_MTE2_MTE3_V)
{
    static constexpr TPosition tp[8] = {TPosition::MAX, TPosition::MAX, TPosition::MAX, TPosition::MAX,
                                        TPosition::MAX, TPosition::MAX, TPosition::MAX, TPosition::MAX};
    static constexpr TQueConfig confLoopQueue = GetTQueConfig(false, false, false, 0, 0, 0, tp, false, true);
    g_coreType = AscendC::AIV_TYPE;
    TPipe pipe;
    // VECIN: src=GM, dst=VECIN, freeBufEvt=V_MTE2
    TQueBind<TPosition::GM, TPosition::VECIN, 2, &confLoopQueue> tqueVecIn;
    // VECOUT: src=VECOUT, dst=GM, freeBufEvt=MTE3_V
    TQueBind<TPosition::VECOUT, TPosition::GM, 2, &confLoopQueue> tqueVecOut;
    pipe.InitBuffer(tqueVecIn, 1, 1024);
    pipe.InitBuffer(tqueVecOut, 1, 1024);

    // Use VECOUT queue: AllocTensor -> EnQue -> DeQue -> FreeTensor
    // This will set freeBufEvtID from MTE3_V pool and freeBufEvt=MTE3_V
    auto tensorOut = tqueVecOut.AllocTensor<float>();
    tqueVecOut.EnQue(tensorOut);
    auto dequeOut = tqueVecOut.DeQue<float>();
    tqueVecOut.FreeTensor(dequeOut);

    // Manually make VECIN queue's buffer point to VECOUT's freed buffer state
    // Copy the VECOUT buffer's freeBufEvtID (from MTE3_V pool) and freeBufEvt to VECIN's buffer
    struct TBufType* outBufPtr = tqueVecOut.bufStart;
    struct TBufType* inBufPtr = tqueVecIn.bufStart;
    inBufPtr->freeBufEvtID = outBufPtr->freeBufEvtID;
    inBufPtr->freeBufEvt = HardEvent::MTE3_V;
    inBufPtr->state = TBufState::FREE;

    // Reset VECOUT's buffer to avoid double-release
    outBufPtr->freeBufEvtID = INVALID_TEVENTID;
    outBufPtr->state = TBufState::FREE;

    // Now use VECIN queue: AllocTensor -> EnQue -> DeQue -> FreeTensor
    // AllocBuffer will find freeBufEvt(V_MTE2) != ret->freeBufEvt(MTE3_V),
    // hitting line 588: freeBufEvt == HardEvent::V_MTE2 && ret->freeBufEvt == HardEvent::MTE3_V
    auto tensor2 = tqueVecIn.AllocTensor<float>();
    EXPECT_EQ(tqueVecIn.bufUsedCount, 1);
    tqueVecIn.EnQue(tensor2);
    auto deque2 = tqueVecIn.DeQue<float>();
    tqueVecIn.FreeTensor(deque2);
}

// enableLoopQueue=true, freeBufEvt(MTE3_V) != ret->freeBufEvt(V_MTE2)
// This simulates the case where a buffer freed by a VECIN queue (freeBufEvt=V_MTE2)
// is re-allocated by a VECOUT queue (freeBufEvt=MTE3_V)
TEST_F(TEST_TSCM, TEST_LOOP_QUEUE_ALLOC_FREE_DIFF_EVT_MTE3_V_V_MTE2)
{
    static constexpr TPosition tp[8] = {TPosition::MAX, TPosition::MAX, TPosition::MAX, TPosition::MAX,
                                        TPosition::MAX, TPosition::MAX, TPosition::MAX, TPosition::MAX};
    static constexpr TQueConfig confLoopQueue = GetTQueConfig(false, false, false, 0, 0, 0, tp, false, true);
    g_coreType = AscendC::AIV_TYPE;
    TPipe pipe;
    // VECIN: src=GM, dst=VECIN, freeBufEvt=V_MTE2
    TQueBind<TPosition::GM, TPosition::VECIN, 2, &confLoopQueue> tqueVecIn;
    // VECOUT: src=VECOUT, dst=GM, freeBufEvt=MTE3_V
    TQueBind<TPosition::VECOUT, TPosition::GM, 2, &confLoopQueue> tqueVecOut;
    pipe.InitBuffer(tqueVecIn, 1, 1024);
    pipe.InitBuffer(tqueVecOut, 1, 1024);

    // Use VECIN queue: AllocTensor -> EnQue -> DeQue -> FreeTensor
    // This will set freeBufEvtID from V_MTE2 pool and freeBufEvt=V_MTE2
    auto tensorIn = tqueVecIn.AllocTensor<float>();
    tqueVecIn.EnQue(tensorIn);
    auto dequeIn = tqueVecIn.DeQue<float>();
    tqueVecIn.FreeTensor(dequeIn);

    // Manually make VECOUT queue's buffer point to VECIN's freed buffer state
    // Copy the VECIN buffer's freeBufEvtID (from V_MTE2 pool) and freeBufEvt to VECOUT's buffer
    struct TBufType* inBufPtr = tqueVecIn.bufStart;
    struct TBufType* outBufPtr = tqueVecOut.bufStart;
    outBufPtr->freeBufEvtID = inBufPtr->freeBufEvtID;
    outBufPtr->freeBufEvt = HardEvent::V_MTE2;
    outBufPtr->state = TBufState::FREE;

    // Reset VECIN's buffer to avoid double-release
    inBufPtr->freeBufEvtID = INVALID_TEVENTID;
    inBufPtr->state = TBufState::FREE;

    // Now use VECOUT queue: AllocTensor -> EnQue -> DeQue -> FreeTensor
    // AllocBuffer will find freeBufEvt(MTE3_V) != ret->freeBufEvt(V_MTE2),
    auto tensor2 = tqueVecOut.AllocTensor<float>();
    EXPECT_EQ(tqueVecOut.bufUsedCount, 1);
    tqueVecOut.EnQue(tensor2);
    auto deque2 = tqueVecOut.DeQue<float>();
    tqueVecOut.FreeTensor(deque2);
}
