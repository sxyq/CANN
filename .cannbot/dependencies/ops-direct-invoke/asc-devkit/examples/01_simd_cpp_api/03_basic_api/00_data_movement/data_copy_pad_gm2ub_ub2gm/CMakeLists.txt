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
#   1 - 输入[1, 20]，输出[1, 32]，half类型，使用SetPadValue填充
#   2 - 输入[32, 59]，输出[32, 64]，float类型，rightPadding，不使用SetPadValue
#   3 - 输入[3, 24]，输出[1, 80]，half类型，Compact模式
#   4 - 输入[1, 320]，输出[1, 576]，int8类型，SetLoopModePara loop mode（Compact模式）
#   5 - 输入[1, 320]，输出[1, 576]，int8类型，SetLoopModePara loop mode（Normal模式）
#   6 - 输入[2, 4, 3, 128, 126]五维，搬运[2, 2, 2, 64, 126]，int8类型，SetLoopModePara loop mode（Normal模式）
set(SCENARIO_NUM "1" CACHE STRING "Scene number (1-6)")

if(NOT SCENARIO_NUM MATCHES "^[1-6]$")
    message(FATAL_ERROR "SCENARIO_NUM must be 1, 2, 3, 4, 5 or 6, but got ${SCENARIO_NUM}")
endif()

# 场景3/4/5/6仅支持dav-3510
if(SCENARIO_NUM STREQUAL "3" OR SCENARIO_NUM STREQUAL "4" OR SCENARIO_NUM STREQUAL "5" OR SCENARIO_NUM STREQUAL "6")
    if(CMAKE_ASC_ARCHITECTURES STREQUAL "dav-2201")
        message(FATAL_ERROR "SCENARIO_NUM=${SCENARIO_NUM} only supports dav-3510, not dav-2201")
    endif()
endif()

add_definitions(-DSCENARIO_NUM=${SCENARIO_NUM})

add_executable(demo
    data_copy_pad.asc
)

target_compile_options(demo PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
)
