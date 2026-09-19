# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------

# ============================================================================
# CMake 构建配置 - Ascend C Kernel 直调
# ============================================================================
#
# 目录结构:
#   op_kernel/     - NPU 计算层（tiling.h + kernel.asc）
#   op_host/       - Host 直调层（.asc 入口 + data_utils.h）
#   op_extension/  - PyTorch 接入层（torch.cpp + register.cpp + ops.h）
#   scripts/       - 测试脚本
#
# [MODIFY] 创建新算子时修改:
# 1. 所有 add_custom → <your_op>（add_custom 是整体算子名，_kernel/_host/_ops 是固定后缀）
# 2. target_compile_options 中的 --npu-arch
# ============================================================================

cmake_minimum_required(VERSION 3.16)

find_package(ASC REQUIRED)

# [MODIFY] 工程名
project(add_rms_norm_bias LANGUAGES ASC CXX)

set(CMAKE_CXX_STANDARD 17)

set(ACL_INCLUDE_DIR "$ENV{ASCEND_HOME_PATH}/aarch64-linux/include")
set(ACL_LIB_DIR "$ENV{ASCEND_HOME_PATH}/lib64")

# ============================================================================
# Target 1: 可执行文件（直调验证）
# ============================================================================

# [MODIFY] 目标名和源文件名
set(RNB_STANDARD_KERNEL_SOURCE
    "${CMAKE_CURRENT_SOURCE_DIR}/op_kernel/add_rms_norm_bias_kernel.asc")

add_executable(add_rms_norm_bias
    op_host/add_rms_norm_bias.asc
    ${RNB_STANDARD_KERNEL_SOURCE})

target_link_libraries(add_rms_norm_bias PRIVATE
    tiling_api
    register
    platform
    unified_dlog
    dl
    m
    graph_base
)

target_link_directories(add_rms_norm_bias PRIVATE
    /usr/lib/gcc/aarch64-linux-gnu/11
    /usr/lib/aarch64-linux-gnu
)

target_include_directories(add_rms_norm_bias PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/op_kernel
    ${CMAKE_CURRENT_SOURCE_DIR}/op_host
)

# [CONFIG] NPU 架构配置
# - dav-2201: Atlas A2 系列（默认）
# - dav-3510: Atlas A5 系列
# 更多架构详见 npu-arch skill
# [MODIFY] 工程名 - add_custom → <your_op>
target_compile_options(add_rms_norm_bias PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:-isystem/usr/include/c++/11>
    $<$<COMPILE_LANGUAGE:ASC>:-isystem/usr/include/aarch64-linux-gnu/c++/11>
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>
)
