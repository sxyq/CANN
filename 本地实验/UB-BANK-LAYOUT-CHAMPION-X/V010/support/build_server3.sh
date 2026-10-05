#!/usr/bin/env bash
set -euo pipefail

SUPPORT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REVISION_DIR="$(cd "${SUPPORT_DIR}/.." && pwd)"
REPO_ROOT="$(git -C "${SUPPORT_DIR}" rev-parse --show-toplevel)"
PARENT_SOURCE="${REPO_ROOT}/线上结果/R31B/V011/submission.asc"
CANDIDATE_SOURCE="${REVISION_DIR}/submission.asc"
REMOTE_BASE="/home/data4t2/lelinfeng/server_runs/UB-BANK-LAYOUT-CHAMPION-X/V010"
STAMP="$(date -u '+%Y%m%dT%H%M%SZ')-$$"
REMOTE_ATTEMPT="${REMOTE_BASE}/compile-${STAMP}"
LOG="${SUPPORT_DIR}/logs/compile-${STAMP}.log"

if [[ -e "${LOG}" ]]; then
    printf 'Refusing to overwrite compile evidence: %s\n' "${LOG}" >&2
    exit 66
fi
exec > >(tee -a "${LOG}") 2>&1

printf 'ROUTE=UB-BANK-LAYOUT-CHAMPION-X\nREVISION=V010\n'
printf 'DIRECT_PARENT=R31B-V011\nPARENT_SOURCE_COMMIT=43a1049a1e08e518c88e354a754fdebb85a96f99\n'
printf 'REMOTE_ATTEMPT=%s\n' "${REMOTE_ATTEMPT}"
printf 'LOCAL_HOST=%s\n' "$(hostname)"
date '+%Y-%m-%dT%H:%M:%S%z'
shasum -a 256 "${PARENT_SOURCE}" "${CANDIDATE_SOURCE}"

ssh cann-server3 "mkdir -p '${REMOTE_ATTEMPT}/parent' '${REMOTE_ATTEMPT}/candidate' '${REMOTE_ATTEMPT}/support'"
scp "${PARENT_SOURCE}" "cann-server3:${REMOTE_ATTEMPT}/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc"
scp "${CANDIDATE_SOURCE}" "cann-server3:${REMOTE_ATTEMPT}/candidate/submission.asc"
scp "${SUPPORT_DIR}/CMakeLists.txt" "${SUPPORT_DIR}/paired_runner.asc" \
    "${SUPPORT_DIR}/paired_runner.cpp" "${SUPPORT_DIR}/paired_runner_abi.h" \
    "cann-server3:${REMOTE_ATTEMPT}/support/"

ssh cann-server3 "bash -s -- '${REMOTE_ATTEMPT}'" <<'REMOTE'
set -euo pipefail
ATTEMPT="$1"
CANN_ROOT=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
printf 'REMOTE_HOST=%s\n' "$(hostname)"
date -Is
printf 'PARENT_SOURCE_SHA256='
sha256sum "${ATTEMPT}/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc" | awk '{print $1}'
printf 'CANDIDATE_SOURCE_SHA256='
sha256sum "${ATTEMPT}/candidate/submission.asc" | awk '{print $1}'
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
set +u
source "${CANN_ROOT}/aarch64-linux/script/set_env.sh"
set -u
export ASCEND_HOME_PATH="${CANN_ROOT}"
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:${CANN_ROOT}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:${CANN_ROOT}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:${CANN_ROOT}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"
cmake -S "${ATTEMPT}/support" -B "${ATTEMPT}/build" \
    -DCMAKE_BUILD_TYPE=Release \
    -DASCEND_CANN_PACKAGE_PATH="${CANN_ROOT}"
cmake --build "${ATTEMPT}/build" --parallel 4
test -x "${ATTEMPT}/build/ub_bank_runner"
printf 'COMPILE=PASS\nRUNNER=%s\n' "${ATTEMPT}/build/ub_bank_runner"
REMOTE
