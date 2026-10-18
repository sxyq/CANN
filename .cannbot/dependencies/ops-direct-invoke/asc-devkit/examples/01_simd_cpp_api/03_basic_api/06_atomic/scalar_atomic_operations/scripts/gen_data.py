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
    场景1：AtomicAdd原子加操作，三个核分别对GM地址的第一个元素加1
    场景2：AtomicCas原子比较交换操作，三个核分别比较GM地址第一个元素是否为1，若为1则替换为2
    """
    block_length = 256
    use_core_num = 3

    os.makedirs("input", exist_ok=True)
    os.makedirs("output", exist_ok=True)

    if scenario_num == 1:
        input_y = np.ones([block_length], dtype=np.int32)
        input_y.tofile("./input/input_y.bin")
        golden = input_y.copy()
        golden[0] = golden[0] + use_core_num * 1
        golden.tofile("./output/golden.bin")
    elif scenario_num == 2:
        input_y = np.ones([block_length], dtype=np.int32)
        input_y.tofile("./input/input_y.bin")
        golden = input_y.copy()
        golden[0] = 2
        golden.tofile("./output/golden.bin")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-scenarioNum", type=int, default=1, choices=[1, 2])
    args = parser.parse_args()
    gen_golden_data(args.scenarioNum)
