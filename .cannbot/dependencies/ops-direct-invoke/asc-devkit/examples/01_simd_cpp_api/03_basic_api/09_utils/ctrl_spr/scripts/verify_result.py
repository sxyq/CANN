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


LOSS = 1e-3
MINIMUM = 10e-10


def verify_ctrl(real_result, golden):
    real_result = np.fromfile(real_result, dtype=np.int64)
    golden = np.fromfile(golden, dtype=np.int64)
    result = np.abs(real_result - golden)
    result_atol = np.less_equal(result, LOSS)
    if not result_atol.all():
        if np.sum(result_atol == False) > real_result.size * LOSS:
            print("[ERROR] result error")
            return False
    return True


def verify_sat(real_result, golden):
    real_result = np.fromfile(real_result, dtype=np.float16)
    golden = np.fromfile(golden, dtype=np.float16)

    for i in range(len(real_result)):
        if np.isinf(golden[i]):
            if not np.isinf(real_result[i]):
                print(f"[ERROR] index {i}: expected inf, got {real_result[i]}")
                return False
        else:
            diff = np.abs(real_result[i] - golden[i])
            if diff > LOSS:
                print(f"[ERROR] index {i}: diff {diff} > {LOSS}")
                return False
    return True


if __name__ == "__main__":
    ctrl_pass = verify_ctrl("./output/output_ctrl.bin", "./output/golden_ctrl.bin")
    if not ctrl_pass:
        print("[ERROR] output_ctrl.bin failed")

    sat_pass = verify_sat("./output/output_sat.bin", "./output/golden_sat.bin")
    if not sat_pass:
        print("[ERROR] output_sat.bin failed")

    if ctrl_pass and sat_pass:
        print("test pass!")
    else:
        print("[ERROR] test failed")
