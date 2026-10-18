# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------


cmake_minimum_required(VERSION 3.16)

set(CMAKE_ASC_ARCHITECTURES "dav-3510" CACHE STRING "NPU architecture: dav-3510")

find_package(ASC REQUIRED)

project(kernel_samples LANGUAGES ASC CXX)

if(NOT DEFINED SCENARIO_NUM)
    message(FATAL_ERROR "SCENARIO_NUM is not defined. Please set it with -DSCENARIO_NUM=1 or -DSCENARIO_NUM=2")
endif()

if(SCENARIO_NUM EQUAL 1)
    set(SCENARIO_MACRO "SCENARIO_NUM=1")
elseif(SCENARIO_NUM EQUAL 2)
    set(SCENARIO_MACRO "SCENARIO_NUM=2")
else()
    message(FATAL_ERROR "Invalid SCENARIO_NUM: ${SCENARIO_NUM}. Valid values are 1 or 2")
endif()

add_executable(demo
    gather_ld_reg.asc
)

target_compile_definitions(demo PRIVATE
    SCENARIO_NUM=${SCENARIO_NUM}
)

target_compile_options(demo PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
)
