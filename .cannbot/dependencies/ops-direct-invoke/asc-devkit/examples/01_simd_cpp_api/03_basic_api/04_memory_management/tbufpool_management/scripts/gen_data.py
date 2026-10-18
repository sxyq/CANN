#!/usr/bin/python3
# -*- coding: utf-8 -*-

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


def gen_golden_data(scenarioNum):
    """
    生成测试输入数据和真值数据
    Args:
        scenarioNum: 场景编号(1=TBufPool内存复用,2=TBufPool资源细分,3=自定义TBufPool)
    """
    os.makedirs("input", exist_ok=True)
    os.makedirs("output", exist_ok=True)

    if scenarioNum == 1:
        # 场景1：TBufPool内存复用，shape=[3, 65536]
        total_size = 3 * 65536
        mask = 512
    elif scenarioNum == 2:
        # 场景2：TBufPool资源细分，shape=[4, 32768]
        total_size = 4 * 32768
        mask = 16384
    elif scenarioNum == 3:
        # 场景3：自定义TBufPool，shape=[1, 65536]
        total_size = 65536
        mask = 32768
    else:
        print(f"scenarioNum {scenarioNum} is not supported!")
        return

    x = np.random.uniform(1, 10, [total_size]).astype(np.float16)
    y = np.random.uniform(1, 10, [total_size]).astype(np.float16)
    golden = np.zeros(total_size, dtype=np.float16)
    golden[0:mask] = x[0:mask] + y[0:mask]

    x.tofile("./input/input_x.bin")
    y.tofile("./input/input_y.bin")
    golden.tofile("./output/golden.bin")


if __name__ == "__main__":
    parser = argparse.ArgumentParser()
    parser.add_argument("-scenarioNum", type=int, default=1, choices=range(1, 4))
    args = parser.parse_args()
    gen_golden_data(args.scenarioNum)
