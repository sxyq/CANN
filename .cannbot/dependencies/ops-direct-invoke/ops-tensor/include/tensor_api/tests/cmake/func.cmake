# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2025 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------
function(run_llt_test)
    cmake_parse_arguments(LLT "" "TARGET;TASK_NUM;ENV_FILE" "" ${ARGN})

    string(REPLACE "," ";" TARGET_LIST "${TEST_MOD}")
    list(REMOVE_ITEM TARGET_LIST "")
    if (NOT "${TEST_MOD}" STREQUAL "all" AND NOT ${LLT_TARGET} IN_LIST TARGET_LIST)
        return()
    endif()

    if (ENABLE_ASAN)
        execute_process(COMMAND ${CMAKE_C_COMPILER} -dumpversion OUTPUT_VARIABLE GCC_MAJOR)
        string(REGEX MATCHALL "[0-9]+" GCC_MAJOR ${GCC_MAJOR})
        if("${CMAKE_HOST_SYSTEM_PROCESSOR}" STREQUAL "x86_64")
            set(LD_PRELOAD_ "LD_PRELOAD=/usr/lib/gcc/x86_64-linux-gnu/${GCC_MAJOR}/libasan.so:/usr/lib/gcc/x86_64-linux-gnu/${GCC_MAJOR}/libstdc++.so")
        else()
            set(LD_PRELOAD_ "LD_PRELOAD=/usr/lib/gcc/aarch64-linux-gnu/${GCC_MAJOR}/libasan.so:/usr/lib/gcc/aarch64-linux-gnu/${GCC_MAJOR}/libstdc++.so")
        endif()
        # 谨慎修改 ASAN_OPTIONS_ 取值, 当前出现 ASAN 告警会使 UT 失败.
        set(ASAN_OPTIONS_ "ASAN_OPTIONS=detect_leaks=0:halt_on_error=0")
        add_custom_command(
            TARGET ${LLT_TARGET} POST_BUILD
            COMMAND export LD_LIBRARY_PATH=$ENV{LD_LIBRARY_PATH} && ${LD_PRELOAD_} && ulimit -s 32768 && ${ASAN_OPTIONS_} $<TARGET_FILE:${LLT_TARGET}>
            COMMENT "Run ${LLT_TARGET} with asan"
        )
    else()
        add_custom_command(
            TARGET ${LLT_TARGET} POST_BUILD
            COMMAND export LD_LIBRARY_PATH=$ENV{LD_LIBRARY_PATH} && $<TARGET_FILE:${LLT_TARGET}>
            COMMENT "Run ${LLT_TARGET}"
        )
    endif()

    if(ENABLE_GCOV)
        set(_collect_coverage_data_target collect_coverage_data)

        get_filename_component(_ops_builtin_bin_path ${CMAKE_BINARY_DIR} DIRECTORY)
        get_filename_component(_pkg_path "${ASCEND_CANN_PACKAGE_PATH}/../" REALPATH)
        set(_cov_report ${CMAKE_BINARY_DIR}/cov_report)
        set(_cov_html ${_cov_report})
        set(_cov_data ${_cov_report}/coverage.info)

        if (NOT TARGET ${_collect_coverage_data_target})
            add_custom_target(${_collect_coverage_data_target} ALL
                COMMAND bash ${GENERATE_CPP_COV} ${_ops_builtin_bin_path} ${_cov_data} ${_cov_html} ${_pkg_path}
                COMMENT "Run collect coverage data"
            )
        endif()

        add_dependencies(${_collect_coverage_data_target} ${LLT_TARGET})
    endif()
endfunction(run_llt_test)
