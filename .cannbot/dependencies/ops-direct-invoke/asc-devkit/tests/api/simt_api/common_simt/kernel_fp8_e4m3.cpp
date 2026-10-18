/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

/*!
 * \file kernel_fp8_e4m3.cpp
 * \brief
 */
#include "kernel_fp8_e4m3.h"
#include "kernel_fp32.h"
#include "kernel_utils.h"

namespace float8_e4m3 {
namespace {
// FP8 (E4M3)
#define FP8_SIGN_INDEX (7)
#define FP8_T_MAX (0x7E)
#define FP8_T_NEG_MAX (0x8E)
#define FP8_T_NAN (0x7F)
constexpr uint32_t FP8E4M3_EXP_BIAS = 7;
constexpr uint32_t FP8E4M3_EXP_LEN = 4;
constexpr uint32_t FP8E4M3_MAN_LEN = 3;
#define FP8_MAX_EXP (0xF)
#define FP8_MAX_MAN (0x7)
#define FP8_MAN_HIDE_BIT (0x8)

bool Fp8e4m3IsNan(const int8_t& x)
{
    return (((x == static_cast<int8_t>(0x7F)) || (x == static_cast<int8_t>(0xFF))) ? true : false);
}

uint8_t Fp8e4m3Constructor(uint16_t s, uint16_t e, uint16_t m)
{
    return (((s) << FP8_SIGN_INDEX) | ((e) << FP8E4M3_MAN_LEN) | ((m)&FP8_MAX_MAN));
}

uint16_t Fp8e4m3ExtracSign(uint8_t x) { return (((x) >> FP8_SIGN_INDEX) & 0x1); }

uint16_t Fp8e4m3ExtracExp(uint8_t x) { return (((x) >> FP8E4M3_MAN_LEN) & 0xF); }

uint16_t Fp8e4m3ExtracMan(uint8_t x)
{
    return ((((x) >> 0) & 0x7) | (((((x) >> FP8E4M3_MAN_LEN) & 0xF) > 0 ? 1 : 0) * 0x8));
}

void ExtractFP8(const int8_t val, uint8_t& s, int8_t& e, uint8_t& m)
{
    // 1.Extract
    s = Fp8e4m3ExtracSign(val);
    e = static_cast<int16_t>(Fp8e4m3ExtracExp(val));
    m = Fp8e4m3ExtracMan(val);

    // Denormal
    if (e == -7) {
        e = -6;
    }
}

uint32_t Fp8e4m3ToFp32(const int8_t fpVal)
{
    uint32_t ret = 0;
    if (fpVal == 0x0) {
        return 0x0;
    }
    if (fpVal == static_cast<int8_t>(0x80)) {
        return 0x80000000;
    }
    // no INF value for fp8
    if (Fp8e4m3IsNan(fpVal)) {
        return FP32_NAN;
    }

    uint8_t hf8Sign;
    uint8_t hf8Man;
    int8_t hf8Exp;
    ExtractFP8(fpVal, hf8Sign, hf8Exp, hf8Man);
    bool isDenormal = false;
    if (hf8Exp == 0) {
        isDenormal = true;
    }

    while ((hf8Man != 0) && ((hf8Man & FP8_MAN_HIDE_BIT) == 0)) {
        hf8Man <<= 1;
        hf8Exp--;
    }
    uint32_t eRet;
    uint32_t mRet;
    uint32_t sRet = hf8Sign;
    if (hf8Man == 0) {
        eRet = 0;
        mRet = 0;
    } else {
        if (isDenormal) {
            // denormal
            eRet = (static_cast<uint64_t>(static_cast<int64_t>(hf8Exp + 1)) - FP8E4M3_EXP_BIAS) + FP32_EXP_BIAS;
        } else {
            eRet = (static_cast<uint64_t>(static_cast<int64_t>(hf8Exp)) - FP8E4M3_EXP_BIAS) + FP32_EXP_BIAS;
        }
        mRet = hf8Man << (FP32_MAN_LEN - FP8E4M3_MAN_LEN);
    }
    // No overflow or underflow
    uint32_t fVal = Fp32Constructor(sRet, eRet, mRet);
    ret = fVal;
    return ret;
}

bool IsRoundOne(uint32_t sign, uint64_t man, uint16_t truncLen)
{
    (void)sign;
    if (truncLen == 0) {
        return false;
    }
    uint64_t roundingTruncLen = 64;
    uint64_t mask0 = (truncLen >= roundingTruncLen) ? 0 : 0x1ul << truncLen;
    uint64_t mask1 = (truncLen > roundingTruncLen) ? 0 : 0x1ul << (truncLen - 1);
    uint64_t mask2 = mask1 - 1;

    // ROUND_TO_NEAREST
    bool lastBit = ((man & mask0) > 0);      // Last bit after conversion
    bool truncHighBit = ((man & mask1) > 0); // Highest bit in the truncated part
    bool truncLeft = ((man & mask2) > 0);    // Truncated left part (except for the highest bit)
    return (truncHighBit && (truncLeft || lastBit));
}

void Fp8e4m3Normalize(int16_t& exp, uint32_t& man)
{
    if (exp >= FP8_MAX_EXP) {
        exp = FP8_MAX_EXP;
        if (exp > FP8_MAX_EXP) {
            man = FP8_MAX_MAN - 1;
        } else if ((exp == FP8_MAX_EXP) && (man >= FP8_MAX_MAN)) {
            man = FP8_MAX_MAN - 1;
        }
    } else if (exp == 0 && man == FP8_MAN_HIDE_BIT) {
        exp++;
        man = 0;
    }
}
} // namespace

int8_t Fp8e4m3T::FloatToFp8e4m3(const float src) const
{
    uint32_t srcVal = AscendC::GetScalarBitcodeValue<float, uint32_t>(src);
    if (Fp32IsInf(srcVal)) {
        return FP8_T_NAN;
    }

    if (Fp32IsNan(srcVal)) {
        return FP8_T_NAN;
    }
    if (Fp32IsZero(srcVal)) {
        return ((Fp32ExtracSign(srcVal) << FP8_SIGN_INDEX) | 0x0);
    }
    int8_t ret = 0;
    uint32_t mRet = 0;
    int16_t eRet = 0;
    uint16_t shiftOut = 0;

    uint16_t sRet = Fp32ExtracSign(srcVal);
    uint32_t ef = Fp32ExtracExp(srcVal);
    uint32_t mf = (srcVal & 0x007FFFFF); // 23 bit mantissa dont't need to care about denormal
    uint32_t mLenDelta = FP32_MAN_LEN - FP8E4M3_MAN_LEN;

    bool needRound = false;

    // -10
    if (ef == 0x75) {
        // denormal case // S.0000.0001
        // m=0.0001000?  e4m3 man=0.000 else  man=0.001
        ret = (mf == 0x0) ? 0x0 : 0x1;
        return ((sRet << FP8_SIGN_INDEX) | (ret & 0xff));
    }
    // Exponent overflow/NaN converts to signed inf/NaN
    // -6 ~ 8
    if (((ef == 0x87u) && (mf >= 0x680000)) // 65520.0
        || (ef > 0x87u)) {
        eRet = FP8_MAX_EXP;
        mRet = FP8_MAX_MAN - 1;
    } else if (ef <= 0x78u) {
        // 0x78u:120=127-7 Exponent underflow converts to denormalized half or signed zero
        eRet = 0;
        if (ef > 0x75) {
            mf = (mf | FP32_MAN_HIDE_BIT); // mantissa + 1 --> 1p23

            shiftOut = FP32_MAN_LEN - FP8E4M3_MAN_LEN + (0x78 - (ef)) + 1;
            uint64_t mTmp = mf;
            needRound = IsRoundOne(sRet, mTmp, shiftOut);
            mRet = static_cast<uint16_t>(mf >> shiftOut);
            if (needRound) {
                mRet++;
            }
        } else {
            mRet = 0;
        }
    } else {
        // Regular case with no overflow or underflow
        eRet = static_cast<int16_t>(ef - 0x78u);
        needRound = IsRoundOne(sRet, mf, mLenDelta);
        mRet = static_cast<uint16_t>(mf >> mLenDelta);
        if (needRound) {
            mRet++;
        }

        if (((mRet & FP8_MAN_HIDE_BIT) != 0) && (needRound)) {
            eRet++;
            mRet = 0;
        }
    }

    Fp8e4m3Normalize(eRet, mRet);
    ret = Fp8e4m3Constructor(sRet, eRet, mRet);
    return ret;
}

Fp8e4m3T::operator float() const { return AscendC::GetScalarBitcodeValue<uint32_t, float>(Fp8e4m3ToFp32(val)); }

float Fp8e4m3T::ToFloat() const { return AscendC::GetScalarBitcodeValue<uint32_t, float>(Fp8e4m3ToFp32(val)); }
} // namespace float8_e4m3
