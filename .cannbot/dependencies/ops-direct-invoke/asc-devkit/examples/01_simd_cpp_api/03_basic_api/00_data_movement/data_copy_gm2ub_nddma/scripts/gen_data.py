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
import argparse
import numpy as np


def gen_golden_data(scenarioNum=1):
    """
    根据场景编号生成输入数据和Golden数据
    场景1：2D Padding场景，输入[16, 32]，输出[32, 64]，使用constant填充0
    场景2：2D Padding场景，输入[28, 15]，输出[32, 32]，使用最近值填充
    场景3：2D Transpose场景，输入[16, 64]，输出[64, 16]，转置操作
    场景4：2D BroadCast场景，输入[1, 16]，输出[3, 16]，广播操作
    场景5：2D Slice场景，输入[32, 64]，输出[16, 16]，切片操作
    """
    data_type = np.float32

    if scenarioNum == 1:
        input_shape = [16, 32]
        output_shape = [32, 64]

        input_x = np.arange(1, 16 * 32 + 1, 1, data_type).reshape(input_shape)
        padding = ((13, 3), (15, 17))
        golden = np.pad(input_x, padding, mode="constant", constant_values=0)

    elif scenarioNum == 2:
        input_shape = [28, 15]
        output_shape = [32, 32]

        input_x = np.arange(1, 28 * 15 + 1, 1, data_type).reshape(input_shape)
        golden = np.zeros(output_shape, data_type)

        for i in range(output_shape[0]):
            for j in range(output_shape[1]):
                src_i = i - 3
                src_j = j - 11

                if src_i < 0:
                    src_i = 0
                elif src_i >= input_shape[0]:
                    src_i = input_shape[0] - 1

                if src_j < 0:
                    src_j = 0
                elif src_j >= input_shape[1]:
                    src_j = input_shape[1] - 1

                golden[i, j] = input_x[src_i, src_j]
    elif scenarioNum == 3:
        input_shape = [16, 64]
        output_shape = [64, 16]

        input_x = np.arange(1, 16 * 64 + 1, 1, data_type).reshape(input_shape)
        golden = np.zeros(output_shape, data_type)

        for i in range(output_shape[0]):
            for j in range(output_shape[1]):
                golden[i, j] = input_x[j, i]
    elif scenarioNum == 4:
        input_shape = [1, 16]
        output_shape = [3, 16]

        input_x = np.arange(1, 17, 1, data_type).reshape(input_shape)
        golden = np.zeros(output_shape, data_type)

        for i in range(output_shape[0]):
            for j in range(output_shape[1]):
                golden[i, j] = input_x[0, j]
    elif scenarioNum == 5:
        input_shape = [32, 64]
        output_shape = [16, 16]

        input_x = np.arange(1, 32 * 64 + 1, 1, data_type).reshape(input_shape)
        golden = np.zeros(output_shape, data_type)

        for i in range(output_shape[0]):
            for j in range(output_shape[1]):
                golden[i, j] = input_x[i, j]

    os.makedirs("input", exist_ok=True)
    os.makedirs("output", exist_ok=True)
    input_x.tofile("./input/input_x.bin")
    golden.tofile("./output/golden.bin")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-scenarioNum", type=int, default=1, choices=[1, 2, 3, 4, 5])
    args = parser.parse_args()
    gen_golden_data(args.scenarioNum)
