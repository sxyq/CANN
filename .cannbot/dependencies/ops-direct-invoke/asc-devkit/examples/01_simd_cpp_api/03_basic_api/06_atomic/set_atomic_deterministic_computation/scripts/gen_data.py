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
    x = 1.0
    y = 1e16
    z = -1e16
    data_size = 8
    # 保存数据到文件
    input_type = np.dtype("float32")
    output_type = input_type
    sync_type = np.dtype("int32")

    # 用于存储输出数据的gm初始化的值为0
    input_gm = np.array([0.0] * data_size, dtype=input_type)

    # 三个累加的数据
    input_x = np.array([x] * data_size, dtype=input_type)
    input_y = np.array([y] * data_size, dtype=input_type)
    input_z = np.array([z] * data_size, dtype=input_type)

    # 证明浮点数累加不具备交换律
    left = (input_x + input_y) + input_z
    right = input_x + (input_y + input_z)

    # 预期输出数据
    golden = np.array([right[0] + input_gm[0]] * data_size, dtype=output_type)

    # 用于核间同步，在多个核之间共享的gm，gm必须初始为0
    input_sync = np.zeros(256, dtype=sync_type)
    # 创建目录
    os.makedirs("input", exist_ok=True)
    os.makedirs("output", exist_ok=True)

    # 保存数据
    input_x.tofile("./input/input_x.bin")
    input_y.tofile("./input/input_y.bin")
    input_z.tofile("./input/input_z.bin")
    golden.tofile("./output/golden.bin")
    input_gm.tofile("./input/input_gm.bin")
    input_sync.tofile("./input/input_sync.bin")


if __name__ == "__main__":
    gen_golden_data()
