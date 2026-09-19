#!/usr/bin/env bash
set -euo pipefail

: "${ASCEND_HOME_PATH:?ASCEND_HOME_PATH must point to the CANN toolkit}"
toolchain_include="${ASCEND_HOME_PATH}/aarch64-linux/ascendc/include/basic_api"
toolchain_impl="${ASCEND_HOME_PATH}/aarch64-linux/ascendc/include/basic_api/impl"
legacy_include="${ASCEND_HOME_PATH}/aarch64-linux/asc/include/basic_api"
legacy_root="${ASCEND_HOME_PATH}/aarch64-linux/asc/include"
legacy_impl="${ASCEND_HOME_PATH}/aarch64-linux/asc/impl/basic_api"
mkdir -p build
bisheng -c -x cce --cce-aicore-only --cce-aicore-arch=dav-c220-vec \
    -std=c++17 -O2 -I"${toolchain_include}" -I"${toolchain_impl}" \
    -I"${legacy_include}" -I"${legacy_root}" -I"${legacy_impl}" \
    kernel.txt -o build/kernel.o
