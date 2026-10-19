#!/usr/bin/env bash
set -eo pipefail

expected_parent_sha256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
support_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
revision_dir="$(cd "${support_dir}/.." && pwd)"
parent_source="${revision_dir}/submission.asc"
actual_parent_sha256="$(sha256sum "${parent_source}" | awk '{print $1}')"
if [[ ${actual_parent_sha256} != "${expected_parent_sha256}" ]]; then
    printf 'Parent SHA256 mismatch: expected=%s actual=%s\n' \
        "${expected_parent_sha256}" "${actual_parent_sha256}" >&2
    exit 1
fi

export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"
source /usr/local/Ascend/ascend-toolkit/set_env.sh

build_dir="$(mktemp -d "${revision_dir}/build-parent-contract-20261008.XXXXXX")"
printf 'PARENT_BUILD_META source=%s source_sha256=%s candidate_target=OFF build_dir=%s\n' \
    "${parent_source}" "${actual_parent_sha256}" "${build_dir}"
cmake -S "${support_dir}" -B "${build_dir}" \
    -DPARENT_SOURCE="${parent_source}" \
    -DROUTE_EXPECTED_PARENT_SHA256="${expected_parent_sha256}" \
    -DROUTE_BUILD_CANDIDATE_RUNNER=OFF
cmake --build "${build_dir}" --target parent_runner --parallel 1
printf 'PARENT_RUNNER_PATH=%s/parent_runner\n' "${build_dir}"
sha256sum "${build_dir}/parent_runner"
