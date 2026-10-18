#!/usr/bin/python3
# coding=utf-8

# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2025 Huawei Technologies Co., Ltd.
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
    np.random.seed(0)

    # 输入shape: [32, 136] (Pattern=RA: rLength=32, aLength=136)
    dtype = np.float32
    x_gm = np.random.uniform(-10, 10, [32, 136]).astype(dtype)

    # Pattern=1 (RA模式): 输入[rLength, aLength] = [32, 136]
    # 沿axis=0（第一个维度）归约，对每列的32个元素求最大值
    # 输出shape为[aLength] = [136]
    y_gm = np.max(x_gm, axis=0).astype(dtype)

    os.makedirs("input", exist_ok=True)
    x_gm.tofile("./input/input_x.bin")
    os.makedirs("output", exist_ok=True)
    y_gm.tofile("./output/golden.bin")


if __name__ == "__main__":
    gen_golden_data()
