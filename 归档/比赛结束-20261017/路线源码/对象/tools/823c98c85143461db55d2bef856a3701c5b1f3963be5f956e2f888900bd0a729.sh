#!/usr/bin/env bash
set -euo pipefail

toolkit="${ASCEND_CANN_PACKAGE_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
build_dir="${1:-build}"

rm -rf "${build_dir}"
cmake -S . -B "${build_dir}" \
  -DASCEND_CANN_PACKAGE_PATH="${toolkit}" \
  -DSOC_VERSION=Ascend910B3 \
  -DCMAKE_CXX_COMPILER="${toolkit}/aarch64-linux/ccec_compiler/bin/bisheng" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build "${build_dir}" --verbose 2>&1 | tee "${build_dir}/compile.log"
