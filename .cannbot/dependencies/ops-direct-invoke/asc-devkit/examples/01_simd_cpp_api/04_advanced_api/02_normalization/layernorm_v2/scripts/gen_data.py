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


def gen_layernormv2_data():
    A_LENGTH = 32
    R_LENGTH = 32
    R_LENGTH_WITH_PAD = 32
    EPSILON = 0.0001
    dtype = np.float32

    inputX = np.arange(A_LENGTH * R_LENGTH).reshape([A_LENGTH, R_LENGTH]).astype(dtype)
    gamma = np.arange(R_LENGTH).reshape([R_LENGTH]).astype(dtype)
    beta = np.arange(R_LENGTH).reshape([R_LENGTH]).astype(dtype)

    # 计算LayerNorm真值
    # isOutputRstd=true 时输出rstd
    reduce_axis = 1
    mean = np.mean(inputX, reduce_axis, keepdims=True)
    variance = np.mean(np.power((inputX - mean), 2), reduce_axis, keepdims=True)
    variance_with_eps = variance + EPSILON
    rstd = 1.0 / np.sqrt(variance_with_eps)
    layernorm_result = gamma * ((inputX - mean) / np.sqrt(variance_with_eps)) + beta

    # Normalize使用LayerNorm的原始输入inputX，统计量使用LayerNorm计算的mean和rstd
    normalize_result = gamma * ((inputX - mean) / np.sqrt(variance_with_eps)) + beta

    # isOutputRstd=true 时，LayerNorm输出rstd
    rstd_output = rstd

    # 从rstd计算variance用于输出: var = 1/(rstd*rstd) - epsilon
    variance_calc = 1.0 / (rstd * rstd) - EPSILON

    # 保存真值文件
    os.makedirs("output", exist_ok=True)
    layernorm_result.tofile("./output/golden_coop_layernormY.bin")
    mean.reshape(-1).tofile("./output/golden_coop_mean.bin")
    rstd_output.reshape(-1).tofile("./output/golden_coop_rstd.bin")
    variance_calc.reshape(-1).tofile("./output/golden_coop_var.bin")
    normalize_result.tofile("./output/golden_coop_normalizeY.bin")

    # 保存输入文件
    os.makedirs("input", exist_ok=True)
    inputX.tofile("./input/coop_inputX.bin")
    gamma.tofile("./input/coop_gamma.bin")
    beta.tofile("./input/coop_beta.bin")


if __name__ == "__main__":
    gen_layernormv2_data()
