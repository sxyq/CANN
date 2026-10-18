# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2025 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------


cmake_minimum_required(VERSION 3.16)

set(CMAKE_ASC_ARCHITECTURES "dav-2201" CACHE STRING "NPU architecture: dav-2201, dav-3510")

find_package(ASC REQUIRED)

project(kernel_samples LANGUAGES ASC CXX)

# 场景参数（通过命令行传入）
# SCENARIO_NUM: 场景编号
#   1 - [1, 512]数据搬运，mask连续模式
#   2 - 从[18, 64]搬运[18, 8]数据，mask连续模式
#   3 - 从[18, 64]搬运[18, 8]数据，counter模式
set(SCENARIO_NUM "1" CACHE STRING "Scene number (1,2,3)")

add_definitions(-DSCENARIO_NUM=${SCENARIO_NUM})

add_executable(demo
    copy.asc
)

target_compile_options(demo PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
)
