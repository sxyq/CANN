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
#include "reg_compute/kernel_reg_compute_common_intf.h"
#endif

extern "C" __simd_vf__ void common_test()
{
    AscendC::Reg::MaskReg a;
    AscendC::Reg::UnalignRegForLoad b;
    AscendC::Reg::UnalignRegForStore c;
    AscendC::Reg::AddrReg d;
    AscendC::Reg::UnalignReg e;
    __local_mem__ uint64_t* f;
}
