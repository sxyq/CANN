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


def gen_welford_coop_data():
    """
    生成WelfordUpdate+WelfordFinalize配合使用测试数据
    """
    RN_SIZE = 1
    AB_SIZE = 64
    AB_COMPUTE_LENGTH = 35
    NREC = 1.0 / 8

    print("Generating welford coop data...")

    # 使用与WelfordUpdate样例相同的随机种子
    np.random.seed(19)

    # 生成输入数据
    x1 = np.random.uniform(1, 100, [RN_SIZE * AB_SIZE]).astype(np.float32)
    x2 = np.random.uniform(-60000, 60000, [RN_SIZE * AB_SIZE]).astype(np.float32)
    x3 = np.random.uniform(0, 60000, [RN_SIZE * AB_SIZE]).astype(np.float32)

    # 计算WelfordUpdate结果
    golden1 = x2.copy()
    golden2 = x3.copy()

    n = np.float32(NREC)
    for i in range(AB_COMPUTE_LENGTH):
        golden1[i] = x2[i] + (x1[i] - x2[i]) * n
        golden2[i] = x3[i] + (x1[i] - x2[i]) * (x1[i] - golden1[i])

    # 对于WelfordFinalize，由于只有一个块
    # 最终结果应该与WelfordUpdate的第一个元素相同
    final_mean = golden1[0]
    final_var = golden2[0]  # 注意：WelfordUpdate输出的就是方差

    # 对齐到8
    final_mean_array = np.zeros(8, dtype=np.float32)
    final_var_array = np.zeros(8, dtype=np.float32)
    final_mean_array[0] = final_mean
    final_var_array[0] = final_var

    # 保存输入
    os.makedirs("input", exist_ok=True)
    x1.tofile("./input/coop_src.bin")
    x2.tofile("./input/coop_inMean.bin")
    x3.tofile("./input/coop_inVar.bin")

    # 保存真值
    os.makedirs("output", exist_ok=True)
    golden1.tofile("./output/golden_outMeanGm.bin")
    golden2.tofile("./output/golden_outVarGm.bin")
    final_mean_array.tofile("./output/golden_coop_finalMean.bin")
    final_var_array.tofile("./output/golden_coop_finalVar.bin")

    print("Data generated successfully")
    print(f"First mean: {final_mean}")
    print(f"First var: {final_var}")
    return True


if __name__ == "__main__":
    gen_welford_coop_data()
