#!/bin/bash
set -eo pipefail

toolkit=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
source "${toolkit}/aarch64-linux/script/set_env.sh"
export ASCEND_HOME_PATH="${toolkit}"
export ASCEND_CANN_PACKAGE_PATH="${toolkit}"

hcc_root="${toolkit}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
hcc_include_roots="${hcc_root}:${hcc_root}/aarch64-target-linux-gnu:${hcc_root}/backward"
export CPLUS_INCLUDE_PATH="${hcc_include_roots}${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"
export CPATH="${hcc_include_roots}${CPATH:+:${CPATH}}"

source_file=${W5CTL_SOURCE:?W5CTL_SOURCE is required}
artifact_file=${W5CTL_ARTIFACT:?W5CTL_ARTIFACT is required}
source_root=$(cd "$(dirname "${source_file}")" && pwd)
mkdir -p "$(dirname "${artifact_file}")"
build_dir=$(cd "$(dirname "${artifact_file}")" && pwd)

test -f "${source_root}/support/CMakeLists.txt"
test -f "${source_root}/support/runner_main.cpp"
test "$(sha256sum "${source_root}/Parent.asc" | awk '{print $1}')" = \
  "6075d390df86bb56f7db4d025f11c244791f9d8c722cb4091898fa01940cf794"
test "$(sha256sum "${source_root}/Candidate.asc" | awk '{print $1}')" = \
  "3698c229dfb6a4b336e7ee0506eb1ceaf898c1c37c937c0af7d3eef71fbc685d"

cmake -S "${source_root}/support" -B "${build_dir}" \
  -DASCEND_HOME_PATH="${ASCEND_HOME_PATH}" \
  -DCMAKE_BUILD_TYPE=Release
cmake --build "${build_dir}" --parallel 1
test -x "${artifact_file}"
printf 'CROSSROW_PUBLIC_BUILD_DIR=%s\n' "${build_dir}"
printf 'CROSSROW_PUBLIC_ARTIFACT=%s\n' "${artifact_file}"
