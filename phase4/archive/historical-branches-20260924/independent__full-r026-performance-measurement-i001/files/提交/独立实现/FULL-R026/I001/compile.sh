#!/usr/bin/env bash
set -euo pipefail

route_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
template_dir="${TEMPLATE_DIR:-/home/data4t2/lelinfeng/addrmsnormbias_problem_1742_template}"
stage_dir="${STAGE_DIR:-${route_dir}/build-compile}"

mkdir -p "${stage_dir}"
cp "${route_dir}/main.asc" "${stage_dir}/main.asc"
cp "${route_dir}/CMakeLists.txt" "${stage_dir}/CMakeLists.txt"
cp "${route_dir}/kernel.txt" "${stage_dir}/kernel.asc"

cmake -S "${stage_dir}" -B "${stage_dir}/build" -DNPU_ARCH="${NPU_ARCH:-dav-2201}"
cmake --build "${stage_dir}/build" --parallel 4
