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

add_executable(demo
    cast.asc
)

message(STATUS "[INFO] SCENARIO_NUM is ${SCENARIO_NUM}")

if(NOT DEFINED SCENARIO_NUM)
    message(FATAL_ERROR "SCENARIO_NUM is not set. Please specify -DSCENARIO_NUM=0/1")
endif()

set(VALID_SCENARIOS 0 1)
if(NOT SCENARIO_NUM IN_LIST VALID_SCENARIOS)
    message(FATAL_ERROR "SCENARIO_NUM must be 0 or 1, got: ${SCENARIO_NUM}")
endif()

target_compile_definitions(demo PRIVATE
    SCENARIO_NUM=${SCENARIO_NUM}
)

target_compile_options(demo PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
)
