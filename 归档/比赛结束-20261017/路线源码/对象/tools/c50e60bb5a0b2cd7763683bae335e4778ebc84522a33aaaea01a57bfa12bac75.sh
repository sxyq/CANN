#!/usr/bin/env bash
set -euo pipefail

SUPPORT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
V001_DIR="$(cd "${SUPPORT_DIR}/.." && pwd)"
PARENT_SOURCE="${SUPPORT_DIR}/../../../../线上结果/R31B/V011/submission.asc"
CANDIDATE_SOURCE="${V001_DIR}/submission.asc"
PARENT_SHA="a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3"
CANDIDATE_SHA="9a28f5cd8703dc4ff5c46fba09f59a537132594e5a879a50a17349086bfe4d59"
RUN_ID="${EPI_RUN_ID:-}"
DEVICE_ID="${DEVICE_ID:-7}"
OUTPUT_ROOT="${EPI_SERVER_OUTPUT_ROOT:-/home/data4t2/lelinfeng/cann/server_runs/EPI-ARITH-CHAMPION-W2-X/V001}"
SERVER_ROOT="/home/data4t2/lelinfeng/cann"

[[ "${RUN_ID}" =~ ^[0-9]{8}T[0-9]{6}Z$ ]] || {
    echo "EPI_RUN_ID must use UTC format YYYYMMDDTHHMMSSZ" >&2
    exit 2
}
[[ "${DEVICE_ID}" == "7" ]] || {
    echo "This assigned correctness harness is pinned to device 7" >&2
    exit 2
}
RUN_DIR="${OUTPUT_ROOT}/${RUN_ID}"
BUILD_DIR="${RUN_DIR}/build"
RESULT_DIR="${RUN_DIR}/correctness"
LOG_DIR="${RUN_DIR}/logs"
RUNNER="${BUILD_DIR}/epi_arith_v001_correctness"
test -d "${LOG_DIR}"
MATRIX_LOG="${LOG_DIR}/correctness-matrix.log"
exec > >(tee "${MATRIX_LOG}") 2>&1

record_server_snapshot() {
    local phase="$1"
    printf '\n=== %s ===\n' "${phase}"
    date -u '+SNAPSHOT_UTC=%Y-%m-%dT%H:%M:%SZ'
    hostname
    echo "ASSIGNED_DEVICE=${DEVICE_ID}"
    npu-smi info
    ps -eo pid,ppid,user,stat,comm
    df -h "${SERVER_ROOT}"
    du -sh "${SERVER_ROOT}"/*
}

check_disk_space() {
    local available_kb
    available_kb="$(df -Pk "${SERVER_ROOT}" | awk 'NR == 2 {print $4}')"
    [[ "${available_kb}" =~ ^[0-9]+$ ]]
    printf 'AVAILABLE_DISK_KB=%s REQUIRED_DISK_KB=15728640\n' "${available_kb}"
    (( available_kb >= 15728640 ))
}

finish_correctness() {
    local rc=$?
    trap - EXIT
    echo "CORRECTNESS_FINISHED=$(date -u '+%Y-%m-%dT%H:%M:%SZ')"
    record_server_snapshot CORRECTNESS_POST || true
    exit "${rc}"
}
trap finish_correctness EXIT

date -u '+CORRECTNESS_STARTED=%Y-%m-%dT%H:%M:%SZ'
echo "ROUTE=EPI-ARITH-CHAMPION-W2-X REVISION=V001 RUN_ID=${RUN_ID} DEVICE=${DEVICE_ID}"
record_server_snapshot CORRECTNESS_PRE
if ! check_disk_space; then
    echo "CORRECTNESS=INCOMPLETE reason=AVAILABLE_DISK_BELOW_15_GIB"
    exit 2
fi

PARENT_ACTUAL_SHA="$(sha256sum "${PARENT_SOURCE}" | awk '{print $1}')"
CANDIDATE_ACTUAL_SHA="$(sha256sum "${CANDIDATE_SOURCE}" | awk '{print $1}')"
echo "PARENT_SOURCE_SHA256=${PARENT_ACTUAL_SHA}"
echo "CANDIDATE_SOURCE_SHA256=${CANDIDATE_ACTUAL_SHA}"
[[ "${PARENT_ACTUAL_SHA}" == "${PARENT_SHA}" ]]
[[ "${CANDIDATE_ACTUAL_SHA}" == "${CANDIDATE_SHA}" ]]
test -x "${RUNNER}"
mkdir "${RESULT_DIR}"
printf 'side\tdevice\trows\twidth\tdtype\tblock_count\tsource_derived_batch_rows\trunner_rc\tstatus\tresult\tcommand_log\n' > "${RESULT_DIR}/summary.tsv"

failed=0
for width in 8192 8193 12288 16384 18416 18417 32768; do
    for side in parent candidate; do
        result="${RESULT_DIR}/${side}-r2-d${width}-fp32.tsv"
        case_log="${LOG_DIR}/correctness-${side}-r2-d${width}-fp32.log"
        printf 'COMMAND: %q %q %q %q %q\n' "${RUNNER}" "${DEVICE_ID}" "${side}" "${width}" "${result}"
        if "${RUNNER}" "${DEVICE_ID}" "${side}" "${width}" "${result}" >"${case_log}" 2>&1; then
            rc=0
        else
            rc=$?
            failed=1
        fi
        status="MISSING"
        batch_rows="NA"
        if [[ -f "${result}" ]]; then
            status="$(tail -n 1 "${result}" | cut -f15)"
            batch_rows="$(tail -n 1 "${result}" | cut -f8)"
        fi
        printf '%s\t%s\t2\t%s\tFP32\t1\t%s\t%s\t%s\t%s\t%s\n' \
            "${side}" "${DEVICE_ID}" "${width}" "${batch_rows}" "${rc}" "${status}" \
            "${result}" "${case_log}" | tee -a "${RESULT_DIR}/summary.tsv"
    done
done

printf 'TIMING=NOT_RUN\nMATRIX_STATUS=%s\n' "$([[ ${failed} -eq 0 ]] && echo PASS || echo FAIL)"
exit "${failed}"
