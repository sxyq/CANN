#!/bin/bash
# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------

set -euo pipefail

CASE_REL=01_simd_cpp_api/02_features/99_acl_based/00_acl_compilation/parallel_ops_package
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
source "$SCRIPT_DIR/../../../../../_case_entry.sh"
presmoke_case_init "$CASE_REL"

case_build() {
    local cmake_args=(cmake -S . -B build)
    if [[ -n "${NLOHMANN_JSON_URL:-}" ]]; then
        cmake_args+=("-DNLOHMANN_JSON_URL=$NLOHMANN_JSON_URL")
    fi
    if [[ -n "${RUN_MODE_ARG:-}" ]]; then
        cmake_args+=("$RUN_MODE_ARG")
    fi

    mkdir -p "$BUILD_DIR"
    (cd "$CASE_DIR" && soc_version=$SOC_VERSION presmoke_run_command "${cmake_args[@]}")
    (cd "$CASE_DIR" && soc_version=$SOC_VERSION presmoke_run_command cmake --build build -j)
}

case_run() {
    mkdir -p "$BUILD_DIR"
    (cd "$CASE_DIR" && soc_version=$SOC_VERSION bash -lc './build/add_custom/custom_opp_*.run')
    (cd "$CASE_DIR" && soc_version=$SOC_VERSION bash -lc './build/leaky_relu_custom/custom_opp_*.run')
}

case_verify() {
    mkdir -p "$BUILD_DIR"
    :
}

case_clean() {
    rm -rf "$CASE_DIR/build" "$BUILD_DIR"
}

presmoke_case_main "$@"
