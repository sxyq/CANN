#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 ]]; then
    echo "usage: $0 <correctness|local> <device-id> [remote-output-path]" >&2
    exit 2
fi

MODE="$1"
DEVICE="$2"
ROOT="$(cd "$(dirname "$0")" && pwd)"
REMOTE_DIR="/home/data4t2/lelinfeng/cann/w4/R11-row-remainder-balance-x/V003"
if [[ "${MODE}" != "correctness" && "${MODE}" != "same" && "${MODE}" != "local" ]]; then
    echo "mode must be correctness, same, or local" >&2
    exit 2
fi
if [[ ("${MODE}" == "local" || "${MODE}" == "same") && $# -ne 3 ]]; then
    echo "same/local mode requires a new local output path" >&2
    exit 2
fi
if [[ ("${MODE}" == "correctness") && $# -ne 2 ]]; then
    echo "correctness mode takes only mode and device" >&2
    exit 2
fi

# 设备准入：唯一条件是目标 NPU FREE_HBM >= 100 MB。
# 不要求独占设备、lease、load window、AICore 或 VLLM；这些只作为测量上下文记录。
MIN_FREE_HBM_MB=100
free_hbm_ok() {
    ssh -o BatchMode=yes -o ConnectTimeout=10 cann-server3 \
        "npu-smi info -t usages -i '${DEVICE}'" 2>/dev/null | awk -v min="${MIN_FREE_HBM_MB}" '
        /HBM Capacity\(MB\)/ { capacity = $NF }
        /^[[:space:]]*HBM Usage Rate\(%\)/ { rate = $NF }
        END {
            if (capacity == "" || rate == "") exit 1
            if (capacity * (100 - rate) / 100 >= min) exit 0
            exit 1
        }
    '
}
if ! free_hbm_ok; then
    echo "RESOURCE_BLOCKER=ALL_NPU_FREE_HBM_BELOW_${MIN_FREE_HBM_MB}MB device=${DEVICE}" >&2
    exit 75
fi
echo "ADMISSION=RULE=FREE_HBM_GE_${MIN_FREE_HBM_MB}MB"
echo "LOAD_NOTE=load, lease, AICore and VLLM are measurement context only"

ssh -o BatchMode=yes -o ConnectTimeout=10 cann-server3 \
    "date -u; hostname; cat /proc/loadavg; npu-smi info; npu-smi info -t usages -i '${DEVICE}'" \
    > "${ROOT}/logs/${MODE}-device${DEVICE}-preflight.txt" 2>&1

shift 2
if [[ "${MODE}" == "correctness" ]]; then
    ssh -o BatchMode=yes -o ConnectTimeout=10 cann-server3 \
        "source /usr/local/Ascend/ascend-toolkit/set_env.sh >/dev/null 2>&1 && cd '${REMOTE_DIR}' && ./build-server3-v003/adaptive_probe --mode '${MODE}' --device '${DEVICE}'"
else
    LOCAL_OUTPUT="$1"
    if [[ "${LOCAL_OUTPUT}" != /* ]]; then LOCAL_OUTPUT="${ROOT}/${LOCAL_OUTPUT}"; fi
    if [[ -e "${LOCAL_OUTPUT}" ]]; then echo "refusing to overwrite existing evidence: ${LOCAL_OUTPUT}" >&2; exit 2; fi
    REMOTE_OUTPUT="${REMOTE_DIR}/results/$(basename "${LOCAL_OUTPUT}")"
    ssh -o BatchMode=yes -o ConnectTimeout=10 cann-server3 \
        "mkdir -p '${REMOTE_DIR}/results' && source /usr/local/Ascend/ascend-toolkit/set_env.sh >/dev/null 2>&1 && cd '${REMOTE_DIR}' && ./build-server3-v003/adaptive_probe --mode '${MODE}' --device '${DEVICE}' --output '${REMOTE_OUTPUT}'"
    mkdir -p "$(dirname "${LOCAL_OUTPUT}")"
    scp "cann-server3:${REMOTE_OUTPUT}" "${LOCAL_OUTPUT}"
fi

ssh -o BatchMode=yes -o ConnectTimeout=10 cann-server3 \
    "date -u; hostname; cat /proc/loadavg; npu-smi info; npu-smi info -t usages -i '${DEVICE}'" \
    > "${ROOT}/logs/${MODE}-device${DEVICE}-postflight.txt" 2>&1
