/**
 * Copyright (c) 2025 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

/*!
 * \file broadcast_utils.h
 * \brief
 */

#if !defined(__ASCENDC_INCLUDE_INTERNAL_HEADERS__)
#define __ASCENDC_INCLUDE_INTERNAL_HEADERS__
#define __UNDEF_ASCENDC_INCLUDE_INTERNAL_HEADERS_BROADCAST_UTILS_H__
#endif

#ifndef LIB_PAD_BROADCAST_UTILS_H
#define LIB_PAD_BROADCAST_UTILS_H

#include <cstdint>

namespace AscendC {
struct BroadcastTiling {
    uint32_t oriRank;
    uint32_t rank;
    uint32_t dstSize;
    uint32_t srcSize;
    uint32_t loopNum = 0;
    uint32_t oriSrcShape[9];
    uint32_t oriDstShape[9];
    uint32_t dstShape[9];
    uint32_t dstStride[9];
    uint32_t srcStride[10];
};
} // namespace AscendC
#endif // LIB_PAD_BROADCAST_UTILS_H

#if defined(__UNDEF_ASCENDC_INCLUDE_INTERNAL_HEADERS_BROADCAST_UTILS_H__)
#undef __ASCENDC_INCLUDE_INTERNAL_HEADERS__
#undef __UNDEF_ASCENDC_INCLUDE_INTERNAL_HEADERS_BROADCAST_UTILS_H__
#endif
