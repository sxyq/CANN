/**
 * Copyright (c) 2025 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

/* !
 * \file antiquantize_tiling_impl.cpp
 * \brief
 */
#include "adv_api/utils/types.h"
#include "adv_api/quantization/antiquantize_tiling.h"

namespace AscendC {
void GetAntiQuantizeTmpBufferFactorSize(
    const AscendC::TensorShape& srcShape, const AscendC::TensorShape& scaleShape, AscendC::TensorDataType inputDataType,
    AscendC::TensorDataType outputDataType, uint32_t& maxLiveNodeCount, uint32_t& extraBuf)
{
    (void)srcShape;
    (void)scaleShape;
    (void)inputDataType;
    (void)outputDataType;
    extraBuf = 0;
    maxLiveNodeCount = 0;
}

void GetAntiQuantizeMaxMinTmpSize(
    const AscendC::TensorShape& srcShape, const AscendC::TensorShape& scaleShape, bool isTranspose,
    AscendC::TensorDataType inputDataType, AscendC::TensorDataType outputDataType, uint32_t& maxValue,
    uint32_t& minValue)
{
    (void)srcShape;
    (void)scaleShape;
    (void)isTranspose;
    (void)inputDataType;
    (void)outputDataType;
    maxValue = 0;
    minValue = 0;
}
} // namespace AscendC
