#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
REMOTE_DIR="/home/data4t2/lelinfeng/cann/w4/R11-row-remainder-balance-x/V002"
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
LOCAL_LOG="${ROOT}/logs/server3-transfer-compile-${STAMP}.log"
mkdir -p "${ROOT}/logs"

{
    FREE_HBM_MB="$(ssh -o BatchMode=yes -o ConnectTimeout=10 cann-server3 \
        "source /usr/local/Ascend/ascend-toolkit/set_env.sh >/dev/null 2>&1; npu-smi info -t usages -i 0" | awk '
        /HBM Capacity\(MB\)/ { capacity = $NF }
        /^[[:space:]]*HBM Usage Rate\(%\)/ { rate = $NF }
        END { if (capacity == "" || rate == "") exit 1; printf "%d", capacity * (100-rate) / 100 }
    ' )"
    echo "COMPILE_DEVICE=0 FREE_HBM_MB=${FREE_HBM_MB}"
    if [[ "${FREE_HBM_MB}" -lt 100 ]]; then
        echo "RESOURCE_BLOCKER=FREE_HBM_BELOW_100MB" >&2
        exit 75
    fi
    ssh -o BatchMode=yes -o ConnectTimeout=10 cann-server3 \
        "mkdir -p '/home/data4t2/lelinfeng/cann/w4/R11-row-remainder-balance-x' && mkdir '${REMOTE_DIR}'"
    scp "${ROOT}/CMakeLists.txt" "${ROOT}/parent.asc" "${ROOT}/submission.asc" \
        "cann-server3:${REMOTE_DIR}/"
    scp -r "${ROOT}/support" "cann-server3:${REMOTE_DIR}/"
    ssh -o BatchMode=yes -o ConnectTimeout=10 cann-server3 \
        "bash '${REMOTE_DIR}/support/build_on_server3.sh'"
} 2>&1 | tee "${LOCAL_LOG}"
