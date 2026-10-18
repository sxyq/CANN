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
 * \file kernel_operator_selectwithbytesmask_intf.h
 * \brief
 */

#if defined(__NPU_COMPILER_INTERNAL_PURE_SIMT__)
#error "kernel_operator_selectwithbytesmask_intf.h cannot be used with compile flag --enable-simt enabled."
#endif

#if !defined(__ASCENDC_INCLUDE_INTERNAL_HEADERS__)
#define __ASCENDC_INCLUDE_INTERNAL_HEADERS__
#define __UNDEF_ASCENDC_INCLUDE_INTERNAL_HEADERS_KERNEL_OPERATOR_SELECTWITHBYTESMASK_INTF_H__
#endif

#ifndef LIB_SELECT_KERNEL_OPERATOR_SELECT_WITH_BYTES_MASK_INTF_H
#define LIB_SELECT_KERNEL_OPERATOR_SELECT_WITH_BYTES_MASK_INTF_H
#include "selectwithbytesmask.h"

namespace AscendC {
[[deprecated(
    __FILE__
    " is deprecated, please use sekectwithbytesmask.h instead!")]] typedef void SelectWithBytesMaskDeprecatedHeader;
using AscendCModuleSelectWithBytesMaskInterface = SelectWithBytesMaskDeprecatedHeader;
} // namespace AscendC
#endif // LIB_SELECT_KERNEL_OPERATOR_SELECT_WITH_BYTES_MASK_INTF_H

#if defined(__UNDEF_ASCENDC_INCLUDE_INTERNAL_HEADERS_KERNEL_OPERATOR_SELECTWITHBYTESMASK_INTF_H__)
#undef __ASCENDC_INCLUDE_INTERNAL_HEADERS__
#undef __UNDEF_ASCENDC_INCLUDE_INTERNAL_HEADERS_KERNEL_OPERATOR_SELECTWITHBYTESMASK_INTF_H__
#endif
