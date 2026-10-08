#!/usr/bin/env bash
set -eo pipefail

if [[ $# -ne 1 || ( $1 != parent_runner && $1 != candidate_runner ) ]]; then
    echo "usage: $0 <parent_runner|candidate_runner>" >&2
    exit 2
fi

support_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
revision_dir="$(cd "${support_dir}/.." && pwd)"
build_candidate=OFF
if [[ $1 == candidate_runner ]]; then
    build_candidate=ON
fi
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"

source /usr/local/Ascend/ascend-toolkit/set_env.sh

cmake -S "${support_dir}" -B "${revision_dir}/build-parent" \
    -DPARENT_SOURCE="${revision_dir}/submission.asc" \
    -DCANDIDATE_SOURCE="${revision_dir}/submission-datacopy.asc" \
    -DROUTE_BUILD_CANDIDATE_RUNNER="${build_candidate}"
cmake --build "${revision_dir}/build-parent" --target "$1" --parallel 1
