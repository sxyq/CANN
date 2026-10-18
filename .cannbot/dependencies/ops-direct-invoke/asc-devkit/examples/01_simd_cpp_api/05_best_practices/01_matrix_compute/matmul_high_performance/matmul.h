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
 * \file matmul.h
 * \brief Header file for matmul allcase implementation
 */

#ifndef MATMUL_H
#define MATMUL_H

#include "data_utils.h"
#include "kernel_tiling/kernel_tiling.h"
#include "tiling/platform/platform_ascendc.h"
#include "tiling/tiling_api.h"
#include "acl/acl.h"
#include "kernel_operator.h"
#define ASCENDC_CUBE_ONLY
#include "lib/matmul_intf.h"
constexpr uint32_t scenarioNum = SCENARIO_NUM;
constexpr uint32_t M = 8192;
constexpr uint32_t N = 8192;
constexpr uint32_t K = 8192;
constexpr bool IS_TRANS_A = false;
constexpr bool IS_TRANS_B = true;
constexpr bool IS_BIAS = false;

constexpr uint32_t SINGLE_M = 2048;
constexpr uint32_t SINGLE_M_SPLIT = 4096;
constexpr uint32_t SINGLE_M_L2CACHE = 1024;
constexpr uint32_t SINGLE_K = 8192;

constexpr uint32_t BASE_N = 256;
constexpr uint32_t BASE_K = 64;
constexpr uint32_t TILING_DEPTHB1_PARAM = 8;
constexpr uint32_t TILING_STEPKB_PARAM = 4;
constexpr uint32_t TILING_STEPMN_PARAM = 1;

struct MatmulProblemShape {
    uint32_t usedCoreNum;
    uint32_t m;
    uint32_t n;
    uint32_t k;
    uint32_t singleCoreM;
    uint32_t singleCoreN;
    uint32_t singleCoreK;
    uint32_t isBias;
};

template <typename TilingType>
__aicore__ inline void CopyTiling(TilingType* tiling, __gm__ uint8_t* tilingGM)
{
    uint32_t* ptr = reinterpret_cast<uint32_t*>(tiling);
    auto tiling32 = reinterpret_cast<__gm__ uint32_t*>(tilingGM);
    for (uint32_t i = 0; i < sizeof(TilingType) / sizeof(uint32_t); i++, ptr++) {
        *ptr = *(tiling32 + i);
    }
}

template <typename AType, typename BType, typename CType, typename BiasType, bool isMdl = false>
class MatmulKernel {
public:
    static constexpr auto MatmulConfig = isMdl ? CFG_MDL : CFG_NORM;
    __aicore__ inline MatmulKernel(){};
    __aicore__ inline void Init(
        __gm__ uint8_t* a, __gm__ uint8_t* b, __gm__ uint8_t* bias, __gm__ uint8_t* c, const TCubeTiling& tiling);
    __aicore__ inline void Process(AscendC::TPipe* pipe);

    AscendC::Matmul<
        AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, AType>,
        AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, BType>,
        AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, CType>,
        AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, BiasType>, MatmulConfig>
        matmulObj;

private:
    __aicore__ inline void CalcOffset(
        int32_t blockIdx, int32_t& offsetA, int32_t& offsetB, int32_t& offsetC, int32_t& offsetBias);

    AscendC::GlobalTensor<AType> aGlobal;
    AscendC::GlobalTensor<BType> bGlobal;
    AscendC::GlobalTensor<CType> cGlobal;
    AscendC::GlobalTensor<BiasType> biasGlobal;
    TCubeTiling tiling;
};

template <typename AType, typename BType, typename CType, typename BiasType, bool isMdl>
__aicore__ inline void MatmulKernel<AType, BType, CType, BiasType, isMdl>::Init(
    __gm__ uint8_t* a, __gm__ uint8_t* b, __gm__ uint8_t* bias, __gm__ uint8_t* c, const TCubeTiling& tiling)
{
    this->tiling = tiling;
    aGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ AType*>(a), tiling.M * tiling.Ka);
    bGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ BType*>(b), tiling.Kb * tiling.N);
    cGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ CType*>(c), tiling.M * tiling.N);
    biasGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ BiasType*>(bias), tiling.N);

    int32_t offsetA = 0, offsetB = 0, offsetC = 0, offsetBias = 0;
    CalcOffset(AscendC::GetBlockIdx(), offsetA, offsetB, offsetC, offsetBias);
    aGlobal = aGlobal[offsetA];
    bGlobal = bGlobal[offsetB];
    cGlobal = cGlobal[offsetC];
    biasGlobal = biasGlobal[offsetBias];

    if (GetSysWorkSpacePtr() == nullptr) {
        return;
    }
}

template <typename AType, typename BType, typename CType, typename BiasType, bool isMdl>
__aicore__ inline void MatmulKernel<AType, BType, CType, BiasType, isMdl>::Process(AscendC::TPipe* pipe)
{
    // Case 0-5: 单次迭代完成当前核分片计算
    matmulObj.SetTensorA(aGlobal, IS_TRANS_A);
    matmulObj.SetTensorB(bGlobal, IS_TRANS_B);
    if (tiling.isBias) {
        matmulObj.SetBias(biasGlobal);
    }
    matmulObj.IterateAll(cGlobal);
    matmulObj.End();
}

template <typename AType, typename BType, typename CType, typename BiasType, bool isMdl>
__aicore__ inline void MatmulKernel<AType, BType, CType, BiasType, isMdl>::CalcOffset(
    int32_t blockIdx, int32_t& offsetA, int32_t& offsetB, int32_t& offsetC, int32_t& offsetBias)
{
    const TCubeTiling& tiling = this->tiling;
    auto mSingleBlocks = (tiling.M + tiling.singleCoreM - 1) / tiling.singleCoreM;
    // blockIdx -> M/N方向分片索引
    auto mCoreIndx = blockIdx % mSingleBlocks;
    auto nCoreIndx = blockIdx / mSingleBlocks;

    offsetA = mCoreIndx * tiling.Ka * tiling.singleCoreM;
    if (IS_TRANS_A) {
        offsetA = mCoreIndx * tiling.singleCoreM;
    }
    offsetB = nCoreIndx * tiling.singleCoreN;
    if (IS_TRANS_B) {
        offsetB = nCoreIndx * tiling.Kb * tiling.singleCoreN;
    }
    offsetC = mCoreIndx * tiling.N * tiling.singleCoreM + nCoreIndx * tiling.singleCoreN;
    offsetBias = nCoreIndx * tiling.singleCoreN;

    // M/N尾块处理
    int tailM = tiling.M - mCoreIndx * tiling.singleCoreM;
    tailM = tailM < tiling.singleCoreM ? tailM : tiling.singleCoreM;
    int tailN = tiling.N - nCoreIndx * tiling.singleCoreN;
    tailN = tailN < tiling.singleCoreN ? tailN : tiling.singleCoreN;
    if (tailM < tiling.singleCoreM || tailN < tiling.singleCoreN) {
        matmulObj.SetTail(tailM, tailN);
    }
}

template <typename AType, typename BType, typename CType, typename BiasType>
class MatmulKernelL2Cache {
public:
    __aicore__ inline MatmulKernelL2Cache(){};
    __aicore__ inline void Init(
        __gm__ uint8_t* a, __gm__ uint8_t* b, __gm__ uint8_t* bias, __gm__ uint8_t* c, const TCubeTiling& tiling);
    __aicore__ inline void Process(AscendC::TPipe* pipe);

    static constexpr auto MatmulConfig = CFG_MDL;
    AscendC::Matmul<
        AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, AType>,
        AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, BType>,
        AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, CType>,
        AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, BiasType>, MatmulConfig>
        matmulObj;

private:
    __aicore__ inline void CalcOffset(
        int32_t blockIdx, int32_t& offsetA, int32_t& offsetB, int32_t& offsetC, int32_t& offsetBias);

    AscendC::GlobalTensor<AType> aGlobal;
    AscendC::GlobalTensor<BType> bGlobal;
    AscendC::GlobalTensor<CType> cGlobal;
    AscendC::GlobalTensor<BiasType> biasGlobal;
    TCubeTiling tiling;
    uint32_t mIdx;
    uint32_t nIdx;
};

template <typename AType, typename BType, typename CType, typename BiasType>
__aicore__ inline void MatmulKernelL2Cache<AType, BType, CType, BiasType>::Init(
    __gm__ uint8_t* a, __gm__ uint8_t* b, __gm__ uint8_t* bias, __gm__ uint8_t* c, const TCubeTiling& tiling)
{
    this->tiling = tiling;
    aGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ AType*>(a), tiling.M * tiling.Ka);
    bGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ BType*>(b), tiling.Kb * tiling.N);
    cGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ CType*>(c), tiling.M * tiling.N);
    biasGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ BiasType*>(bias), tiling.N);

    if (GetSysWorkSpacePtr() == nullptr) {
        return;
    }
}

template <typename AType, typename BType, typename CType, typename BiasType>
__aicore__ inline void MatmulKernelL2Cache<AType, BType, CType, BiasType>::Process(AscendC::TPipe* pipe)
{
    REGIST_MATMUL_OBJ(pipe, GetSysWorkSpacePtr(), matmulObj, &tiling);

    int32_t offsetA = 0, offsetB = 0, offsetC = 0, offsetBias = 0;
    CalcOffset(AscendC::GetBlockIdx(), offsetA, offsetB, offsetC, offsetBias);

    int tailM = tiling.M - mIdx * tiling.singleCoreM;
    tailM = tailM < tiling.singleCoreM ? tailM : tiling.singleCoreM;
    int tailN = tiling.N - nIdx * tiling.singleCoreN;
    tailN = tailN < tiling.singleCoreN ? tailN : tiling.singleCoreN;
    if (tailM < tiling.singleCoreM || tailN < tiling.singleCoreN) {
        matmulObj.SetTail(tailM, tailN);
    }

    // Case 6: L2Cache优化，A矩阵M轴切分为2份，B矩阵复用
    for (int i = 0; i < 2; i++) {
        matmulObj.SetTensorA(aGlobal[offsetA + i * (M >> 1) * K], IS_TRANS_A);
        matmulObj.SetTensorB(bGlobal[offsetB], IS_TRANS_B);
        if (tiling.isBias) {
            matmulObj.SetBias(biasGlobal);
        }
        matmulObj.IterateAll(cGlobal[offsetC + i * (M >> 1) * N]);
    }
    matmulObj.End();
}

template <typename AType, typename BType, typename CType, typename BiasType>
__aicore__ inline void MatmulKernelL2Cache<AType, BType, CType, BiasType>::CalcOffset(
    int32_t blockIdx, int32_t& offsetA, int32_t& offsetB, int32_t& offsetC, int32_t& offsetBias)
{
    const TCubeTiling& tiling = this->tiling;
    constexpr uint32_t mSingleBlocks = 4;
    // Case 6: M方向固定为4份，N方向由blockIdx展开
    mIdx = blockIdx % mSingleBlocks;
    nIdx = blockIdx / mSingleBlocks;

    offsetA = mIdx * tiling.Ka * tiling.singleCoreM;
    if (IS_TRANS_A) {
        offsetA = mIdx * tiling.singleCoreM;
    }
    offsetB = nIdx * tiling.singleCoreN;
    if (IS_TRANS_B) {
        offsetB = nIdx * tiling.Kb * tiling.singleCoreN;
    }
    offsetC = mIdx * tiling.N * tiling.singleCoreM + nIdx * tiling.singleCoreN;
    offsetBias = nIdx * tiling.singleCoreN;
}

template <
    typename AType, typename BType, typename CType, typename BiasType, uint32_t singleN, uint32_t baseM,
    uint32_t depthA1Param, uint32_t stepKaParam, bool useUnitFlag = false>
__aicore__ inline constexpr MatmulApiStaticTiling GetCustomConstantCFG()
{
    // Case 7/8: 编译期生成MatmulApiStaticTiling
    constexpr MatmulShapeParams shapeParams = {SINGLE_M_L2CACHE, singleN, SINGLE_K, baseM, BASE_N, BASE_K};
    MatmulConfig mmCFG = GetMMConfig<MatmulConfigMode::CONFIG_MDL>(shapeParams);
    mmCFG.enUnitFlag = useUnitFlag;
    auto constantCFG = AscendC::GetMatmulApiTiling<AType, BType, CType, BiasType>(mmCFG);
    constantCFG.depthA1 = depthA1Param;
    constantCFG.depthB1 = TILING_DEPTHB1_PARAM;
    constantCFG.stepKa = stepKaParam;
    constantCFG.stepKb = TILING_STEPKB_PARAM;
    constantCFG.stepM = TILING_STEPMN_PARAM;
    constantCFG.stepN = TILING_STEPMN_PARAM;
    return constantCFG;
}

template <
    typename AType, typename BType, typename CType, typename BiasType, uint32_t singleN, uint32_t baseM,
    uint32_t depthA1Param, uint32_t stepKaParam, bool useUnitFlag = false>
class MatmulKernelMdlL2CacheConstant {
public:
    __aicore__ inline MatmulKernelMdlL2CacheConstant(){};
    __aicore__ inline void Init(
        __gm__ uint8_t* a, __gm__ uint8_t* b, __gm__ uint8_t* bias, __gm__ uint8_t* c, __gm__ uint8_t* tiling);
    __aicore__ inline void Process(AscendC::TPipe* pipe);

    using A_TYPE = AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, AType>;
    using B_TYPE = AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, BType>;
    using C_TYPE = AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, CType>;
    using BIAS_TYPE = AscendC::MatmulType<AscendC::TPosition::GM, CubeFormat::ND, BiasType>;

    // CONSTANT_CFG: 常量Tiling配置，减少运行时Scalar计算
    constexpr static auto CONSTANT_CFG = GetCustomConstantCFG<
        A_TYPE, B_TYPE, C_TYPE, BIAS_TYPE, singleN, baseM, depthA1Param, stepKaParam, useUnitFlag>();
    AscendC::Matmul<A_TYPE, B_TYPE, C_TYPE, BIAS_TYPE, CONSTANT_CFG> matmulObj;
    MatmulProblemShape shapes;

private:
    __aicore__ inline void CalcOffset(
        const MatmulProblemShape& param, uint32_t& offsetA, uint32_t& offsetB, uint32_t& offsetC, uint32_t& offsetBias);

    AscendC::GlobalTensor<AType> aGlobal;
    AscendC::GlobalTensor<BType> bGlobal;
    AscendC::GlobalTensor<CType> cGlobal;
    AscendC::GlobalTensor<BiasType> biasGlobal;
    uint32_t mIdx;
    uint32_t nIdx;
};

template <
    typename AType, typename BType, typename CType, typename BiasType, uint32_t singleN, uint32_t baseM,
    uint32_t depthA1Param, uint32_t stepKaParam, bool useUnitFlag>
__aicore__ inline void
MatmulKernelMdlL2CacheConstant<AType, BType, CType, BiasType, singleN, baseM, depthA1Param, stepKaParam, useUnitFlag>::
    Init(__gm__ uint8_t* a, __gm__ uint8_t* b, __gm__ uint8_t* bias, __gm__ uint8_t* c, __gm__ uint8_t* tiling)
{
    CopyTiling(&shapes, tiling);
    if (AscendC::GetBlockIdx() >= shapes.usedCoreNum) {
        return;
    }
    aGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ AType*>(a), shapes.m * shapes.k);
    bGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ BType*>(b), shapes.n * shapes.k);
    cGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ CType*>(c), shapes.m * shapes.n);
    biasGlobal.SetGlobalBuffer(reinterpret_cast<__gm__ BiasType*>(bias), shapes.n);

    if (GetSysWorkSpacePtr() == nullptr) {
        return;
    }
}

template <
    typename AType, typename BType, typename CType, typename BiasType, uint32_t singleN, uint32_t baseM,
    uint32_t depthA1Param, uint32_t stepKaParam, bool useUnitFlag>
__aicore__ inline void MatmulKernelMdlL2CacheConstant<
    AType, BType, CType, BiasType, singleN, baseM, depthA1Param, stepKaParam, useUnitFlag>::Process(AscendC::TPipe*
                                                                                                        pipe)
{
    REGIST_MATMUL_OBJ(pipe, GetSysWorkSpacePtr(), matmulObj, (TCubeTiling*)nullptr);
    matmulObj.SetOrgShape(shapes.m, shapes.n, shapes.k);

    uint32_t offsetA = 0, offsetB = 0, offsetC = 0, offsetBias = 0;
    CalcOffset(shapes, offsetA, offsetB, offsetC, offsetBias);

    uint32_t tailM = shapes.m - mIdx * shapes.singleCoreM;
    tailM = tailM < shapes.singleCoreM ? tailM : shapes.singleCoreM;
    uint32_t tailN = shapes.n - nIdx * shapes.singleCoreN;
    tailN = tailN < shapes.singleCoreN ? tailN : shapes.singleCoreN;
    if (tailM < shapes.singleCoreM || tailN < shapes.singleCoreN) {
        matmulObj.SetTail(tailM, tailN);
    }

    // Case 7/8: 沿用L2Cache切分策略，循环2次
    for (int i = 0; i < 2; i++) {
        matmulObj.SetTensorA(aGlobal[offsetA + i * (M >> 1) * K], IS_TRANS_A);
        matmulObj.SetTensorB(bGlobal[offsetB], IS_TRANS_B);
        if (shapes.isBias) {
            matmulObj.SetBias(biasGlobal);
        }
        matmulObj.IterateAll(cGlobal[offsetC + i * (M >> 1) * N]);
    }
    matmulObj.End();
}

template <
    typename AType, typename BType, typename CType, typename BiasType, uint32_t singleN, uint32_t baseM,
    uint32_t depthA1Param, uint32_t stepKaParam, bool useUnitFlag>
__aicore__ inline void
MatmulKernelMdlL2CacheConstant<AType, BType, CType, BiasType, singleN, baseM, depthA1Param, stepKaParam, useUnitFlag>::
    CalcOffset(
        const MatmulProblemShape& param, uint32_t& offsetA, uint32_t& offsetB, uint32_t& offsetC, uint32_t& offsetBias)
{
    auto blockIdx = AscendC::GetBlockIdx();
    constexpr uint32_t mSingleBlocks = 4;
    // Case 7/8: M方向固定为4份，N方向由blockIdx展开
    mIdx = blockIdx % mSingleBlocks;
    nIdx = blockIdx / mSingleBlocks;

    offsetA = mIdx * param.k * param.singleCoreM;
    if (IS_TRANS_A) {
        offsetA = mIdx * param.singleCoreM;
    }
    offsetB = nIdx * param.singleCoreN;
    if (IS_TRANS_B) {
        offsetB = nIdx * param.k * param.singleCoreN;
    }
    offsetC = mIdx * param.n * param.singleCoreM + nIdx * param.singleCoreN;
    offsetBias = nIdx * param.singleCoreN;
}

template <typename TilingType, bool isTiling = false, uint32_t baseM = 64>
inline void SetTilingTypes(TilingType& tilingApi, optiling::TCubeTiling& tilingData)
{
    tilingApi.SetAType(
        matmul_tiling::TPosition::GM, matmul_tiling::CubeFormat::ND, matmul_tiling::DataType::DT_FLOAT16, IS_TRANS_A);
    tilingApi.SetBType(
        matmul_tiling::TPosition::GM, matmul_tiling::CubeFormat::ND, matmul_tiling::DataType::DT_FLOAT16, IS_TRANS_B);
    tilingApi.SetCType(
        matmul_tiling::TPosition::GM, matmul_tiling::CubeFormat::ND, matmul_tiling::DataType::DT_FLOAT16);
    tilingApi.SetBiasType(
        matmul_tiling::TPosition::GM, matmul_tiling::CubeFormat::ND, matmul_tiling::DataType::DT_FLOAT16);
    tilingApi.SetOrgShape(M, N, K);
    tilingApi.SetShape(M, N, K);
    tilingApi.EnableBias(IS_BIAS);
    tilingApi.SetBufferSpace(-1, -1, -1);
    if constexpr (isTiling) {
        tilingApi.SetFixSplit(baseM, BASE_N, BASE_K);
    } else {
        tilingApi.SetFixSplit(64, 64, 64);
    }
    int64_t res = tilingApi.GetTiling(tilingData);
    if (res == -1) {
        std::cout << "gen tiling failed" << std::endl;
    }
}

void SetL1(optiling::TCubeTiling& tilingData)
{
    tilingData.set_depthA1(1);
    tilingData.set_depthB1(1);
    tilingData.set_stepM(1);
    tilingData.set_stepN(1);
    tilingData.set_stepKa(1);
    tilingData.set_stepKb(1);
}
template <uint32_t baseM, bool isTiling = false>
void GenerateTilingSingleCore(platform_ascendc::PlatformAscendC* ascendcPlatform, uint8_t* tilingBuf);

template <
    uint32_t singleN, uint32_t singleNSplit, uint32_t baseM, uint32_t depthA1Param, uint32_t stepKaParam,
    bool isSplit = false, bool isMdl = false, bool isL1Cache = false, bool isL2Cache = false>
void GenerateTilingMultiCore(platform_ascendc::PlatformAscendC* ascendcPlatform, uint8_t* tilingBuf);

#endif
