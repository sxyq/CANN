#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 2 ]]; then
    echo "usage: $0 <correctness|local> <device-id> [remote-output-path]" >&2
    exit 2
fi

MODE="$1"
DEVICE="$2"
REMOTE_DIR="/home/data4t2/lelinfeng/phase4-review-repro/ADAPTIVE-CORE-OWNERSHIP-CHAMPION-X/V035"
if [[ "${MODE}" != "correctness" && "${MODE}" != "local" ]]; then
    echo "mode must be correctness or local" >&2
    exit 2
fi
if [[ "${MODE}" == "local" && $# -ne 3 ]]; then
    echo "local mode requires a new remote output path" >&2
    exit 2
fi

# 设备准入：唯一条件是目标 NPU FREE_HBM >= 100 MB。
# 不检查独占设备、lease、load window、AICore 或 VLLM；这些只作为测量上下文记录。
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

shift 2
ssh -o BatchMode=yes -o ConnectTimeout=10 cann-server3 \
    "source /usr/local/Ascend/ascend-toolkit/set_env.sh >/dev/null 2>&1 && cd '${REMOTE_DIR}' && ./build-server3-v035/adaptive_probe --mode '${MODE}' --device '${DEVICE}' ${1:+--output '$1'}"
