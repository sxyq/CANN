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


def ensure_dirs():
    os.makedirs("input", exist_ok=True)
    os.makedirs("output", exist_ok=True)


def save_data(src0, src1, golden_dst0, golden_dst1):
    ensure_dirs()
    src0.tofile("./input/input_src0.bin")
    src1.tofile("./input/input_src1.bin")
    golden_dst0.tofile("./output/golden_dst0.bin")
    golden_dst1.tofile("./output/golden_dst1.bin")


def gen_golden_data_interleave():
    count = 512
    data_type = np.float16

    src0 = np.arange(1, count + 1).astype(data_type)
    src1 = np.arange(count + 1, count * 2 + 1).astype(data_type)

    golden_dst0 = np.zeros(count).astype(data_type)
    golden_dst1 = np.zeros(count).astype(data_type)

    half_count = count // 2
    for i in range(half_count):
        golden_dst0[i * 2] = src0[i]
        golden_dst0[i * 2 + 1] = src1[i]
        golden_dst1[i * 2] = src0[half_count + i]
        golden_dst1[i * 2 + 1] = src1[half_count + i]

    save_data(src0, src1, golden_dst0, golden_dst1)


def gen_golden_data_deinterleave():
    count = 512
    data_type = np.float16

    src0 = np.arange(1, count + 1).astype(data_type)
    src1 = np.arange(count + 1, count * 2 + 1).astype(data_type)

    golden_dst0 = np.zeros(count).astype(data_type)
    golden_dst1 = np.zeros(count).astype(data_type)

    for i in range(count):
        if i < count // 2:
            golden_dst0[i] = src0[i * 2]
            golden_dst1[i] = src0[i * 2 + 1]
        else:
            golden_dst0[i] = src1[(i - count // 2) * 2]
            golden_dst1[i] = src1[(i - count // 2) * 2 + 1]

    save_data(src0, src1, golden_dst0, golden_dst1)


def gen_golden_data(scenarioNum):
    if scenarioNum == 1:
        gen_golden_data_interleave()
    elif scenarioNum == 2:
        gen_golden_data_deinterleave()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-scenarioNum", type=int, default=1, choices=[1, 2])
    args = parser.parse_args()
    gen_golden_data(args.scenarioNum)
