#!/usr/bin/python3
# coding=utf-8
# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------

import os
import numpy as np


def gen_golden_data():
    """Reg::Scatter - 按元素分散到UB"""
    total_length = 128
    data_type = np.float16
    x = np.random.uniform(0, 1, [1, total_length]).astype(data_type)
    # index: uint16_t类型，每个元素对应srcReg中元素在UB中的元素索引（非byte偏移）
    # 倒序分散：src[i] -> dst[127-i]
    index = np.arange(total_length - 1, -1, -1).astype(np.uint16)
    # golden: Scatter后dst按顺序读出 = 倒序的src
    golden = x.flatten()[::-1]
    os.makedirs("input", exist_ok=True)
    os.makedirs("output", exist_ok=True)
    x.tofile("./input/input_x.bin")
    index.tofile("./input/input_y.bin")
    golden.tofile("./output/golden.bin")


if __name__ == "__main__":
    gen_golden_data()
