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
 * \file kernel_hif8.cpp
 * \brief
 */
#include <cmath>
#include <algorithm>
#include "kernel_hif8.h"
#include "kernel_fp32.h"
#include "kernel_utils.h"

namespace hifloat8 {

int8_t Hif8T::FloatToHif8(const float src) const { return src; }

Hif8T::operator float() const { return val; }

float Hif8T::ToFloat() const { return val; }
} // namespace hifloat8
