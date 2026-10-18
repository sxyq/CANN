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


def gen_golden_data(scenario_num=1):
    """
    根据场景编号生成输入数据和Golden数据
    场景1：三个核读取相同输入，input_x全1，input_y全2，golden = 2 + 1*3 = 5（原子累加）
    场景2：三个核读取不同输入，每个输入包含随机值，golden = max(input_x0, input_x1, input_x2)（原子最大值比较）
    """
    input_type = np.float16
    output_type = input_type
    block_length = 256
    use_core_num = 3

    input_shape = [block_length]
    output_shape = input_shape

    os.makedirs("input", exist_ok=True)
    os.makedirs("output", exist_ok=True)

    if scenario_num == 1:
        input_x = np.ones(input_shape).astype(input_type)
        input_y = 2 * np.ones(input_shape).astype(input_type)
        golden = input_y + use_core_num * input_x
        input_x.tofile("./input/input_x.bin")
        input_y.tofile("./input/input_y.bin")
        golden.tofile("./output/golden.bin")
    elif scenario_num == 2:
        np.random.seed(9)
        input_x0 = np.random.uniform(1, 5, input_shape).astype(input_type)
        input_x1 = np.random.uniform(3, 7, input_shape).astype(input_type)
        input_x2 = np.random.uniform(2, 6, input_shape).astype(input_type)

        input_x0.tofile("./input/input_x0.bin")
        input_x1.tofile("./input/input_x1.bin")
        input_x2.tofile("./input/input_x2.bin")

        input_y = np.zeros(input_shape).astype(input_type)
        input_y.tofile("./input/input_y.bin")

        golden = np.maximum.reduce([input_x0, input_x1, input_x2])
        golden.tofile("./output/golden.bin")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-scenarioNum", type=int, default=1, choices=[1, 2])
    args = parser.parse_args()
    gen_golden_data(args.scenarioNum)
