/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

/* !
 * \file matmul_mx.h
 * \brief Header file for matmul_mx case1~case2 implementation
 */

#ifndef MATMULMX_H
#define MATMULMX_H

#include "kernel_tiling/kernel_tiling.h"
#include "tiling/tiling_api.h"
#include "kernel_operator.h"
#define SPLIT_CORE_CUBE
#define ASCENDC_CUBE_ONLY
#include "lib/matmul_intf.h"

struct ScaleFactors {
    uint32_t ka;
    uint32_t kb;
    uint32_t m;
    uint32_t n;
};

struct MatmulMxStaticCfg {
    static constexpr uint32_t M = 8192;
    static constexpr uint32_t N = 8192;
    static constexpr uint32_t K = 8192;

    static constexpr bool IS_TRANS_A = false;
    static constexpr bool IS_TRANS_B = false;

    static constexpr uint32_t BASE_M = 256;
    static constexpr uint32_t BASE_N = 256;
    static constexpr uint32_t BASE_K = 256;

    static constexpr uint32_t SINGLE_M = 2048;
    static constexpr uint32_t SINGLE_N = 1024;
    static constexpr uint32_t SINGLE_K = 8192;

    static constexpr uint32_t SCALE_CEIL_NUMBER = 64;
    static constexpr uint32_t SCALE_NUMBER = 2;
    static constexpr uint32_t SCALE_K = (K + SCALE_CEIL_NUMBER - 1) / SCALE_CEIL_NUMBER * SCALE_NUMBER;

    static constexpr uint32_t CORE_BLOCKS_M = M / SINGLE_M;
    static constexpr uint32_t CORE_BLOCKS_N = N / SINGLE_N;
    static constexpr uint32_t CORE_NUM = CORE_BLOCKS_M * CORE_BLOCKS_N;

    static constexpr int DEPTH_A1 = 4;
    static constexpr int DEPTH_B1 = 4;
    static constexpr int STEP_KA = 2;
    static constexpr int STEP_KB = 2;
    static constexpr int STEP_M = 1;
    static constexpr int STEP_N = 1;
    static constexpr int DB_L0A = 2;
    static constexpr int DB_L0B = 2;

    static constexpr ScaleFactors CASE1_SCALE = {1U, 1U, 1U, 1U};
    static constexpr ScaleFactors CASE2_SCALE = {4U, 4U, 1U, 1U};
};

using cfg = MatmulMxStaticCfg;

static_assert(cfg::M % cfg::SINGLE_M == 0, "M must be divisible by SINGLE_M");
static_assert(cfg::N % cfg::SINGLE_N == 0, "N must be divisible by SINGLE_N");
static_assert(cfg::K == cfg::SINGLE_K, "K must equal SINGLE_K");

// mxTypePara bit layout (MatmulApiStaticTiling/TCubeTiling):
// [0:6]   scaleFactorKa
// [8:14]  scaleFactorKb
// [16:22] scaleFactorM
// [24:30] scaleFactorN
constexpr uint32_t BuildMxTypePara(const ScaleFactors& scale)
{
    return ((scale.ka & 0x7FU) << 0) | ((scale.kb & 0x7FU) << 8) | ((scale.m & 0x7FU) << 16) |
           ((scale.n & 0x7FU) << 24);
}

constexpr uint32_t CASE1_MX_TYPE_PARA = BuildMxTypePara(cfg::CASE1_SCALE);
constexpr uint32_t CASE2_MX_TYPE_PARA = BuildMxTypePara(cfg::CASE2_SCALE);

constexpr MatmulShapeParams SHAPE_PARAMS = {cfg::SINGLE_M, cfg::SINGLE_N, cfg::SINGLE_K,
                                            cfg::BASE_M,   cfg::BASE_N,   cfg::BASE_K};

template <typename AType, typename BType, typename CType, bool EnableScaleCache>
__aicore__ inline constexpr MatmulApiStaticTiling GetMxConstantCFG()
{
    MatmulConfig mmCFG = GetMMConfig<MatmulConfigMode::CONFIG_MDL>(SHAPE_PARAMS);
    auto constantCFG = AscendC::GetMatmulApiTiling<AType, BType, CType, CType>(mmCFG);

    constantCFG.depthA1 = cfg::DEPTH_A1;
    constantCFG.depthB1 = cfg::DEPTH_B1;
    constantCFG.stepKa = cfg::STEP_KA;
    constantCFG.stepKb = cfg::STEP_KB;
    constantCFG.stepM = cfg::STEP_M;
    constantCFG.stepN = cfg::STEP_N;
    constantCFG.dbL0A = cfg::DB_L0A;
    constantCFG.dbL0B = cfg::DB_L0B;
    constantCFG.mxTypePara = EnableScaleCache ? CASE2_MX_TYPE_PARA : CASE1_MX_TYPE_PARA;
    return constantCFG;
}

template <bool EnableScaleCache>
class MatmulKernel {
public:
    __aicore__ inline MatmulKernel(){};

    using aType = AscendC::MatmulTypeWithScale<
        AscendC::TPosition::GM, AscendC::TPosition::GM, CubeFormat::ND, fp4x2_e1m2_t, false>;
    using bType = AscendC::MatmulTypeWithScale<
        AscendC::TPosition::GM, AscendC::TPosition::GM, CubeFormat::ND, fp4x2_e1m2_t, false>;
    using cType = AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, bfloat16_t>;

    constexpr static auto CONSTANT_CFG = GetMxConstantCFG<aType, bType, cType, EnableScaleCache>();

    AscendC::Matmul<
        aType, bType, cType, cType, CONSTANT_CFG, AscendC::MatmulCallBackFunc<nullptr, nullptr, nullptr>,
        AscendC::Impl::Detail::MatmulWithScalePolicy>
        matmulObj;

    __aicore__ inline void Init(
        __gm__ uint8_t* a, __gm__ uint8_t* b, __gm__ uint8_t* as, __gm__ uint8_t* bs, __gm__ uint8_t* c)
    {
        aGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ fp4x2_e1m2_t*>(a), kM * kK);
        bGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ fp4x2_e1m2_t*>(b), kK * kN);
        cGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ bfloat16_t*>(c), kM * kN);
        asGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ AscendC::fp8_e8m0_t*>(as), kM * kScaleK);
        bsGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ AscendC::fp8_e8m0_t*>(bs), kScaleK * kN);

        int32_t offsetA = 0;
        int32_t offsetB = 0;
        int32_t offsetC = 0;
        int32_t offsetAscale = 0;
        int32_t offsetBscale = 0;
        CalcOffset(AscendC::GetBlockIdx(), offsetA, offsetB, offsetAscale, offsetBscale, offsetC);

        aGlobal = aGlobal[offsetA];
        bGlobal = bGlobal[offsetB];
        cGlobal = cGlobal[offsetC];
        asGlobal = asGlobal[offsetAscale];
        bsGlobal = bsGlobal[offsetBscale];

        if (GetSysWorkSpacePtr() == nullptr) {
            return;
        }
    }

    __aicore__ inline void Process(AscendC::TPipe* pipe)
    {
        if (AscendC::GetBlockIdx() >= kCoreNum) {
            return;
        }

        REGIST_MATMUL_OBJ(pipe, GetSysWorkSpacePtr(), matmulObj, (TCubeTiling*)nullptr);
        matmulObj.SetOrgShape(kM, kN, kK);

        matmulObj.SetTensorA(aGlobal, kIsTransA);
        matmulObj.SetTensorB(bGlobal, kIsTransB);
        matmulObj.SetTensorScaleA(asGlobal, false);
        matmulObj.SetTensorScaleB(bsGlobal, false);

        matmulObj.IterateAll(cGlobal);
        matmulObj.End();
    }

private:
    static constexpr uint32_t kM = cfg::M;
    static constexpr uint32_t kN = cfg::N;
    static constexpr uint32_t kK = cfg::K;
    static constexpr uint32_t kScaleK = cfg::SCALE_K;
    static constexpr uint32_t kSingleM = cfg::SINGLE_M;
    static constexpr uint32_t kSingleN = cfg::SINGLE_N;
    static constexpr uint32_t kScaleNumber = cfg::SCALE_NUMBER;
    static constexpr uint32_t kCoreBlocksM = cfg::CORE_BLOCKS_M;
    static constexpr uint32_t kCoreNum = cfg::CORE_NUM;
    static constexpr bool kIsTransA = cfg::IS_TRANS_A;
    static constexpr bool kIsTransB = cfg::IS_TRANS_B;

    __aicore__ inline void CalcOffset(
        int32_t blockIdx, int32_t& offsetA, int32_t& offsetB, int32_t& offsetAscale, int32_t& offsetBscale,
        int32_t& offsetC)
    {
        auto mCoreIndex = blockIdx % kCoreBlocksM;
        auto nCoreIndex = blockIdx / kCoreBlocksM;

        offsetA = mCoreIndex * kK * kSingleM;
        if (kIsTransA) {
            offsetA = mCoreIndex * kSingleM;
        }

        offsetB = nCoreIndex * kSingleN;
        if (kIsTransB) {
            offsetB = nCoreIndex * kK * kSingleN;
        }

        offsetAscale = mCoreIndex * kScaleK * kSingleM;
        offsetBscale = nCoreIndex * kSingleN * kScaleNumber;

        offsetC = mCoreIndex * kN * kSingleM + nCoreIndex * kSingleN;
    }

    AscendC::GlobalTensor<fp4x2_e1m2_t> aGlobal;
    AscendC::GlobalTensor<fp4x2_e1m2_t> bGlobal;
    AscendC::GlobalTensor<bfloat16_t> cGlobal;
    AscendC::GlobalTensor<AscendC::fp8_e8m0_t> asGlobal;
    AscendC::GlobalTensor<AscendC::fp8_e8m0_t> bsGlobal;
};

#endif
