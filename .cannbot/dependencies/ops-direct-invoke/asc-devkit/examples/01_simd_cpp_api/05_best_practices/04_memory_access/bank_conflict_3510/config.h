/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under
 * the terms and conditions of CANN Open Software License Agreement Version 2.0
 * (the "License"). Please refer to the License for details. You may not use
 * this file except in compliance with the License. THIS SOFTWARE IS PROVIDED ON
 * AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS
 * FOR A PARTICULAR PURPOSE. See LICENSE in the root of the software repository
 * for the full text of the License.
 */

#ifndef BANK_CONFLICT_CONFIG_H
#define BANK_CONFLICT_CONFIG_H

#include "kernel_operator.h"

#if SCENARIO_NUM == 1
// Case 1：二读一写，src0/src1同bank，dst不同bank，读读冲突
constexpr uint32_t TOTAL_LENGTH = 4096;
constexpr uint32_t xLocalAddr = 0x0000;
constexpr uint32_t yLocalAddr = 0x5000;
constexpr uint32_t zLocalAddr = 0xA100;
using T1 = float;
using T2 = float;
using T3 = float;
#elif SCENARIO_NUM == 2
// Case 2：二读一写，src0/dst同bank（不同行），src0/src1同bank group，读写冲突
constexpr uint32_t TOTAL_LENGTH = 4096;
constexpr uint32_t xLocalAddr = 0x0000;
constexpr uint32_t yLocalAddr = 0x5100;
constexpr uint32_t zLocalAddr = 0xA000;
using T1 = float;
using T2 = float;
using T3 = float;
#elif SCENARIO_NUM == 3
// Case 3：二读一写，src0与dst地址重叠读写冲突：src0=dst=bank0, src1=bank1
constexpr uint32_t TOTAL_LENGTH = 4096;
constexpr uint32_t xLocalAddr = 0x0000;
constexpr uint32_t yLocalAddr = 0x5100;
constexpr uint32_t zLocalAddr = 0x0000;
using T1 = float;
using T2 = float;
using T3 = float;
#elif SCENARIO_NUM == 4
// Case 4：二读一写，src1与dst同bank（不同行），src0/src1同bank group，读写冲突
constexpr uint32_t TOTAL_LENGTH = 4096;
constexpr uint32_t xLocalAddr = 0x0000;
constexpr uint32_t yLocalAddr = 0x5100;
constexpr uint32_t zLocalAddr = 0xA100;
using T1 = float;
using T2 = float;
using T3 = float;
#elif SCENARIO_NUM == 5
// Case 5：二读一写，src1与dst地址重叠读写冲突：src1=dst=bank1, src0=bank0
constexpr uint32_t TOTAL_LENGTH = 4096;
constexpr uint32_t xLocalAddr = 0x0000;
constexpr uint32_t yLocalAddr = 0x5100;
constexpr uint32_t zLocalAddr = 0x5100;
using T1 = float;
using T2 = float;
using T3 = float;
#elif SCENARIO_NUM == 6
// Case 6：二读一写，src0/src1/dst同bank，全冲突(读读+读写)，无循环展开
constexpr uint32_t TOTAL_LENGTH = 4096;
constexpr uint32_t xLocalAddr = 0x0000;
constexpr uint32_t yLocalAddr = 0x5000;
constexpr uint32_t zLocalAddr = 0xA000;
using T1 = float;
using T2 = float;
using T3 = float;
#elif SCENARIO_NUM == 7
// Case 7：二读一写，全冲突 + 循环展开优化：同Case 6地址
constexpr uint32_t TOTAL_LENGTH = 4096;
constexpr uint32_t xLocalAddr = 0x0000;
constexpr uint32_t yLocalAddr = 0x5000;
constexpr uint32_t zLocalAddr = 0xA000;
using T1 = float;
using T2 = float;
using T3 = float;
#elif SCENARIO_NUM == 8
// Case 8：二读一写，同Case6地址，计算bound（增加大量计算，观测LDU/STU变化）
constexpr uint32_t TOTAL_LENGTH = 4096;
constexpr uint32_t xLocalAddr = 0x0000;
constexpr uint32_t yLocalAddr = 0x5000;
constexpr uint32_t zLocalAddr = 0xA000;
using T1 = float;
using T2 = float;
using T3 = float;
#elif SCENARIO_NUM == 9
// Case 9：离散读写，Gather数据索引[0..255]连续，跨bank分散
constexpr uint32_t TOTAL_LENGTH = 256;
constexpr uint32_t xLocalAddr = 0x0000;
constexpr uint32_t yLocalAddr = 0x0400;
constexpr uint32_t zLocalAddr = 0x0400;
using T1 = uint32_t;
using T2 = float;
using T3 = float;
#elif SCENARIO_NUM == 10
// Case 10：离散读写，Gather数据索引0、128交替重复，同subbank不同行冲突
constexpr uint32_t TOTAL_LENGTH = 256;
constexpr uint32_t xLocalAddr = 0x0000;
constexpr uint32_t yLocalAddr = 0x0400;
constexpr uint32_t zLocalAddr = 0x0400;
using T1 = uint32_t;
using T2 = float;
using T3 = float;
#elif SCENARIO_NUM == 11
// Case 11：一读一写，DataBlock stride=1 横着搬运（按行），repeat间间隔256B
constexpr uint32_t TOTAL_LENGTH = 512;
constexpr uint32_t xLocalAddr = 0x0000;
constexpr uint32_t yLocalAddr = 0x0800;
constexpr uint32_t zLocalAddr = 0x4100;
using T1 = float;
using T2 = float;
using T3 = float;
#elif SCENARIO_NUM == 12
// Case 12：一读一写，DataBlock stride=8 竖着搬运（按列），repeat间间隔32B
constexpr uint32_t TOTAL_LENGTH = 512;
constexpr uint32_t xLocalAddr = 0x0000;
constexpr uint32_t yLocalAddr = 0x0800;
constexpr uint32_t zLocalAddr = 0x4100;
using T1 = float;
using T2 = float;
using T3 = float;
#elif SCENARIO_NUM == 13
// Case 13：一读一写，DataBlock srcBlockStride=0，同一repeat内同地址重复读取
constexpr uint32_t TOTAL_LENGTH = 512;
constexpr uint32_t xLocalAddr = 0x0000;
constexpr uint32_t yLocalAddr = 0x0800;
constexpr uint32_t zLocalAddr = 0x4100;
using T1 = float;
using T2 = float;
using T3 = float;
#elif SCENARIO_NUM == 14
// Case 14：一读一写，DataBlock srcRepeatStride=0，repeat间从同一首地址读取
constexpr uint32_t TOTAL_LENGTH = 512;
constexpr uint32_t xLocalAddr = 0x0000;
constexpr uint32_t yLocalAddr = 0x0800;
constexpr uint32_t zLocalAddr = 0x4100;
using T1 = float;
using T2 = float;
using T3 = float;
#else
#error "Unsupported SCENARIO_NUM. Valid values: 1-14"
#endif
#endif // BANK_CONFLICT_CONFIG_H
