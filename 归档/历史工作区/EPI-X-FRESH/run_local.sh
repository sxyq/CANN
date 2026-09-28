#!/usr/bin/env bash
set -euo pipefail

root_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
toolkit_home="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
toolkit_env="${toolkit_home}"
if [ ! -f "${toolkit_env}/set_env.sh" ]; then
    toolkit_env="/usr/local/Ascend/ascend-toolkit"
fi
 [ -f "${toolkit_env}/set_env.sh" ] || { echo "CANN set_env.sh not found" >&2; exit 1; }
set +u
source "${toolkit_env}/set_env.sh"
set -u
export ASCEND_HOME_PATH="${toolkit_home}"
export ASCEND_CANN_PACKAGE_PATH="${toolkit_home}"
export SOC_VERSION="Ascend910B3"
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"
asc_cmake_dir="${toolkit_home}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
if [ ! -d "${asc_cmake_dir}" ]; then
    asc_cmake_dir="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/tikcpp/ascendc_kernel_cmake"
fi
export CMAKE_PREFIX_PATH="${asc_cmake_dir}${CMAKE_PREFIX_PATH:+:${CMAKE_PREFIX_PATH}}"
cmake -S "${root_dir}" -B "${root_dir}/build" -DSOC_VERSION="${SOC_VERSION}"
cmake --build "${root_dir}/build" -j4
"${root_dir}/build/epi_x_fresh"
