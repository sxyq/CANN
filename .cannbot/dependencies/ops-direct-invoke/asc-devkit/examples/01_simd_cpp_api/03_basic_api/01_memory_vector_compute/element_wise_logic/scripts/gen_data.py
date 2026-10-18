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


COUNT = 512
SHIFT_BITS = 2


def ensure_dirs():
    os.makedirs("input", exist_ok=True)
    os.makedirs("output", exist_ok=True)


def save_data(src0, src1, golden):
    ensure_dirs()
    src0.astype(np.uint16).tofile("./input/input_src0.bin")
    src1.astype(np.uint16).tofile("./input/input_src1.bin")
    golden.astype(np.uint16).tofile("./output/golden.bin")


def gen_golden_data_and():
    src0 = np.arange(1, COUNT + 1, dtype=np.uint16)
    src1 = np.arange(COUNT, 0, -1, dtype=np.uint16)
    golden = src0 & src1
    save_data(src0, src1, golden)


def gen_golden_data_ors():
    src0 = np.full(COUNT, 1, dtype=np.uint16)
    src1 = np.arange(1, COUNT + 1, dtype=np.uint16)
    golden = src0[0] | src1
    save_data(src0, src1, golden)


def gen_golden_data_shiftleft():
    src0 = np.arange(1, COUNT + 1, dtype=np.uint16)
    src1 = np.full(COUNT, SHIFT_BITS, dtype=np.uint16)
    golden = src0 << SHIFT_BITS
    save_data(src0, src1, golden)


def gen_golden_data_shiftright():
    src0 = np.arange(1, COUNT + 1, dtype=np.uint16)
    src1 = np.zeros(COUNT, dtype=np.uint16)
    golden = src0 >> SHIFT_BITS
    save_data(src0, src1, golden)


def gen_golden_data(scenarioNum):
    if scenarioNum == 1:
        gen_golden_data_and()
    elif scenarioNum == 2:
        gen_golden_data_ors()
    elif scenarioNum == 3:
        gen_golden_data_shiftleft()
    elif scenarioNum == 4:
        gen_golden_data_shiftright()


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-scenarioNum", type=int, default=1, choices=[1, 2, 3, 4])
    args = parser.parse_args()
    gen_golden_data(args.scenarioNum)
