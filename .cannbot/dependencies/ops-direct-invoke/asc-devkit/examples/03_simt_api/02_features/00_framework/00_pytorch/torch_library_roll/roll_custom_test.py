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

import torch
import torch_npu
from torch_npu.testing.testcase import TestCase, run_tests

torch.ops.load_library("libascendc_roll.so")


class TestCustomRoll(TestCase):
    def test_roll_custom_ops(self):
        length = [8, 2048]
        input = torch.rand(length, device="cpu", dtype=torch.float32)
        output = torch.ops.ascendc_ops.ascendc_roll(input.npu(), [1], [0]).cpu()
        expected = torch.roll(input, [1], [0])
        self.assertRtolEqual(output, expected)


if __name__ == "__main__":
    run_tests()
