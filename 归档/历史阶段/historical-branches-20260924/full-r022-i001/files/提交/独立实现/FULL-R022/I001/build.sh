#!/usr/bin/env bash
set -euo pipefail

: "${ASCEND_HOME_PATH:?ASCEND_HOME_PATH must point to the CANN toolkit}"
toolchain_include="${ASCEND_HOME_PATH}/aarch64-linux/include"
ascendc_include="${ASCEND_HOME_PATH}/compiler/tikcpp/tikcfw"
ascendc_impl="${ASCEND_HOME_PATH}/compiler/tikcpp/tikcfw/impl"
ascendc_interface="${ASCEND_HOME_PATH}/compiler/tikcpp/tikcfw/interface"
runtime_include="${ASCEND_HOME_PATH}/tools/tikicpulib/lib/include"
acl_include="${ASCEND_HOME_PATH}/runtime/include"
cpp_include="${ASCEND_HOME_PATH}/tools/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
cpp_target_include="${cpp_include}/aarch64-target-linux-gnu"
gcc_include="${ASCEND_HOME_PATH}/tools/hcc/aarch64-target-linux-gnu/include"
sys_include="${ASCEND_HOME_PATH}/tools/hcc/aarch64-target-linux-gnu/sys-include"
mkdir -p build
bisheng -c -x cce --cce-aicore-only --cce-aicore-arch=dav-c220-vec \
    -std=c++17 -O2 -I"${toolchain_include}" -I"${ascendc_include}" \
    -I"${ascendc_impl}" -I"${ascendc_interface}" -I"${runtime_include}" -I"${acl_include}" \
    -I"${cpp_include}" -I"${cpp_target_include}" -I"${gcc_include}" -I"${sys_include}" \
    kernel.txt -o build/kernel.o
