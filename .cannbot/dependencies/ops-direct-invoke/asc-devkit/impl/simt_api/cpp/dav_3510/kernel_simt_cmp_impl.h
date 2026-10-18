/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#ifndef IMPL_SIMT_API_CPP_DAV_C310_KERNEL_SIMT_CMP_IMPL_H
#define IMPL_SIMT_API_CPP_DAV_C310_KERNEL_SIMT_CMP_IMPL_H

#include "impl/simt_api/cpp/dav_3510/kernel_simt_constant.h"

namespace AscendC {

namespace Simt {

template <typename T>
__SIMT_DEVICE_FUNCTIONS_DECL__ inline bool IsNanImpl(T x)
{
    return __isnan(x);
}

template <typename T>
__SIMT_DEVICE_FUNCTIONS_DECL__ inline bool IsInfImpl(T x)
{
    return __isinf(x);
}

/**
 * This only check positiveFinite, when -inf use this need use Abs to protect
 */
__SIMT_DEVICE_FUNCTIONS_DECL__ inline bool IsPositiveInfImpl(float x)
{
    uint32_t* intX = (uint32_t*)&x;
    return *intX == ConstantsInternal::INF;
}

__SIMT_DEVICE_FUNCTIONS_DECL__ inline bool IsNegativeInfImpl(float x)
{
    uint32_t* intX = (uint32_t*)&x;
    return *intX == ConstantsInternal::NEG_INF;
}

__SIMT_DEVICE_FUNCTIONS_DECL__ inline bool IsFiniteImpl(float x) { return __isfinite(x); }

__SIMT_DEVICE_FUNCTIONS_DECL__ inline bool IsFiniteImpl(half x) { return __isfinite(x); }
} // namespace Simt
} // namespace AscendC
#endif // IMPL_SIMT_API_CPP_DAV_C310_KERNEL_SIMT_CMP_IMPL_H
