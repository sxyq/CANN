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
    mmad_gemv.asc
)

set(DISABLE_GEMV 0 CACHE STRING "DISABLE_GEMV, e.g. 0 or 1")
if(NOT DISABLE_GEMV MATCHES "^[0-1]$")
    message(FATAL_ERROR "DISABLE_GEMV must be 0 or 1, but got ${DISABLE_GEMV}")
endif()
if(DISABLE_GEMV STREQUAL "1" AND CMAKE_ASC_ARCHITECTURES STREQUAL "dav-2201")
    message(FATAL_ERROR "DISABLE_GEMV=1 only supports dav-3510, not dav-2201")
endif()

message(STATUS "[INFO] DISABLE_GEMV is ${DISABLE_GEMV}")
target_compile_definitions(demo PRIVATE
    DISABLE_GEMV=${DISABLE_GEMV}
)

target_compile_options(demo PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${CMAKE_ASC_ARCHITECTURES}>
)
