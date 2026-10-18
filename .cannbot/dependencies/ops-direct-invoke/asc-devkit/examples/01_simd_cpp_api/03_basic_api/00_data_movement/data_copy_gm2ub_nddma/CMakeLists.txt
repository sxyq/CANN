# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------


cmake_minimum_required(VERSION 3.16)

set(CMAKE_ASC_ARCHITECTURES "dav-3510" CACHE STRING "NPU architecture: dav-3510")

find_package(ASC REQUIRED)

project(kernel_samples LANGUAGES ASC CXX)

# 场景参数（通过命令行传入）
# SCENARIO_NUM: 场景编号
#   1 - 2D Padding场景：输入[16, 32]，输出[32, 64]
#   2 - 2D Padding场景，使能最近值填充方式：输入[28, 15]，输出[32, 32]
#   3 - 2D Transpose场景：输入[16, 64]，输出[64, 16]
#   4 - 2D BroadCast场景：输入[1, 16]，输出[3, 16]
#   5 - 2D Slice场景：输入[32, 64]，输出[16, 16]
set(SCENARIO_NUM "1" CACHE STRING "Scene number (1-5)")

add_definitions(-DSCENARIO_NUM=${SCENARIO_NUM})

add_executable(demo
    multidimensional_data_movement.asc
)

target_compile_options(demo PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
)
