#!/usr/bin/env bash
set -euo pipefail

route_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
build_dir="${BUILD_DIR:-${route_dir}/build}"

: "${ASCEND_HOME_PATH:?ASCEND_HOME_PATH must point to CANN 9.0.0}"

rm -rf "${build_dir}"
cmake -S "${route_dir}" -B "${build_dir}" -DNPU_ARCH="${NPU_ARCH:-dav-2201}"
cmake --build "${build_dir}" --parallel "${BUILD_JOBS:-4}"
