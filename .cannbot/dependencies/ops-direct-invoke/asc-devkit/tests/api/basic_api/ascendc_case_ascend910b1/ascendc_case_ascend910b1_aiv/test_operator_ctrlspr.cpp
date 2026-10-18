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
#include "kernel_operator.h"
#include "mockcpp/mockcpp.hpp"

using namespace std;
using namespace AscendC;

template <int8_t startBit, int8_t endBit>
void CtrlSprKernel()
{
    int64_t initValue = GetCtrlSpr<startBit, endBit>();
    EXPECT_EQ(initValue, 0x00);
    bool res = GetSaturationFlag<SaturationMode::FLOAT>();
    EXPECT_EQ(res, true);

    SetCtrlSpr<startBit, endBit>(1);
    initValue = GetCtrlSpr<startBit, endBit>();
    EXPECT_EQ(initValue, 0x01);
    res = GetSaturationFlag<SaturationMode::FLOAT>();
    EXPECT_EQ(res, false);

    SetSaturationFlag<SaturationMode::FLOAT>(true);
    initValue = GetCtrlSpr<startBit, endBit>();
    EXPECT_EQ(initValue, 0x00);
    res = GetSaturationFlag<SaturationMode::FLOAT>();
    EXPECT_EQ(res, true);

    SetSaturationFlag<SaturationMode::CAST>(false);
    res = GetSaturationFlag<SaturationMode::CAST>();
    EXPECT_EQ(res, false);
    SetSaturationFlag<SaturationMode::CAST>(true);
    res = GetSaturationFlag<SaturationMode::CAST>();
    EXPECT_EQ(res, true);

    SetSaturationFlag<SaturationMode::INT>(false);
    res = GetSaturationFlag<SaturationMode::INT>();
    EXPECT_EQ(res, false);
    SetSaturationFlag<SaturationMode::INT>(true);
    res = GetSaturationFlag<SaturationMode::INT>();
    EXPECT_EQ(res, true);
}

struct CtrlSprTestParams {
    void (*calFunc)();
};

class CtrlSprTestsuite : public testing::Test, public testing::WithParamInterface<CtrlSprTestParams> {
protected:
    void SetUp() { AscendC::SetGCoreType(2); }
    void TearDown()
    {
        AscendC::SetGCoreType(0);
        GlobalMockObject::verify();
    }
};

INSTANTIATE_TEST_CASE_P(TEST_CTRL_SPR, CtrlSprTestsuite, ::testing::Values(CtrlSprTestParams{CtrlSprKernel<48, 48>}));

TEST_P(CtrlSprTestsuite, CtrlSprTestCase)
{
    auto param = GetParam();
    param.calFunc();
}
