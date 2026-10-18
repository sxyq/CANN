/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#ifndef VERIFY_SINGLE_HEADER
#include "reg_compute/kernel_reg_compute_intf.h"
#else
#include "reg_compute/kernel_reg_compute_utils.h"
#endif

extern "C" __simd_vf__ void utils_test()
{
    // kernel_reg_compute_utils.h defines enums and structs used by other headers
    // Test that the key enums are accessible

    // ExpConfig
    AscendC::ExpConfig expCfg = AscendC::DEFAULT_EXP_CONFIG;
    AscendC::ExpAlgo expAlgo = expCfg.algo;

    // LnConfig
    AscendC::LnConfig lnCfg = AscendC::DEFAULT_LN_CONFIG;
    AscendC::LnAlgo lnAlgo = lnCfg.algo;

    // DivConfig
    AscendC::DivConfig divCfg = AscendC::DEFAULT_DIV_CONFIG;
    AscendC::DivAlgo divAlgo = divCfg.algo;

    // SqrtConfig
    AscendC::SqrtConfig sqrtCfg = AscendC::DEFAULT_SQRT_CONFIG;
    AscendC::SqrtAlgo sqrtAlgo = sqrtCfg.algo;

    // Reg namespace enums
    AscendC::Reg::RegLayout layout = AscendC::Reg::RegLayout::ZERO;
    AscendC::Reg::SatMode satMode = AscendC::Reg::SatMode::SAT;
    AscendC::Reg::IndexOrder order = AscendC::Reg::IndexOrder::INCREASE_ORDER;
    AscendC::Reg::MaskMergeMode mode = AscendC::Reg::MaskMergeMode::ZEROING;
    AscendC::Reg::HighLowPart part = AscendC::Reg::HighLowPart::LOWEST;
    AscendC::Reg::MaskPattern pattern = AscendC::Reg::MaskPattern::ALL;
    AscendC::Reg::LoadDist loadDist = AscendC::Reg::LoadDist::DIST_NORM;
    AscendC::Reg::StoreDist storeDist = AscendC::Reg::StoreDist::DIST_NORM_B32;
    AscendC::Reg::MemType memType = AscendC::Reg::MemType::VEC_STORE;
    AscendC::Reg::ReduceType reduceType = AscendC::Reg::ReduceType::SUM;
    AscendC::RoundMode roundMode = AscendC::RoundMode::CAST_NONE;

    // DefaultType
    AscendC::Reg::DefaultType defaultType;

    // CastTrait
    AscendC::Reg::CastTrait trait = AscendC::Reg::castTrait;

    // Specific modes
    AscendC::Reg::ExpSpecificMode expMode;
    AscendC::Reg::DivSpecificMode divMode;
    AscendC::Reg::SqrtSpecificMode sqrtMode;
}
