#!/usr/bin/env bash
set -euo pipefail

SUPPORT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
V001_DIR="$(cd "${SUPPORT_DIR}/.." && pwd)"
WORKTREE_ROOT="$(cd "${V001_DIR}/../../.." && pwd)"
BUILD_DIR="${SUPPORT_DIR}/build"
ASCEND_HOME_PATH_EXPECTED="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
PARENT_SOURCE="${WORKTREE_ROOT}/线上结果/R31B/V011/submission.asc"
CANDIDATE_SOURCE="${V001_DIR}/submission.asc"

EXPECTED_PARENT_SOURCE_SHA256="a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3"
EXPECTED_CANDIDATE_SOURCE_SHA256="4dc1973ef198cd67693610b6ffedad6883b59f52207b9804052975a13f4e032b"
EXPECTED_PARENT_WRAPPER_SHA256="a9e57afb2dfa8ea5ffbef06988c606e7fd0716d0cd2f816fbf8a8bc15ff58e2c"
EXPECTED_CANDIDATE_WRAPPER_SHA256="d3f893620df8486c16288e9db2f3e3fb1771ddc27acab48d2710ce262d134aa8"
EXPECTED_PERFORMANCE_SOURCE_SHA256="3a73cfad5bfb652506a0b2f91470729ff617af93b89d8a50ed88ff410a3e0197"

check_sha256() {
    local label="$1"
    local path="$2"
    local expected="$3"
    local actual
    actual="$(sha256sum "${path}" | awk '{print $1}')"
    printf '%s=%s\n' "${label}" "${actual}"
    if [[ "${actual}" != "${expected}" ]]; then
        printf 'IDENTITY_MISMATCH=%s expected=%s actual=%s\n' "${label}" "${expected}" "${actual}" >&2
        exit 68
    fi
}

printf 'TIMING_BUILD_PROTOCOL=V001_HBH10\n'
printf 'HOST=%s\n' "$(hostname)"
printf 'BRANCH=%s\n' "$(git -C "${WORKTREE_ROOT}" branch --show-current)"
printf 'COMMIT=%s\n' "$(git -C "${WORKTREE_ROOT}" rev-parse HEAD)"
printf 'ASCEND_HOME_PATH_EXPECTED=%s\n' "${ASCEND_HOME_PATH_EXPECTED}"
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
export PYTHONPATH="${PYTHONPATH:-}"
source /usr/local/Ascend/ascend-toolkit/set_env.sh
export ASCEND_HOME_PATH="${ASCEND_HOME_PATH_EXPECTED}"
printf 'ASCEND_HOME_PATH=%s\n' "${ASCEND_HOME_PATH}"

check_sha256 PARENT_SOURCE_SHA256 "${PARENT_SOURCE}" "${EXPECTED_PARENT_SOURCE_SHA256}"
check_sha256 CANDIDATE_SOURCE_SHA256 "${CANDIDATE_SOURCE}" "${EXPECTED_CANDIDATE_SOURCE_SHA256}"
check_sha256 PARENT_WRAPPER_SHA256 "${SUPPORT_DIR}/parent_wrapper.asc" "${EXPECTED_PARENT_WRAPPER_SHA256}"
check_sha256 CANDIDATE_WRAPPER_SHA256 "${SUPPORT_DIR}/candidate_wrapper.asc" "${EXPECTED_CANDIDATE_WRAPPER_SHA256}"
check_sha256 PERFORMANCE_SOURCE_SHA256 "${SUPPORT_DIR}/performance_main.cpp" "${EXPECTED_PERFORMANCE_SOURCE_SHA256}"

cmake -S "${SUPPORT_DIR}" -B "${BUILD_DIR}" \
    -DASCEND_HOME_PATH="${ASCEND_HOME_PATH}" \
    -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}" --target hotloop_branch_v001_performance -- -j2

test -x "${BUILD_DIR}/hotloop_branch_v001_performance"
printf 'PERFORMANCE_EXECUTABLE_SHA256=%s\n' \
    "$(sha256sum "${BUILD_DIR}/hotloop_branch_v001_performance" | awk '{print $1}')"
printf 'PARENT_LIBRARY_SHA256=%s\n' \
    "$(sha256sum "${BUILD_DIR}/libhotloop_branch_v001_parent.so" | awk '{print $1}')"
printf 'CANDIDATE_LIBRARY_SHA256=%s\n' \
    "$(sha256sum "${BUILD_DIR}/libhotloop_branch_v001_candidate.so" | awk '{print $1}')"
file "${BUILD_DIR}/hotloop_branch_v001_performance"
