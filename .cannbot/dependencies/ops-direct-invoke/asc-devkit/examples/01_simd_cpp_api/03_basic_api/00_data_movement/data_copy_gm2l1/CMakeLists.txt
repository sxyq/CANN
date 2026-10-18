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

set(CMAKE_ASC_ARCHITECTURES "dav-2201" CACHE STRING "NPU architecture: dav-2201, dav-3510")

find_package(ASC REQUIRED)

project(kernel_samples LANGUAGES ASC CXX)

add_executable(demo
    data_copy_gm2l1.asc
)

target_link_libraries(demo PRIVATE)

# 场景参数（通过命令行传入）
# SCENARIO_NUM: 场景编号
#   1 - 输入格式NZ，输入数据类型half
#   2 - 输入格式ND，输入数据类型half
#   3 - 输入格式DN，输入数据类型half
#   4 - 输入格式ND，输入数据类型half，使能bias
#   5 - 输入格式ND，输入数据类型half，使能Vector量化
set(SCENARIO_NUM "1" CACHE STRING "Scene number (1-5)")
if(NOT SCENARIO_NUM MATCHES "^[1-5]$")
    message(FATAL_ERROR "SCENARIO_NUM must be 1, 2, 3, 4, or 5, but got ${SCENARIO_NUM}")
endif()

if(SCENARIO_NUM EQUAL 3 AND CMAKE_ASC_ARCHITECTURES STREQUAL "dav-2201")
    message(FATAL_ERROR "When SCENARIO_NUM=3, only CMAKE_ASC_ARCHITECTURES=dav-3510 is supported.")
endif()

add_definitions(-DSCENARIO_NUM=${SCENARIO_NUM})

target_compile_options(demo PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
)
