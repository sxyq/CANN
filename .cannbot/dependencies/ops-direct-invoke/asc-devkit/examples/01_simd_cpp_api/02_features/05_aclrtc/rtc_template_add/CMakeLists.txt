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
project(rtc_template_add LANGUAGES CXX)

# 注意：Aclrtc仅支持NPU模式，不支持cpu/sim调试模式
set(CMAKE_ASC_ARCHITECTURES "dav-2201" CACHE STRING "NPU architecture: dav-2201, dav-3510")

add_executable(demo rtc_template_add.cpp)

target_include_directories(demo PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    $ENV{ASCEND_HOME_PATH}/include
)

target_link_directories(demo PRIVATE $ENV{ASCEND_HOME_PATH}/lib64)

target_link_libraries(demo PRIVATE ascendcl acl_rtc)

target_compile_definitions(demo PRIVATE ACLRTC_NPU_ARCH="${CMAKE_ASC_ARCHITECTURES}")
