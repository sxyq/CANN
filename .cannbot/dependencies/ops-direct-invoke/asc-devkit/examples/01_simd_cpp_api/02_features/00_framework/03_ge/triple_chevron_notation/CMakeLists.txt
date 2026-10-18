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
project(triple_chevron_notation LANGUAGES ASC CXX)

set(CUSTOM_OP_OUTPUT_DIR "${CMAKE_BINARY_DIR}/output/op_graph/lib/linux/${CMAKE_SYSTEM_PROCESSOR}")
set(CUSTOM_OP_INCLUDE_DIR "${CMAKE_BINARY_DIR}/output/op_graph/include")
file(MAKE_DIRECTORY "${CUSTOM_OP_OUTPUT_DIR}" "${CUSTOM_OP_INCLUDE_DIR}")

set(OP_PROTO_HEADER_SOURCE_FILE "${CMAKE_SOURCE_DIR}/add_custom_proto.h")
configure_file("${OP_PROTO_HEADER_SOURCE_FILE}" "${CUSTOM_OP_INCLUDE_DIR}/add_custom_proto.h" COPYONLY)

add_library(add_custom_op SHARED
    add_custom.asc
)
target_include_directories(add_custom_op PRIVATE
    "$ENV{ASCEND_HOME_PATH}/include"
)
target_link_directories(add_custom_op PRIVATE "$ENV{ASCEND_HOME_PATH}/lib64")
set_target_properties(add_custom_op PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY "${CUSTOM_OP_OUTPUT_DIR}"
)

add_executable(demo
    graph.cpp
)
target_include_directories(demo PRIVATE
    "${CMAKE_SOURCE_DIR}"
    "$ENV{ASCEND_HOME_PATH}/include"
    "$ENV{ASCEND_HOME_PATH}/include/graph"
    "$ENV{ASCEND_HOME_PATH}/opp/built-in/op_proto/inc"
)
target_link_directories(demo PRIVATE "$ENV{ASCEND_HOME_PATH}/lib64")
target_link_libraries(demo PRIVATE
    graph
    ge_runner
    graph_base
)
