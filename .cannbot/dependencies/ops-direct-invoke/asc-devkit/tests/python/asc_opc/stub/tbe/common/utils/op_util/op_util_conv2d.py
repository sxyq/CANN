#! /usr/bin/env python3
# -*- coding: UTF-8 -*-
# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2025 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------
"""
Copyright (C) 2021. Huawei Technologies Co., Ltd. All rights reserved.

Description:
"""


def is_conv2d_binary():
    """
    true: create binary variable shape
    false: dynamic variable shape
    """
    return


def replace_conv2d_vector_tvm_shapes(vector_inputs, ins_attrs_options):
    res_shapes = [-1, -1, -1, -1]
    return res_shapes
