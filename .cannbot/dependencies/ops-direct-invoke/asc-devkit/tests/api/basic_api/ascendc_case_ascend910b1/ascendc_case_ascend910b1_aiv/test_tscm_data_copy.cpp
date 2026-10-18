/**
 * Copyright (c) 2025 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include <cstring>
#include <gtest/gtest.h>
#include "mockcpp/mockcpp.hpp"
#include "kernel_tpipe_impl.h"
#include "kernel_operator.h"

namespace AscendC {
class TscmRealMsgTestSuite : public testing::Test {
protected:
    static constexpr size_t WORKSPACE_SIZE = 16 * 1024 * 1024;
    uint8_t* workspace = nullptr;
    TPipe pipe;
    TSCM<TPosition::VECIN, 1> inQueueTscm;
    int32_t coreTypeBak = 0;

    void SetUp() override
    {
        workspace = new uint8_t[WORKSPACE_SIZE];
        std::memset(workspace, 0, WORKSPACE_SIZE);
        pipe.InitBuffer(inQueueTscm, 1, 512);
        coreTypeBak = g_coreType;
        SetGCoreType(2);
    }

    void TearDown() override
    {
        g_kfcClient = nullptr;
        SetGCoreType(coreTypeBak);
        delete[] workspace;
        GlobalMockObject::verify();
    }
};

TEST_F(TscmRealMsgTestSuite, ScmDataCopyMsgWritesRealKfcMessage)
{
    KfcCommClient client(workspace, 0);
    g_kfcClient = &client;
    uint8_t src[256] = {0};
    DataCopyParams intriParams{2, 4, 6, 8};
    auto dst = reinterpret_cast<__cbuf__ void*>(static_cast<uintptr_t>(0x40));

    ScmDataCopyMsg(dst, src, intriParams, 123);

    auto* msg = client.ubMsg;
    auto* body = reinterpret_cast<const Gm2L1Params*>(msg->buffer);
    EXPECT_EQ(KfcMsgGetFunID(msg->head), KFC_Enum::SCMFUN_GM2L1);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(body->dst), reinterpret_cast<uintptr_t>(dst));
    EXPECT_EQ(reinterpret_cast<uintptr_t>(body->src), reinterpret_cast<uintptr_t>(src));
    EXPECT_EQ(body->intri.blockCount, intriParams.blockCount);
    EXPECT_EQ(body->intri.blockLen, intriParams.blockLen);
    EXPECT_EQ(body->intri.srcStride, intriParams.srcStride);
    EXPECT_EQ(body->intri.dstStride, intriParams.dstStride);
    EXPECT_EQ(msg->ubAddr, 123);
}

TEST_F(TscmRealMsgTestSuite, ScmDataCopyNd2NzWritesRealKfcMessage)
{
    KfcCommClient client(workspace, 0);
    g_kfcClient = &client;
    uint8_t src[256] = {0};
    Nd2NzParams intriParams{1, 32, 32, 0, 32, 32, 1, 0};
    auto dst = reinterpret_cast<__cbuf__ void*>(static_cast<uintptr_t>(0x80));

    ScmDataCopyND2NZMsg(dst, src, sizeof(half), intriParams, 456);

    auto* msg = client.ubMsg;
    auto* body = reinterpret_cast<const Gm2L1Nd2NzParams*>(msg->buffer);
    EXPECT_EQ(KfcMsgGetFunID(msg->head), KFC_Enum::SCMFUN_GM2L1ND2NZ);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(body->dst), reinterpret_cast<uintptr_t>(dst));
    EXPECT_EQ(reinterpret_cast<uintptr_t>(body->src), reinterpret_cast<uintptr_t>(src));
    EXPECT_EQ(body->dataTypeLen, sizeof(half));
    EXPECT_EQ(body->intri.ndNum, intriParams.ndNum);
    EXPECT_EQ(body->intri.nValue, intriParams.nValue);
    EXPECT_EQ(body->intri.dValue, intriParams.dValue);
    EXPECT_EQ(body->intri.srcNdMatrixStride, intriParams.srcNdMatrixStride);
    EXPECT_EQ(body->intri.srcDValue, intriParams.srcDValue);
    EXPECT_EQ(body->intri.dstNzC0Stride, intriParams.dstNzC0Stride);
    EXPECT_EQ(body->intri.dstNzNStride, intriParams.dstNzNStride);
    EXPECT_EQ(body->intri.dstNzMatrixStride, intriParams.dstNzMatrixStride);
    EXPECT_EQ(msg->ubAddr, 456);
}

TEST_F(TscmRealMsgTestSuite, ScmDataCopyMsgRequiresRegisteredMatmulObject)
{
    uint8_t src[256] = {0};
    DataCopyParams intriParams{2, 4, 6, 8};
    auto dst = reinterpret_cast<__cbuf__ void*>(static_cast<uintptr_t>(0x40));

    EXPECT_DEATH({ ScmDataCopyMsg(dst, src, intriParams, 123); }, "");
}

TEST_F(TscmRealMsgTestSuite, DataCopyGmToTscmImplSendsOffsetMessage)
{
    KfcCommClient client(workspace, 0);
    g_kfcClient = &client;
    uint8_t src[256] = {0};
    DataCopyParams intriParams{2, 4, 6, 8};
    auto* tscmBase = GetBaseAddrCpu(static_cast<int8_t>(TPosition::TSCM));
    ASSERT_NE(tscmBase, nullptr);
    auto* dst = reinterpret_cast<__cbuf__ half*>(tscmBase + 0x40);

    DataCopyGM2L1Impl(dst, reinterpret_cast<__gm__ half*>(src), intriParams);

    auto* msg = client.ubMsg;
    auto* body = reinterpret_cast<const Gm2L1Params*>(msg->buffer);
    EXPECT_EQ(KfcMsgGetFunID(msg->head), KFC_Enum::SCMFUN_GM2L1);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(body->dst), static_cast<uintptr_t>(0x40));
    EXPECT_EQ(reinterpret_cast<uintptr_t>(body->src), reinterpret_cast<uintptr_t>(src));
    EXPECT_EQ(msg->ubAddr, -1);
}

TEST_F(TscmRealMsgTestSuite, DataCopyGmToTscmNd2NzImplSendsOffsetMessage)
{
    KfcCommClient client(workspace, 0);
    g_kfcClient = &client;
    uint8_t src[512] = {0};
    Nd2NzParams intriParams{1, 16, 16, 16, 16, 16, 1, 16};
    auto* tscmBase = GetBaseAddrCpu(static_cast<int8_t>(TPosition::TSCM));
    ASSERT_NE(tscmBase, nullptr);
    auto* dst = reinterpret_cast<__cbuf__ half*>(tscmBase + 0x80);

    DataCopyGM2L1ND2NZImpl(dst, reinterpret_cast<__gm__ half*>(src), intriParams);

    auto* msg = client.ubMsg;
    auto* body = reinterpret_cast<const Gm2L1Nd2NzParams*>(msg->buffer);
    EXPECT_EQ(KfcMsgGetFunID(msg->head), KFC_Enum::SCMFUN_GM2L1ND2NZ);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(body->dst), static_cast<uintptr_t>(0x80));
    EXPECT_EQ(reinterpret_cast<uintptr_t>(body->src), reinterpret_cast<uintptr_t>(src));
    EXPECT_EQ(body->dataTypeLen, sizeof(half));
    EXPECT_EQ(msg->ubAddr, -1);
}

TEST_F(TscmRealMsgTestSuite, DataCopyUbToTscmImplUsesUbMappedSource)
{
    KfcCommClient client(workspace, 0);
    g_kfcClient = &client;
    DataCopyParams intriParams{1, 1, 0, 0};
    auto* ubBase = GetBaseAddrCpu(static_cast<int8_t>(TPosition::VECIN));
    auto* tscmBase = GetBaseAddrCpu(static_cast<int8_t>(TPosition::TSCM));
    ASSERT_NE(ubBase, nullptr);
    ASSERT_NE(tscmBase, nullptr);
    auto* src = reinterpret_cast<__ubuf__ half*>(ubBase + 0x100);
    auto* dst = reinterpret_cast<__cbuf__ half*>(tscmBase + 0x120);

    DataCopyUB2L1Impl(dst, src, intriParams);

    auto* msg = client.ubMsg;
    auto* body = reinterpret_cast<const Gm2L1Params*>(msg->buffer);
    EXPECT_EQ(KfcMsgGetFunID(msg->head), KFC_Enum::SCMFUN_GM2L1);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(body->dst), static_cast<uintptr_t>(0x120));
    EXPECT_EQ(reinterpret_cast<uintptr_t>(body->src), reinterpret_cast<uintptr_t>(client.ubStart + 0x100));
    EXPECT_EQ(msg->ubAddr, -1);
}

TEST_F(TscmRealMsgTestSuite, DataCopyUbToTscmNd2NzImplAllocatesTransferBuffer)
{
    KfcCommClient client(workspace, 0);
    g_kfcClient = &client;
    Nd2NzParams intriParams{1, 16, 16, 16, 16, 16, 1, 16};
    auto* ubBase = GetBaseAddrCpu(static_cast<int8_t>(TPosition::VECIN));
    auto* tscmBase = GetBaseAddrCpu(static_cast<int8_t>(TPosition::TSCM));
    ASSERT_NE(ubBase, nullptr);
    ASSERT_NE(tscmBase, nullptr);
    auto* src = reinterpret_cast<__ubuf__ half*>(ubBase + 0x140);
    auto* dst = reinterpret_cast<__cbuf__ half*>(tscmBase + 0x160);

    DataCopyUB2L1ND2NZImpl(dst, src, intriParams);

    auto* msg = client.ubMsg;
    auto* body = reinterpret_cast<const Gm2L1Nd2NzParams*>(msg->buffer);
    EXPECT_EQ(KfcMsgGetFunID(msg->head), KFC_Enum::SCMFUN_GM2L1ND2NZ);
    EXPECT_EQ(reinterpret_cast<uintptr_t>(body->dst), static_cast<uintptr_t>(0x160));
    EXPECT_EQ(body->dataTypeLen, sizeof(half));
    EXPECT_GE(msg->ubAddr, 0);
}

TEST_F(TscmRealMsgTestSuite, DataCopyPadUbToTscmNd2NzImplSupportsLegacyAndExtParams)
{
    KfcCommClient client(workspace, 0);
    g_kfcClient = &client;
    Nd2NzParams nd2nzParams{1, 16, 16, 16, 16, 16, 1, 16};
    auto* ubBase = GetBaseAddrCpu(static_cast<int8_t>(TPosition::VECIN));
    auto* tscmBase = GetBaseAddrCpu(static_cast<int8_t>(TPosition::TSCM));
    ASSERT_NE(ubBase, nullptr);
    ASSERT_NE(tscmBase, nullptr);
    auto* src = reinterpret_cast<__ubuf__ half*>(ubBase + 0x200);
    auto* dst = reinterpret_cast<__cbuf__ half*>(tscmBase + 0x220);

    DataCopyParams dataCopyParams{1, 1, 0, 0};
    DataCopyPadUB2L1Impl(dst, src, dataCopyParams, nd2nzParams);
    EXPECT_EQ(KfcMsgGetFunID(client.ubMsg->head), KFC_Enum::SCMFUN_GM2L1ND2NZ);

    DataCopyExtParams dataCopyExtParams{1, 32, 0, 0, 0};
    DataCopyPadUB2L1Impl(dst, src, dataCopyExtParams, nd2nzParams);
    EXPECT_EQ(KfcMsgGetFunID(client.ubMsg->head), KFC_Enum::SCMFUN_GM2L1ND2NZ);
}
} // namespace AscendC
