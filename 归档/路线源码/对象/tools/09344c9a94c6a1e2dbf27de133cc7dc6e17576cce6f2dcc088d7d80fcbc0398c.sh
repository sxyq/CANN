#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 3 ]; then
    printf 'usage: run-server3.sh --correctness|--parent-qualify|--paired DEVICE REMOTE_ROOT\n' >&2
    exit 64
fi
MODE="$1"
DEVICE="$2"
REMOTE_ROOT="$3"
if [[ ! "${DEVICE}" =~ ^[0-7]$ ]]; then
    printf 'device must be in [0,7]\n' >&2
    exit 64
fi
case "${MODE}" in
    --correctness|--parent-qualify|--paired) ;;
    *)
        printf 'unknown mode: %s\n' "${MODE}" >&2
        exit 64
        ;;
esac
if [[ "${REMOTE_ROOT}" != /home/data4t2/lelinfeng/cann/server_runs/TINY-MINIMAL-KERNEL-CHAMPION-X/V001/* ]]; then
    printf 'remote root is outside this Route experiment directory\n' >&2
    exit 64
fi

# 设备准入：唯一条件是目标 NPU FREE_HBM >= 100 MB。
# MAIN_DEVICE_LEASE 与 TINY_LOAD_WINDOW 不再是执行 Gate，仅作记录保留。
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

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
VERSION_DIR="$(cd "${SCRIPT_DIR}/.." && pwd)"
EXPECTED_PARENT_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SHA="$(shasum -a 256 "${VERSION_DIR}/parent.asc" | awk '{print $1}')"
CANDIDATE_SHA="$(shasum -a 256 "${VERSION_DIR}/candidate.asc" | awk '{print $1}')"
if [ "${PARENT_SHA}" != "${EXPECTED_PARENT_SHA}" ]; then
    printf 'local parent source does not match formal V011\n' >&2
    exit 68
fi
if [ "${MODE}" = "--paired" ]; then
    QUALIFICATION_LOG="${TINY_PARENT_QUALIFICATION_LOG:-}"
    if [ -z "${QUALIFICATION_LOG}" ] || [ ! -r "${QUALIFICATION_LOG}" ] || \
       ! grep -q '^RUNNER_COMPLETE mode=PARENT_QUALIFY status=PASS cases=3 ' "${QUALIFICATION_LOG}"; then
        printf 'a completed parent qualification log is required before paired timing\n' >&2
        exit 65
    fi
fi
STAMP="$(date '+%Y%m%dT%H%M%S%z')"
LOG="${SCRIPT_DIR}/run-${MODE#--}-${STAMP}.log"
if [ -e "${LOG}" ]; then
    printf 'Refusing to overwrite existing evidence: %s\n' "${LOG}" >&2
    exit 66
fi

{
    printf 'ROUTE=TINY-MINIMAL-KERNEL-CHAMPION-X\nREVISION=V001\nMODE=%s\n' "${MODE}"
    printf 'DEVICE=%s\nMIN_FREE_HBM_MB=%s\n' "${DEVICE}" "${MIN_FREE_HBM_MB}"
    printf 'ADMISSION=RULE=FREE_HBM_GE_%sMB\n' "${MIN_FREE_HBM_MB}"
    printf 'LEASE_METADATA=%s\n' "${MAIN_DEVICE_LEASE:-NONE}"
    printf 'LOAD_METADATA=%s\n' "${TINY_LOAD_WINDOW:-UNSET}"
    printf 'NOTE=lease and load are recorded context only, not admission gates\n'
    if [ "${MODE}" = "--paired" ]; then
        printf 'PARENT_QUALIFICATION_LOG=%s\n' "${QUALIFICATION_LOG}"
    fi
    ssh cann-server3 "bash -s -- '${REMOTE_ROOT}' '${MODE}' '${DEVICE}' '${PARENT_SHA}' '${CANDIDATE_SHA}'" <<'REMOTE'
set -euo pipefail
REMOTE_ROOT="$1"
MODE="$2"
DEVICE="$3"
EXPECTED_PARENT_SHA="$4"
EXPECTED_CANDIDATE_SHA="$5"
PARENT_SOURCE="${REMOTE_ROOT}/src/parent.asc"
CANDIDATE_SOURCE="${REMOTE_ROOT}/src/candidate.asc"
RUNNER="${REMOTE_ROOT}/build/tiny_runner_v001"
actual_parent="$(sha256sum "${PARENT_SOURCE}" | awk '{print $1}')"
actual_candidate="$(sha256sum "${CANDIDATE_SOURCE}" | awk '{print $1}')"
if [ "${actual_parent}" != "${EXPECTED_PARENT_SHA}" ] || \
   [ "${actual_candidate}" != "${EXPECTED_CANDIDATE_SHA}" ]; then
    printf 'REMOTE_SOURCE_IDENTITY=FAIL\n' >&2
    exit 68
fi
printf 'REMOTE_HOST=%s\n' "$(hostname)"
printf 'REMOTE_PARENT_SOURCE_SHA=%s\n' "${actual_parent}"
printf 'REMOTE_CANDIDATE_SOURCE_SHA=%s\n' "${actual_candidate}"
printf 'RUNNER_SHA=%s\n' "$(sha256sum "${RUNNER}" | awk '{print $1}')"
capture_load() {
    printf 'LOAD_SNAPSHOT=%s\n' "$1"
    TZ=Asia/Taipei date '+%Y-%m-%dT%H:%M:%S%z'
    npu-smi info -t usages -i "${DEVICE}"
    printf 'PROCESSES_BEGIN\n'
    ps -eo pid=,comm=,args= | grep -E '[Vv][Ll][Ll][Mm]|EngineCore|Worker_TP|tiny_runner_v001' | grep -v grep | sort -n || true
    printf 'PROCESSES_END\n'
    printf 'LOAD_NOTE=measurement context only, not an admission gate\n'
}
source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh
export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}
capture_load BEFORE_RUN
printf 'RUNNER_COMMAND=%s %s %s\n' "${RUNNER}" "${MODE}" "${DEVICE}"
"${RUNNER}" "${MODE}" "${DEVICE}"
capture_load AFTER_RUN
printf 'REMOTE_RUN=PASS\n'
REMOTE
} 2>&1 | tee "${LOG}"
