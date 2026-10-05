#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VERSION_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
REMOTE_BASE=/home/data4t2/lelinfeng/cann/server_runs/TINY-MINIMAL-KERNEL-CHAMPION-X/V005
RUN_ID="$(date -u '+%Y%m%dT%H%M%SZ')-$$"
REMOTE_ROOT="${REMOTE_BASE}/${RUN_ID}"
EXPECTED_PARENT_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SHA="$(shasum -a 256 "${VERSION_DIR}/parent.asc" | awk '{print $1}')"
CANDIDATE_SHA="$(shasum -a 256 "${VERSION_DIR}/candidate.asc" | awk '{print $1}')"
STAMP="$(date '+%Y%m%dT%H%M%S%z')"
LOG="${SCRIPT_DIR}/compile-${STAMP}.log"

if [ "${PARENT_SHA}" != "${EXPECTED_PARENT_SHA}" ]; then
    printf 'PARENT_SOURCE_IDENTITY=FAIL actual=%s expected=%s\n' \
        "${PARENT_SHA}" "${EXPECTED_PARENT_SHA}" >&2
    exit 68
fi
if [ -e "${LOG}" ]; then
    printf 'Refusing to overwrite existing evidence: %s\n' "${LOG}" >&2
    exit 66
fi

{
    printf 'ROUTE=TINY-MINIMAL-KERNEL-CHAMPION-X\nREVISION=V005\n'
    printf 'DIRECT_PARENT=R31B-V011\nPARENT_SOURCE_SHA=%s\n' "${PARENT_SHA}"
    printf 'CANDIDATE_SOURCE_SHA=%s\nREMOTE_HOST=cann-server3\n' "${CANDIDATE_SHA}"
    printf 'REMOTE_ROOT=%s\n' "${REMOTE_ROOT}"
    ssh cann-server3 "set -euo pipefail; test ! -e '${REMOTE_ROOT}'; mkdir -p '${REMOTE_ROOT}/src'"
    scp "${VERSION_DIR}/parent.asc" \
        "${VERSION_DIR}/candidate.asc" \
        "${SCRIPT_DIR}/CMakeLists.txt" \
        "${SCRIPT_DIR}/paired_runner_abi.h" \
        "${SCRIPT_DIR}/paired_runner.cpp" \
        "${SCRIPT_DIR}/tiny_runner_v005.asc" \
        "cann-server3:${REMOTE_ROOT}/src/"
    ssh cann-server3 "bash -s -- '${REMOTE_ROOT}' '${PARENT_SHA}' '${CANDIDATE_SHA}'" <<'REMOTE'
set -euo pipefail
REMOTE_ROOT="$1"
EXPECTED_PARENT_SHA="$2"
EXPECTED_CANDIDATE_SHA="$3"
export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}
set +u
source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh
set -u
HCC_TOOLCHAIN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0:${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:${HCC_TOOLCHAIN}/aarch64-target-linux-gnu/include/c++/7.3.0/backward:${CPLUS_INCLUDE_PATH:-}"
PARENT_SOURCE="${REMOTE_ROOT}/src/parent.asc"
CANDIDATE_SOURCE="${REMOTE_ROOT}/src/candidate.asc"
actual_parent="$(sha256sum "${PARENT_SOURCE}" | awk '{print $1}')"
actual_candidate="$(sha256sum "${CANDIDATE_SOURCE}" | awk '{print $1}')"
printf 'REMOTE_PARENT_SOURCE_SHA=%s\n' "${actual_parent}"
printf 'REMOTE_CANDIDATE_SOURCE_SHA=%s\n' "${actual_candidate}"
if [ "${actual_parent}" != "${EXPECTED_PARENT_SHA}" ] || \
   [ "${actual_candidate}" != "${EXPECTED_CANDIDATE_SHA}" ]; then
    printf 'REMOTE_SOURCE_IDENTITY=FAIL\n' >&2
    exit 68
fi
printf 'REMOTE_HOST=%s\n' "$(hostname)"
printf 'CANN_VERSION=%s\n' "$(grep -m1 'VERSION=' /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/version.info || true)"
cmake -S "${REMOTE_ROOT}/src" -B "${REMOTE_ROOT}/build"
cmake --build "${REMOTE_ROOT}/build" --target tiny_runner_v005 --parallel 1
printf 'RUNNER_SHA=%s\n' "$(sha256sum "${REMOTE_ROOT}/build/tiny_runner_v005" | awk '{print $1}')"
printf 'COMPILE_RESULT=PASS\n'
REMOTE
} 2>&1 | tee "${LOG}"
