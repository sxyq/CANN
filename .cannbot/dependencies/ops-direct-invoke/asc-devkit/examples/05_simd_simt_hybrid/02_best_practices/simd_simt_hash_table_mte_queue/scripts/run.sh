#!/usr/bin/env bash
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

SCRIPT_DIR=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)
SAMPLE_DIR=$(cd "${SCRIPT_DIR}/.." && pwd)
BUILD_ROOT="${SAMPLE_DIR}/build"
DIMS=(16 32 64 128 256)

run_case() {
    local scenario=$1
    local name=$2
    local build_dir="${BUILD_ROOT}/scenario_${scenario}"

    rm -rf "${build_dir}"
    cmake -S "${SAMPLE_DIR}" -B "${build_dir}" -DSCENARIO_NUM="${scenario}"
    cmake --build "${build_dir}" -j

    for dim in "${DIMS[@]}"; do
        local run_dir="${build_dir}/profile_dim_${dim}"
        local verify_log="${run_dir}/${name}_dim_${dim}_verify.log"
        local log_file="${run_dir}/${name}_dim_${dim}.log"
        rm -rf "${run_dir}"
        mkdir -p "${run_dir}"

        "${build_dir}/hash_table_mte_queue" "${dim}" 2>&1 | tee "${verify_log}"
        grep -q "Verification PASSED" "${verify_log}"
        grep -q "| ${scenario} | ${dim} |" "${verify_log}"

        (
            cd "${run_dir}"
            msopprof "${build_dir}/hash_table_mte_queue" "${dim}" 2>&1 |
                tee "${log_file}"
        )
        if [[ "${scenario}" == "0" ]]; then
            grep -q "insert_or_assign_warp_store_kernel" "${log_file}"
        elif [[ "${scenario}" == "1" ]]; then
            grep -q "insert_or_assign_mte_task_queue_kernel" "${log_file}"
        fi
        find "${run_dir}" -maxdepth 1 -type d -name "OPPROF_*" | grep -q .
    done
}

run_case 0 "simt_warp_insert_or_assign"
run_case 1 "mte_task_queue_insert_or_assign"
