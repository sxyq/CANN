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


DATA_SIZE = 256
CTRL_SIZE = 8


def gen_golden_data():
    """
    生成Golden数据：
    1. input.bin：输入数据（256个half），前128正常值，后128 INF
    2. golden_ctrl.bin：寄存器值验证（8个int64），前4个=1（设置后），后4个=0（reset后）
    3. golden_sat.bin：饱和模式功能验证（256个half），前128正常值+1，后128 INF

    CTRL[48]功能说明：
    - CTRL[48]=0（饱和模式）：INF输出饱和为±MAX，NaN输出饱和为0
    - CTRL[48]=1（非饱和模式）：INF/NaN保持原输出
    - 需配合CTRL[60]=1使能全局饱和模式
    """
    os.makedirs("input", exist_ok=True)
    os.makedirs("output", exist_ok=True)

    src_data = np.zeros(DATA_SIZE, dtype=np.float16)
    src_data[: DATA_SIZE // 2] = np.arange(DATA_SIZE // 2, dtype=np.float16)
    src_data[DATA_SIZE // 2 :] = np.inf
    src_data.tofile("./input/input.bin")

    golden_ctrl = np.zeros(CTRL_SIZE, dtype=np.int64)
    golden_ctrl[: CTRL_SIZE // 2] = 1
    golden_ctrl[CTRL_SIZE // 2 :] = 0
    golden_ctrl.tofile("./output/golden_ctrl.bin")

    golden_sat = np.zeros(DATA_SIZE, dtype=np.float16)
    golden_sat[: DATA_SIZE // 2] = np.arange(DATA_SIZE // 2, dtype=np.float16) + 1
    golden_sat[DATA_SIZE // 2 :] = np.inf
    golden_sat.tofile("./output/golden_sat.bin")


if __name__ == "__main__":
    gen_golden_data()
