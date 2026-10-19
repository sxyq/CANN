#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
REMOTE_DIR="/home/data4t2/lelinfeng/phase4-review-repro/ADAPTIVE-CORE-OWNERSHIP-CHAMPION-X/V022"
STAMP="$(date -u +%Y%m%dT%H%M%SZ)"
LOCAL_LOG="${ROOT}/logs/server3-transfer-compile-${STAMP}.log"
mkdir -p "${ROOT}/logs"

{
    ssh -o BatchMode=yes -o ConnectTimeout=10 cann-server3 \
        "mkdir -p '${REMOTE_DIR}'"
    scp "${ROOT}/CMakeLists.txt" "${ROOT}/parent.asc" "${ROOT}/submission.asc" \
        "cann-server3:${REMOTE_DIR}/"
    scp -r "${ROOT}/support" "cann-server3:${REMOTE_DIR}/"
    ssh -o BatchMode=yes -o ConnectTimeout=10 cann-server3 \
        "bash '${REMOTE_DIR}/support/build_on_server3.sh'"
} 2>&1 | tee "${LOCAL_LOG}"
