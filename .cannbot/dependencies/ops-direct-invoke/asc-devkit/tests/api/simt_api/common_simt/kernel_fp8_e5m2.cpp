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
 * \file kernel_fp8_e5m2.cpp
 * \brief
 */
#include "kernel_fp8_e5m2.h"
#include "kernel_fp32.h"
#include "kernel_utils.h"

namespace float8_e5m2 {
namespace {
// FP8 (E5M2)
#define FP8_SIGN_INDEX (7)
#define FP8_T_NAN (0x7F)
#define FP8_MAX_MAN (0x7)

constexpr int16_t FP8E5M2_EXP_MASK = 0x7C;
constexpr int16_t FP8E5M2_MAN_MASK = 0x3;
constexpr uint32_t FP8E5M2_MAN_LEN = 2;
constexpr uint32_t FP8E5M2_EXP_BIAS = 15;
#define FP8E5M2_MAN_HIDE_BIT (0x4)
#define FP8E5M2_T_MAX (0x7B)
#define FP8E5M2_MAX_EXP (0x1F)
#define FP8E5M2_MAX_MAN (0x3)
#define FP8E5M2_INF (0X7C)
#define FP8E5M2_ABS_MAKS (0X7F)

uint8_t Fp8e5m2Constructor(uint16_t s, uint16_t e, uint16_t m)
{
    return (((s) << FP8_SIGN_INDEX) | ((e) << FP8E5M2_MAN_LEN) | ((m)&FP8E5M2_MAX_MAN));
}

bool Fp8e5m2IsNan(const uint16_t& x)
{
    return ((((x)&FP8E5M2_EXP_MASK) == FP8E5M2_EXP_MASK) && (((x)&FP8E5M2_MAN_MASK) != 0));
}

bool Fp8e5m2IsInf(const uint16_t& x)
{
    return ((x == static_cast<uint8_t>(0x7C)) || (x == static_cast<uint8_t>(0xFC))) ? true : false;
}

int8_t Fp8e5m2ExtracSign(int8_t x) { return (((x) >> FP8_SIGN_INDEX) & 0x1); }

int8_t Fp8e5m2ExtracExp(int8_t x) { return (((x) >> FP8E5M2_MAN_LEN) & 0x1F); }

int8_t Fp8e5m2ExtracMan(uint8_t x)
{
    return ((((x) >> 0) & 0x3) | (((((x) >> FP8E5M2_MAN_LEN) & 0x1F) > 0 ? 1 : 0) * 0x4));
}

void ExtractFp8e5m2(const int8_t val, uint8_t& s, int8_t& e, uint8_t& m)
{
    // 1.Extract
    s = Fp8e5m2ExtracSign(val);
    e = static_cast<int16_t>(Fp8e5m2ExtracExp(val));
    m = Fp8e5m2ExtracMan(val);

    // Denormal---useless
    if (e == -15) {
        e = -14;
    }
}

uint32_t Fp8e5m2ToFp32(const int8_t fpVal)
{
    uint32_t ret = 0;

    if (fpVal == 0x0) {
        return 0x0;
    } else if (fpVal == static_cast<int8_t>(0x80)) {
        return 0x80000000;
    }
    // no INF value for fp8
    if (Fp8e5m2IsNan(fpVal)) {
        return FP32_NAN;
    }
    // for E5M2 need process inf
    if (Fp8e5m2IsInf(fpVal)) {
        return ((Fp8e5m2ExtracSign(fpVal) << FP32_SIGN_INDEX) | FP32_POS_INF);
    }

    uint8_t fp8Sign = 0;
    uint8_t fp8Man = 0;
    int8_t fp8Exp = 0;
    ExtractFp8e5m2(fpVal, fp8Sign, fp8Exp, fp8Man);
    bool isDenormal = false;
    if (fp8Exp == 0) {
        isDenormal = true;
    }

    while ((fp8Man != 0) && ((fp8Man & FP8E5M2_MAN_HIDE_BIT) == 0)) {
        fp8Man <<= 1;
        fp8Exp--;
    }
    uint32_t eRet = 0;
    uint32_t mRet = 0;
    uint32_t sRet = fp8Sign;
    if (fp8Man == 0) {
        eRet = 0;
        mRet = 0;
    } else {
        if (isDenormal) {
            // denormal
            eRet = (static_cast<uint64_t>(static_cast<int64_t>(fp8Exp + 1)) - FP8E5M2_EXP_BIAS) + FP32_EXP_BIAS;
        } else {
            eRet = (static_cast<uint64_t>(static_cast<int64_t>(fp8Exp)) - FP8E5M2_EXP_BIAS) + FP32_EXP_BIAS;
        }
        mRet = fp8Man << (FP32_MAN_LEN - FP8E5M2_MAN_LEN);
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

void Fp8e5m2Normalize(int16_t& exp, uint32_t& man)
{
    if (exp >= FP8E5M2_MAX_EXP) {
        exp = FP8E5M2_MAX_EXP;
        if (exp > FP8E5M2_MAX_EXP) {
            man = FP8E5M2_MAX_MAN - 1;
        } else if ((exp == FP8E5M2_MAX_EXP) && (man >= FP8E5M2_MAX_MAN)) {
            man = FP8_MAX_MAN - 1;
        }
    } else if (exp == 0 && man == FP8E5M2_MAN_HIDE_BIT) {
        exp++;
        man = 0;
    }
}
} // namespace

int8_t Fp8e5m2T::FloatToFp8e5m2(const float src) const
{
    uint32_t srcVal = AscendC::GetScalarBitcodeValue<float, uint32_t>(src);
    if (Fp32IsInf(srcVal)) {
        return ((Fp32ExtracSign(srcVal) << FP8_SIGN_INDEX) | FP8E5M2_INF);
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
    uint32_t mLenDelta = FP32_MAN_LEN - FP8E5M2_MAN_LEN;

    bool needRound = false;

    // E5M2 -14 ~15
    // whenfp32 e=-16，  means  denormal e5m2 + round
    if (ef == 0x6e) { // denormal case //S.00000.001
        // m=0.001000?  e5m2 man=0.00 else  man=0.01
        ret = (mf == 0x0) ? 0x0 : 0x1;
        return ((sRet << FP8_SIGN_INDEX) | (ret & 0xff));
    }
    // Exponent overflow/NaN converts to signed inf/NaN
    // when = &>15 （=bias = 142 0x8e）
    if (((ef == 0x8Eu) && (mf >= 0x700000)) // 65520.0
        || ((ef > 0x8Eu))) {
        eRet = FP8E5M2_MAX_EXP;
        mRet = FP8E5M2_MAX_MAN - 1;
    } else if (ef <= 0x70u) {
        // 0x70u:112=127-15 Exponent underflow converts to denormalized half or signed zero
        // e<=0x70 (-15)    when (-17, -15]
        eRet = 0;
        if (ef > 0x6e) { // -16 ~ -15
            mf = (mf | FP32_MAN_HIDE_BIT);
            shiftOut = FP32_MAN_LEN - FP8E5M2_MAN_LEN + (0x70 - (ef)) + 1;

            uint64_t mTmp = mf;
            needRound = IsRoundOne(sRet, mTmp, shiftOut);
            mRet = static_cast<uint16_t>(mf >> shiftOut);
            if (needRound) {
                mRet++;
            }
        } else { //  e<-17 (=16 has processed in begin)
            mRet = 0;
        }

        // -14~15
    } else { // Regular case with no overflow or underflow
        eRet = static_cast<int16_t>(ef - 0x70u);
        needRound = IsRoundOne(sRet, mf, mLenDelta);
        mRet = static_cast<uint16_t>(mf >> mLenDelta);
        if (needRound) {
            mRet++;
        }
        if (((mRet & FP8E5M2_MAN_HIDE_BIT) != 0) && (needRound)) {
            eRet++;
            mRet = 0;
        }
    }
    Fp8e5m2Normalize(eRet, mRet);
    ret = Fp8e5m2Constructor(sRet, eRet, mRet);

    return ret;
}

Fp8e5m2T::operator float() const { return AscendC::GetScalarBitcodeValue<uint32_t, float>(Fp8e5m2ToFp32(val)); }

float Fp8e5m2T::ToFloat() const { return AscendC::GetScalarBitcodeValue<uint32_t, float>(Fp8e5m2ToFp32(val)); }
} // namespace float8_e5m2
