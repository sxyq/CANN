#!/usr/bin/env bash
set -euo pipefail

if [[ "$#" -ne 2 ]]; then
    printf 'usage: %s <remote-attempt-path> <device-id>\n' "$0" >&2
    exit 64
fi
REMOTE_ATTEMPT="$1"
DEVICE="$2"

# 设备准入：唯一条件是目标 NPU FREE_HBM >= 100 MB。
# 不检查独占设备、lease、load window、AICore 或 VLLM；这些只作为测量上下文记录。
MIN_FREE_HBM_MB=100
check_free_hbm() {
    npu-smi info -t usages -i "$1" 2>/dev/null | awk -v min="${MIN_FREE_HBM_MB}" '
        /HBM/ && /[0-9]+(\.[0-9]+)?[[:space:]]*(MiB|GiB)/ {
            value = $NF; unit = $(NF - 1)
            mb = (unit == "GiB") ? value * 1024 : value
            if (mb >= min) { found = 1 }
        }
        END { exit(found ? 0 : 1) }
    '
}
if ! check_free_hbm "${DEVICE}"; then
    printf 'RESOURCE_BLOCKER=ALL_NPU_FREE_HBM_BELOW_%dMB device=%s\n' \
        "${MIN_FREE_HBM_MB}" "${DEVICE}" >&2
    exit 75
fi

SUPPORT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STAMP="$(date -u '+%Y%m%dT%H%M%SZ')"
LOG="${SUPPORT_DIR}/logs/correctness-${STAMP}.log"
if [[ -e "${LOG}" ]]; then
    printf 'Refusing to overwrite correctness evidence: %s\n' "${LOG}" >&2
    exit 66
fi
exec > >(tee -a "${LOG}") 2>&1

printf 'REMOTE_ATTEMPT=%s\nDEVICE=%s\n' "${REMOTE_ATTEMPT}" "${DEVICE}"
printf 'ADMISSION=RULE=FREE_HBM_GE_%sMB\n' "${MIN_FREE_HBM_MB}"
printf 'LOAD_NOTE=load, lease, AICore and VLLM are measurement context only\n'
date -Is
ssh cann-server3 "bash -s -- '${REMOTE_ATTEMPT}' '${DEVICE}'" <<'REMOTE'
set -euo pipefail
ATTEMPT="$1"
DEVICE="$2"
CANN_ROOT=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
source "${CANN_ROOT}/aarch64-linux/script/set_env.sh"
export ASCEND_HOME_PATH="${CANN_ROOT}"
printf 'REMOTE_HOST=%s\n' "$(hostname)"
date -Is
printf 'LOAD_BEFORE\n'
npu-smi info -t usages -i "${DEVICE}"
ps -eo pid=,comm=,args= | grep -E '[v]llm|[p]aired_runner|[u]b_bank_runner' | head -n 80 || true
set +e
"${ATTEMPT}/build/ub_bank_runner" --correctness-only "${DEVICE}"
RUNNER_RC=$?
set -e
printf 'LOAD_AFTER\n'
npu-smi info -t usages -i "${DEVICE}"
printf 'CORRECTNESS_RUNNER_RC=%s\n' "${RUNNER_RC}"
exit "${RUNNER_RC}"
REMOTE
