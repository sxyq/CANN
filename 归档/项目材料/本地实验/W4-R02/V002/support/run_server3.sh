#!/usr/bin/env bash
set -euo pipefail
R02_SUPPORT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
R02_RUN_DIR="$(cd "${R02_SUPPORT_DIR}/.." && pwd)"
R02_RUNNER="${R02_RUN_DIR}/build/r02_runner"
R02_DEVICE=1
R02_CANN_ROOT="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
export LD_LIBRARY_PATH="${R02_CANN_ROOT}/aarch64-linux/lib64:${R02_CANN_ROOT}/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
mkdir -p "${R02_RUN_DIR}/logs" "${R02_RUN_DIR}/local"

r02_context() {
    local label="$1"
    npu-smi info -t usages -i "${R02_DEVICE}" > "${R02_RUN_DIR}/logs/${label}.usages.txt"
    uptime > "${R02_RUN_DIR}/logs/${label}.load.txt"
    local free_mb
    free_mb="$(awk '/HBM Capacity\(MB\)/ {c=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/ {r=$NF} END {if(c=="") exit 1; printf "%d",c*(100-r)/100}' "${R02_RUN_DIR}/logs/${label}.usages.txt")"
    printf 'STAGE=%s DEVICE=1 FREE_HBM_MB=%s\n' "${label}" "${free_mb}" | tee -a "${R02_RUN_DIR}/logs/stages.txt"
    test "${free_mb}" -ge 100
}

r02_context correctness-pre
npu-smi info > "${R02_RUN_DIR}/logs/correctness-pre.npu.txt"
for width in 18432 20480 22528 24576; do
    for side in parent candidate; do
        output="${R02_RUN_DIR}/logs/correctness-${width}-${side}.tsv"
        if "${R02_RUNNER}" "${R02_DEVICE}" "${width}" fp16 correctness-only "${side}" 0 0 1 "${output}" > "${output}.log" 2>&1; then
            cat "${output}.log"
            printf 'CORRECTNESS width=%s side=%s RC=0\n' "${width}" "${side}" | tee -a "${R02_RUN_DIR}/logs/stages.txt"
        else
            result=$?
            cat "${output}.log"
            printf 'CORRECTNESS width=%s side=%s RC=%s\n' "${width}" "${side}" "${result}" | tee -a "${R02_RUN_DIR}/logs/stages.txt"
            exit "${result}"
        fi
    done
done

r02_context local-pre
for width in 20480 18432 22528; do
    output="${R02_RUN_DIR}/local/same-parent-${width}.tsv"
    "${R02_RUNNER}" "${R02_DEVICE}" "${width}" fp16 same parent 45 21 4 "${output}" > "${output}.log" 2>&1
    printf 'SAME_PARENT width=%s RC=0\n' "${width}" | tee -a "${R02_RUN_DIR}/logs/stages.txt"
    output="${R02_RUN_DIR}/local/paired-${width}.tsv"
    "${R02_RUNNER}" "${R02_DEVICE}" "${width}" fp16 paired - 45 21 4 "${output}" > "${output}.log" 2>&1
    printf 'PAIRED width=%s RC=0\n' "${width}" | tee -a "${R02_RUN_DIR}/logs/stages.txt"
done
r02_context local-post
npu-smi info > "${R02_RUN_DIR}/logs/local-post.npu.txt"
printf 'RUN_COMPLETE RC=0\n' | tee -a "${R02_RUN_DIR}/logs/stages.txt"
