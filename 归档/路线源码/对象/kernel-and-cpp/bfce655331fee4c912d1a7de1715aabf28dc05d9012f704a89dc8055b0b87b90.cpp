/**
 * Copyright (c) 2026 Huawei Technologies Co., Ltd.
 * This program is free software, you can redistribute it and/or modify it under the terms and conditions of
 * CANN Open Software License Agreement Version 2.0 (the "License").
 * Please refer to the License for details. You may not use this file except in compliance with the License.
 * THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
 * See LICENSE in the root of the software repository for the full text of the License.
 */

#include <torch/extension.h>
#include <torch/library.h>
#include "ops.h"

namespace {

// [MODIFY] 注册算子签名 - manual_vector_reduction → <your_op>
TORCH_LIBRARY_FRAGMENT(npu, m)
{
    m.def("manual_vector_reduction(Tensor x1, Tensor x2) -> Tensor");
}

// [MODIFY] 绑定 NPU 实现 - manual_vector_reduction → <your_op>，保持 _torch 后缀
TORCH_LIBRARY_IMPL(npu, PrivateUse1, m)
{
    m.impl("manual_vector_reduction", TORCH_FN(ascend_kernel::manual_vector_reduction_torch));
}

// [MODIFY] 绑定 Meta 实现（torch.compile / fx 需要）
// Meta 函数只推导输出 shape/dtype，不执行实际计算。
// 简单算子可直接返回 at::empty_like；多输出或 shape 变化的算子需按实际逻辑推导。
at::Tensor manual_vector_reduction_meta(const at::Tensor& x1, const at::Tensor& x2)
{
    return at::empty_like(x1);
}

TORCH_LIBRARY_IMPL(npu, Meta, m)
{
    m.impl("manual_vector_reduction", &manual_vector_reduction_meta);
}

} // namespace
