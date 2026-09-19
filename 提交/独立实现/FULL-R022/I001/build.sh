#!/usr/bin/env bash
set -euo pipefail

: "${ASCEND_HOME_PATH:?ASCEND_HOME_PATH must point to the CANN toolkit}"
toolchain_include="${ASCEND_HOME_PATH}/aarch64-linux/ascendc/include/basic_api"
mkdir -p build
bisheng -c -x cce --cce-aicore-only --cce-aicore-arch=dav-c220-vec \
    -std=c++17 -O2 -I"${toolchain_include}" kernel.txt -o build/kernel.o
