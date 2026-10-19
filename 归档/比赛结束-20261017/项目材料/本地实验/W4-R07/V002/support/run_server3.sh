#!/usr/bin/env bash
set -eo pipefail

R07_ROOT="${1:?revision directory is required}"
R07_PHASE="${2:?correctness or local is required}"
R07_ATTEMPT="${3:?attempt label is required}"
R07_CANN="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
source "${R07_CANN}/aarch64-linux/script/set_env.sh"
set -u
export LD_LIBRARY_PATH="${R07_CANN}/aarch64-linux/lib64:${R07_CANN}/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
R07_LOG="${R07_ROOT}/logs/${R07_ATTEMPT}"
R07_RUNNER="${R07_ROOT}/build/w4r07_runner"

date -Is
npu-smi info -t usages -i 0 > "${R07_LOG}-pre-usages.txt"
npu-smi info > "${R07_LOG}-pre-processes.txt"
cat /proc/loadavg > "${R07_LOG}-pre-loadavg.txt"
R07_FREE="$(awk '/HBM Capacity\(MB\)/{c=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/{r=$NF} END{if(c==""||r=="")exit 1; printf "%d",c*(100-r)/100}' "${R07_LOG}-pre-usages.txt")"
printf 'DEVICE=0 FREE_HBM_MB=%s PHASE=%s\n' "${R07_FREE}" "${R07_PHASE}"
if test "${R07_FREE}" -lt 100; then
    printf 'RESOURCE_STATUS=FREE_HBM_BELOW_100_MB\n'
    exit 10
fi

R07_RC=0
if test "${R07_PHASE}" = correctness; then
    for R07_WIDTH in 12288 12304 8192; do
        for R07_SIDE in parent candidate; do
            printf 'COMMAND=%s 0 %s fp16 correctness-only %s 0 0 1 %s\n' \
                "${R07_RUNNER}" "${R07_WIDTH}" "${R07_SIDE}" "${R07_LOG}-${R07_WIDTH}-${R07_SIDE}.tsv"
            if "${R07_RUNNER}" 0 "${R07_WIDTH}" fp16 correctness-only "${R07_SIDE}" 0 0 1 \
                "${R07_LOG}-${R07_WIDTH}-${R07_SIDE}.tsv"; then
                printf 'CASE_RC=0 WIDTH=%s SIDE=%s\n' "${R07_WIDTH}" "${R07_SIDE}"
            else
                R07_CASE_RC=$?
                printf 'CASE_RC=%s WIDTH=%s SIDE=%s\n' "${R07_CASE_RC}" "${R07_WIDTH}" "${R07_SIDE}"
                R07_RC=1
            fi
        done
    done
elif test "${R07_PHASE}" = local; then
    for R07_WIDTH in 12288 8192; do
        printf 'COMMAND=%s 0 %s fp16 same parent 45 31 2 %s\n' \
            "${R07_RUNNER}" "${R07_WIDTH}" "${R07_LOG}-${R07_WIDTH}-same.tsv"
        "${R07_RUNNER}" 0 "${R07_WIDTH}" fp16 same parent 45 31 2 \
            "${R07_LOG}-${R07_WIDTH}-same.tsv"
    done
    for R07_WIDTH in 12288 8192; do
        printf 'COMMAND=%s 0 %s fp16 paired - 45 31 4 %s\n' \
            "${R07_RUNNER}" "${R07_WIDTH}" "${R07_LOG}-${R07_WIDTH}-paired.tsv"
        "${R07_RUNNER}" 0 "${R07_WIDTH}" fp16 paired - 45 31 4 \
            "${R07_LOG}-${R07_WIDTH}-paired.tsv"
    done
else
    printf 'Unsupported phase: %s\n' "${R07_PHASE}" >&2
    exit 2
fi

npu-smi info -t usages -i 0 > "${R07_LOG}-post-usages.txt"
npu-smi info > "${R07_LOG}-post-processes.txt"
cat /proc/loadavg > "${R07_LOG}-post-loadavg.txt"
printf 'PHASE_RC=%s\n' "${R07_RC}"
date -Is
exit "${R07_RC}"
