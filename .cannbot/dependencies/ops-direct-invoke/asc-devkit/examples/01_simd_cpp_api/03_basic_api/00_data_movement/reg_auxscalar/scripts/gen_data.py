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
    """AuxScalar样例：从UB读取4个标量，每个scalar参与128个元素的Adds计算"""
    total_length = 512
    block_size = 128  # 每个scalar参与的元素数
    data_type = np.float16
    # 生成输入向量 x
    x = np.random.uniform(1, 2, [1, total_length]).astype(data_type)
    # 生成4个标量值（存放于UB），每个scalar参与128个元素的计算
    scalar_values = np.array([1.5, 2.5, 3.5, 4.5], dtype=data_type)
    # DataCopy要求32字节对齐，16个half = 32字节，后面补0
    scalar = np.zeros(16, dtype=data_type)
    scalar[0:4] = scalar_values
    # 计算真值：z[i*128..(i+1)*128-1] = x[i*128..(i+1)*128-1] + scalar[i]
    golden = np.zeros_like(x)
    for i in range(4):
        golden[0, i * block_size : (i + 1) * block_size] = (
            x[0, i * block_size : (i + 1) * block_size] + scalar_values[i]
        )
    os.makedirs("input", exist_ok=True)
    os.makedirs("output", exist_ok=True)
    x.tofile("./input/input_x.bin")
    scalar.tofile("./input/input_y.bin")
    golden.tofile("./output/golden.bin")


if __name__ == "__main__":
    gen_golden_data()
