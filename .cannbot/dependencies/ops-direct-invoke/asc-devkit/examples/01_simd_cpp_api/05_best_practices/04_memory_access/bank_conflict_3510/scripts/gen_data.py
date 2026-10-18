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

import numpy as np
import os
import argparse


def gen_golden_add():
    N = 4096
    x = np.random.uniform(1, 100, [N]).astype(np.float32)
    y = np.random.uniform(1, 100, [N]).astype(np.float32)
    return x, y, x + y


def gen_golden_case2(repeat=1):
    N = 4096
    x = np.random.uniform(1, 100, [N]).astype(np.float32)
    y = np.random.uniform(1, 100, [N]).astype(np.float32)
    return x, y, x + repeat * y


def gen_golden_case5(repeat=1):
    N = 4096
    x = np.random.uniform(1, 100, [N]).astype(np.float32)
    y = np.random.uniform(1, 100, [N]).astype(np.float32)
    return x, y, repeat * x + y


def gen_golden_gather(case, repeat=1):
    N = 256
    y = np.random.uniform(1, 100, [N]).astype(np.float32)
    if case == 9:
        indices = np.arange(N, dtype=np.uint32)
    elif case == 10:
        indices = np.empty(N, dtype=np.uint32)
        indices[::2] = 0
        indices[1::2] = 128

    golden = y.copy()
    for pos in np.unique(indices):
        golden[pos] += float(repeat)
    return indices, y, golden


def gen_golden_adds_512():
    N = 512
    x = np.random.uniform(1, 100, [N]).astype(np.float32)
    y = np.random.uniform(1, 100, [N]).astype(np.float32)
    return x, y, x.astype(np.float32)


def gen_golden_block_stride0():
    N = 512
    x = np.random.uniform(1, 100, [N]).astype(np.float32)
    y = np.random.uniform(1, 100, [N]).astype(np.float32)
    golden = np.zeros(N, dtype=np.float32)
    for i in range(8):
        for j in range(64):
            golden[i * 64 + j] = x[i * 64 + j % 8]
    return x, y, golden


def gen_golden_compute_bound():
    N = 4096
    x = np.random.uniform(1, 100, [N]).astype(np.float32)
    y = np.random.uniform(1, 100, [N]).astype(np.float32)
    return x, y, x + 1000.0 + y


def gen_golden_repeat_stride0():
    N = 512
    x = np.random.uniform(1, 100, [N]).astype(np.float32)
    y = np.random.uniform(1, 100, [N]).astype(np.float32)
    golden = np.tile(x[:64], 8)
    return x, y, golden


def gen_golden_data(case=1, repeat=1):
    KERNEL_REPEAT = 1000
    if case in (1, 2, 4, 6, 7):
        x, y, golden = gen_golden_add()
    elif case == 3:
        x, y, golden = gen_golden_case2(KERNEL_REPEAT)
    elif case == 5:
        x, y, golden = gen_golden_case5(KERNEL_REPEAT)
    elif case == 8:
        x, y, golden = gen_golden_compute_bound()
    elif case in (9, 10):
        x, y, golden = gen_golden_gather(case, KERNEL_REPEAT)
    elif case in (11, 12):
        x, y, golden = gen_golden_adds_512()
    elif case == 13:
        x, y, golden = gen_golden_block_stride0()
    elif case == 14:
        x, y, golden = gen_golden_repeat_stride0()
    else:
        raise ValueError(f"Unknown case: {case}")

    os.makedirs("input", exist_ok=True)
    os.makedirs("output", exist_ok=True)
    x.tofile("./input/input_x.bin")
    y.tofile("./input/input_y.bin")
    golden.tofile("./output/golden.bin")


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Generate test data")
    parser.add_argument("--case", "-c", type=int, help="case number", default=1)
    args = parser.parse_args()
    gen_golden_data(args.case)
