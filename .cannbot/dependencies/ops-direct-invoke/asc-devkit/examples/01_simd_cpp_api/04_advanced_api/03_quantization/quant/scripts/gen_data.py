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
import sys


def gen_golden_data_simple(npu_arch):
    shape = [1024]

    # 根据不同架构生成不同精度的 src
    if npu_arch == "dav-2201":
        src = np.random.uniform(low=-4, high=4, size=shape).astype(np.float16)
    else:
        src = np.random.uniform(low=-4, high=4, size=shape).astype(np.float32)

    golden = np.round(src * 2.0 + 0.9).astype(np.int8)
    src = src.astype(np.float32)
    os.makedirs("input", exist_ok=True)
    os.makedirs("output", exist_ok=True)
    src.tofile("./input/input.bin")
    golden.tofile("./output/golden.bin")


if __name__ == "__main__":
    if len(sys.argv) < 2:
        print("用法: python3 gen_data.py -DCMAKE_ASC_ARCHITECTURES=dav-2201")
        sys.exit(1)

    arg = sys.argv[1]
    if arg.startswith("-DCMAKE_ASC_ARCHITECTURES="):
        npu_arch = arg.split("=")[1]
    else:
        npu_arch = arg

    gen_golden_data_simple(npu_arch)
