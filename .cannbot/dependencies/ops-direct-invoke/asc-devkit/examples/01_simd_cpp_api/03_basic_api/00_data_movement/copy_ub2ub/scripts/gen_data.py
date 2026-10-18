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
import argparse
import numpy as np

np.random.seed(9)


def gen_golden_data(scenarioNum=1):
    """
    根据场景编号生成输入数据和Golden数据
    场景1：[1, 512]数据搬运，mask连续模式
    场景2：从[18, 64]搬运[18, 8]数据，mask连续模式
    场景3：从[18, 64]搬运[18, 8]数据，counter模式
    """
    if scenarioNum == 1:
        src_shape = [1, 512]
        dst_shape = [1, 512]
        src_length = 512
        dst_length = 512
    elif scenarioNum == 2:
        src_shape = [18, 64]
        dst_shape = [18, 8]
        src_length = 18 * 64
        dst_length = 18 * 8
    elif scenarioNum == 3:
        src_shape = [18, 64]
        dst_shape = [18, 8]
        src_length = 18 * 64
        dst_length = 18 * 8

    input_data = np.random.randint(-1000, 1000, size=src_length).astype(np.int32)

    if src_length != dst_length:
        src_rows = src_shape[0]
        src_cols = src_shape[1]
        dst_cols = dst_shape[1]
        input_2d = input_data.reshape(src_rows, src_cols)
        golden = input_2d[:, :dst_cols].flatten()
    else:
        golden = input_data.copy()

    os.makedirs("input", exist_ok=True)
    os.makedirs("output", exist_ok=True)
    input_data.tofile("./input/input_x.bin")
    golden.tofile("./output/golden.bin")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-scenarioNum", type=int, default=1, choices=[1, 2, 3])
    args = parser.parse_args()
    gen_golden_data(args.scenarioNum)
