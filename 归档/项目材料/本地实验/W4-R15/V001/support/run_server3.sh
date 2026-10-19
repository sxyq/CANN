#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 1 || ( "$1" != "--correctness" && "$1" != "--paired" ) ]]; then
    printf 'usage: bash run_server3.sh --correctness|--paired\n' >&2
    exit 2
fi

ROUTE_ROOT="/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R15-safe-multirow-dma-x"
REMOTE_HOST="cann-server3"
REMOTE_DIR="/home/data4t2/lelinfeng/cann/本地实验/W4-R15/V001"

ssh -o BatchMode=yes -o ConnectTimeout=8 "${REMOTE_HOST}" \
  env R15_REMOTE_DIR="${REMOTE_DIR}" R15_RUN_MODE="$1" bash -s <<'REMOTE'
set -euo pipefail
cd "${R15_REMOTE_DIR}"
source "${R15_REMOTE_DIR}/support/server3-resource-policy.sh"
device_id="$(choose_eligible_device 0 1 2 3 4 5 6 7)" || {
    printf 'RUN_STATUS=NOT_RUN\nBLOCKER=ALL_AVAILABLE_NPU_FREE_HBM_LT_100_MB\n'
    exit 20
}
printf 'RUN_MODE=%s\nDEVICE_ID=%s\n' "${R15_RUN_MODE}" "${device_id}"
set +u
source /usr/local/Ascend/ascend-toolkit/set_env.sh
set -u
printf 'LOAD_SNAPSHOT=BEFORE\n'
record_load_context "${device_id}"
capture_after() {
    rc=$?
    trap - EXIT
    printf 'LOAD_SNAPSHOT=AFTER\n'
    record_load_context "${device_id}"
    exit "${rc}"
}
trap capture_after EXIT
"${R15_REMOTE_DIR}/build/r15_v001_paired_runner" "${R15_RUN_MODE}" "${device_id}"
trap - EXIT
printf 'LOAD_SNAPSHOT=AFTER\n'
record_load_context "${device_id}"
REMOTE
