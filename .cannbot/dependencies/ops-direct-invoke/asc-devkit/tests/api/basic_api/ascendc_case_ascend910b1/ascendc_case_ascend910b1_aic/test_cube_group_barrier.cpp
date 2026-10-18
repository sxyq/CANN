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

namespace {
struct TestCubeGroupMsg {
    CubeGroupMsgHead head;
    uint8_t payload[62];
};

static_assert(sizeof(TestCubeGroupMsg) == 64, "Cube group message must be 64-byte aligned.");

class CubeGroupBarrierTestSuite : public testing::Test {
protected:
    void TearDown() override
    {
        CheckSyncState();
        SetGCoreType(MIX_TYPE);
        GlobalMockObject::verify();
    }
};

TEST_F(CubeGroupBarrierTestSuite, KfcWorkspaceUpdatesWorkspaceAndEventOnAiv)
{
    uint8_t workspace[sizeof(TestCubeGroupMsg) * MAX_MSG_PER_AIV] = {0};
    SetGCoreType(AIV_TYPE);
    TPipe tpipe;

    {
        KfcWorkspace desc(workspace);
        EXPECT_EQ(desc.GetKfcWorkspace(), workspace);
        desc.UpdateKfcWorkspace(sizeof(TestCubeGroupMsg));
        EXPECT_EQ(desc.GetKfcWorkspace(), workspace + sizeof(TestCubeGroupMsg));
        EXPECT_LT(static_cast<uint32_t>(GetEventId(desc)), 16U);
    }
}

TEST_F(CubeGroupBarrierTestSuite, CubeResGroupHandleReceiveAndFreeMessage)
{
    uint8_t workspace[sizeof(TestCubeGroupMsg) * MAX_MSG_PER_AIV] = {0};
    auto* msg = reinterpret_cast<__gm__ TestCubeGroupMsg*>(workspace);
    msg->head.msgState = CubeMsgState::VALID;
    msg->head.aivID = 0;

    SetGCoreType(AIC_TYPE);
    TPipe tpipe;
    MOCKER_CPP(GetBlockIdxImpl).stubs().will(returnValue(0));
    CubeResGroupHandle<TestCubeGroupMsg> handle(workspace, 0, 2, 1, EVENT_ID0);

    EXPECT_TRUE(__IsRun(handle));
    EXPECT_EQ(__RcvMessage(handle), msg);
    EXPECT_EQ(handle.FreeMessage(msg), 0);
    EXPECT_EQ(static_cast<CubeMsgState>(msg->head.msgState), CubeMsgState::FREE);

    msg->head.msgState = CubeMsgState::QUIT;
    msg->head.aivID = 0;
    __SetAivQuit(&handle, 0);
    EXPECT_FALSE(__IsRun(handle));
}

TEST_F(CubeGroupBarrierTestSuite, GroupBarrierSingleAivArriveCompletes)
{
    uint8_t workspace[BARRIER_SIZE * 2 + CACHE_LINE_LEN * 2] = {0};
    SetGCoreType(AIV_TYPE);
    TPipe tpipe;

    GroupBarrier<PipeMode::MTE3_MODE> barrier(workspace, 1, 1);
    EXPECT_EQ(barrier.GetWorkspaceLen(), CACHE_LINE_LEN);
    barrier.Arrive(0);
}
} // namespace
