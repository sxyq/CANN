# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2025-2026 Huawei Technologies Co., Ltd.
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
    mmad_unitflag.asc
)

set(ENABLE_UNITFLAG 1 CACHE STRING "ENABLE_UNITFLAG, e.g. 0 or 1")
if(NOT ENABLE_UNITFLAG MATCHES "^[0-1]$")
    message(FATAL_ERROR "ENABLE_UNITFLAG must be 0 or 1, but got ${ENABLE_UNITFLAG}")
endif()

message(STATUS "[INFO] ENABLE_UNITFLAG is ${ENABLE_UNITFLAG}")
target_compile_definitions(demo PRIVATE
    ENABLE_UNITFLAG=${ENABLE_UNITFLAG}
)

target_compile_options(demo PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
)
