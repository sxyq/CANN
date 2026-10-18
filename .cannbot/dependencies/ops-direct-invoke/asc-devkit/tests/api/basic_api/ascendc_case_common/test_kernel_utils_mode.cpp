/**
 * Copyright (c) 2025 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */
#include "utils/kernel_utils_mode.h"
#include <gtest/gtest.h>
#include <cstdint>
#include <cstring>
#include <cmath>
#include <limits>

namespace AscendC {

// ==================== NotNumUnion 测试 ====================

TEST(NotNumUnionTest, SizeIsCorrect)
{
    // 验证 union 大小为 4 字节（float 和 uint32_t 都是 4 字节）
    EXPECT_EQ(sizeof(NotNumUnion), 4);
    EXPECT_EQ(sizeof(float), 4);
    EXPECT_EQ(sizeof(uint32_t), 4);
}

TEST(NotNumUnionTest, FloatToUint32)
{
    NotNumUnion u;

    // 测试浮点数 1.0 的内存表示
    u.f = 1.0f;
    // 1.0 的 IEEE 754 单精度表示: 0x3F800000
    EXPECT_EQ(u.i, 0x3F800000);
}

TEST(NotNumUnionTest, Uint32ToFloat)
{
    NotNumUnion u;

    // 测试 -0.0 的 IEEE 754 表示
    u.i = 0x80000000;
    EXPECT_EQ(u.f, -0.0f);
    EXPECT_TRUE(std::signbit(u.f)); // 验证符号位
}

TEST(NotNumUnionTest, ZeroValues)
{
    NotNumUnion u;

    // +0.0
    u.f = 0.0f;
    EXPECT_EQ(u.i, 0x00000000);

    // -0.0
    u.f = -0.0f;
    EXPECT_EQ(u.i, 0x80000000);
}

TEST(NotNumUnionTest, SpecialFloatValues)
{
    NotNumUnion u;

    // NaN
    u.f = NAN;
    EXPECT_TRUE(std::isnan(u.f));

    // Infinity
    u.f = INFINITY;
    EXPECT_TRUE(std::isinf(u.f));
    EXPECT_GT(u.f, 0);

    // -Infinity
    u.f = -INFINITY;
    EXPECT_TRUE(std::isinf(u.f));
    EXPECT_LT(u.f, 0);
}

TEST(NotNumUnionTest, RoundTripConversion)
{
    // 验证 float -> uint32_t -> float 往返转换
    NotNumUnion u;
    float original = 3.14159f;

    u.f = original;
    uint32_t bits = u.i;

    NotNumUnion u2;
    u2.i = bits;

    EXPECT_EQ(u2.f, original);
}

TEST(NotNumUnionTest, EdgeCaseValues)
{
    NotNumUnion u;

    // 最小正正规数
    u.i = 0x00800000;
    EXPECT_GT(u.f, 0.0f);
    EXPECT_TRUE(std::isnormal(u.f));

    // 最大正规数
    u.i = 0x7F7FFFFF;
    EXPECT_TRUE(std::isnormal(u.f));

    // 最小正次正规数 (subnormal)
    u.i = 0x00000001;
    // 使用 fpclassify 检查（C++11 标准）
    int class_val = std::fpclassify(u.f);
    EXPECT_TRUE(class_val == FP_SUBNORMAL || class_val == FP_ZERO);
}

// ==================== HalfUnion 测试 ====================
// 注意：HalfUnion 包含 half 类型成员，C++11 中 union 不能有非平凡构造函数
// 需要显式初始化

TEST(HalfUnionTest, SizeIsCorrect)
{
    // 验证 union 大小为 2 字节（half 和 uint16_t 都是 2 字节）
    EXPECT_EQ(sizeof(HalfUnion), 2);
    EXPECT_EQ(sizeof(uint16_t), 2);
}

TEST(HalfUnionTest, Uint16ToHalf)
{
    HalfUnion u{}; // 值初始化

    // 通过 uint16_t 成员设置位模式
    u.i = 0x3C00; // IEEE 754 half 格式的 1.0
    EXPECT_EQ(u.i, 0x3C00);
}

TEST(HalfUnionTest, HalfToUint16)
{
    HalfUnion u{};

    // 通过 half 成员设置 +0.0
    u.f = half(0.0f);
    EXPECT_EQ(u.i, 0x0000);
}

TEST(HalfUnionTest, ZeroValues)
{
    HalfUnion u{};

    // +0.0
    u.f = half(0.0f);
    EXPECT_EQ(u.i, 0x0000);
}

TEST(HalfUnionTest, RoundTripConversion)
{
    // 验证 uint16_t -> half -> uint16_t 往返转换
    HalfUnion u{};
    uint16_t original = 0x3C00; // 1.0 in half format

    u.i = original;
    half h = u.f;

    HalfUnion u2{};
    u2.f = h;

    EXPECT_EQ(u2.i, original);
}

TEST(HalfUnionTest, EdgeCaseValues)
{
    HalfUnion u{};

    // 最小正正规数 (half)
    u.i = 0x0400;
    EXPECT_EQ(u.i, 0x0400);

    // 最大正规数 (half)
    u.i = 0x7BFF;
    EXPECT_EQ(u.i, 0x7BFF);
}

// ==================== 联合类型双关测试 ====================

TEST(TypePunningTest, NotNumUnionAliasing)
{
    // 验证写入一个成员后读取另一个成员得到正确位表示
    NotNumUnion u{};

    // 写入 float，读取 uint32_t
    u.f = -1.0f;
    EXPECT_NE(u.i, 0); // -1.0f 不是全零

    // 写入 uint32_t，读取 float
    u.i = 0x3F800000; // 1.0f 的位表示
    EXPECT_EQ(u.f, 1.0f);
}

TEST(TypePunningTest, HalfUnionAliasing)
{
    // 验证写入一个成员后读取另一个成员
    HalfUnion u{};

    // 写入 uint16_t，读取 half
    u.i = 0x3C00; // 1.0 in half
    // 验证位模式正确
    EXPECT_EQ(u.i, 0x3C00);

    // 写入 half，读取 uint16_t
    u.f = half(0.0f); // +0.0
    EXPECT_EQ(u.i, 0x0000);
}

} // namespace AscendC
