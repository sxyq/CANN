#!/usr/bin/env bash
set -euo pipefail

ROUTE_ROOT="/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R15-safe-multirow-dma-x"
LOCAL_DIR="${ROUTE_ROOT}/本地实验/W4-R15/V002"
REMOTE_HOST="cann-server3"
REMOTE_DIR="/home/data4t2/lelinfeng/cann/本地实验/W4-R15/V002"

ssh -o BatchMode=yes -o ConnectTimeout=8 "${REMOTE_HOST}" \
  mkdir -p "${REMOTE_DIR}/support" "${REMOTE_DIR}/build"
scp -q "${LOCAL_DIR}/submission.asc" "${REMOTE_HOST}:${REMOTE_DIR}/submission.asc"
scp -q "${LOCAL_DIR}/support/parent.asc" \
  "${LOCAL_DIR}/support/paired_runner.cpp" \
  "${LOCAL_DIR}/support/paired_runner_abi.h" \
  "${LOCAL_DIR}/support/paired_runner_r15.asc" \
  "${LOCAL_DIR}/support/CMakeLists.txt" \
  "${REMOTE_HOST}:${REMOTE_DIR}/support/"
scp -q "${ROUTE_ROOT}/工具/server3-resource-policy.sh" \
  "${REMOTE_HOST}:${REMOTE_DIR}/support/server3-resource-policy.sh"

ssh -o BatchMode=yes -o ConnectTimeout=8 "${REMOTE_HOST}" \
  env R15_REMOTE_DIR="${REMOTE_DIR}" bash -s <<'REMOTE'
set -euo pipefail
cd "${R15_REMOTE_DIR}"
source "${R15_REMOTE_DIR}/support/server3-resource-policy.sh"
device_id="$(choose_eligible_device 2 3 0 1 4 5 6)" || {
    printf 'COMPILE_STATUS=NOT_RUN\nBLOCKER=ALL_AVAILABLE_NPU_FREE_HBM_LT_100_MB\n'
    exit 20
}
printf 'COMPILE_DEVICE_ID=%s\n' "${device_id}"
record_load_context "${device_id}"
set +u
source /usr/local/Ascend/ascend-toolkit/set_env.sh
set -u
export CPATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11${CPATH:+:${CPATH}}"
printf 'HOST=%s\n' "$(hostname)"
printf 'CANN_COMPILER=%s\n' "$(command -v bisheng)"
printf 'SOC_VERSION=Ascend910B3\nNPU_ARCH=dav-2201\n'
cmake -S "${R15_REMOTE_DIR}/support" -B "${R15_REMOTE_DIR}/build"
cmake --build "${R15_REMOTE_DIR}/build" --target r15_v002_paired_runner --parallel 1
sha256sum "${R15_REMOTE_DIR}/submission.asc" \
    "${R15_REMOTE_DIR}/build/r15_v002_paired_runner"
printf 'COMPILE_STATUS=PASS\n'
REMOTE
