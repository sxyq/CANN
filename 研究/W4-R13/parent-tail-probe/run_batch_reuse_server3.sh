#!/usr/bin/env bash
set -uo pipefail
W4_R13_PROBE_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
W4_R13_EVIDENCE="${W4_R13_PROBE_ROOT}/evidence/batch-reuse-01"
W4_R13_TOOLKIT=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
export LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6
export LD_LIBRARY_PATH="${W4_R13_TOOLKIT}/aarch64-linux/lib64:${W4_R13_TOOLKIT}/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
mkdir -p "${W4_R13_EVIDENCE}"

snapshot() {
    date -Is
    npu-smi info -t usages -i 3
    npu-smi info -t proc-mem -i 3
    cat /proc/loadavg
}

variant="$1"
role="$2"
prefix="${W4_R13_EVIDENCE}/${role}-121x9216"
binary="${W4_R13_PROBE_ROOT}/build/w4r13_${variant}_probe"
if [[ -e "${prefix}-run.log" ]]; then
    printf 'Existing evidence retained: %s\n' "${prefix}-run.log" >&2
    exit 2
fi
snapshot > "${prefix}-pre.txt"
free_mb="$(awk '/HBM Capacity\(MB\)/ {c=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/ {r=$NF} END {if (c == "" || r == "") exit 1; printf "%d", c*(100-r)/100}' "${prefix}-pre.txt")" || exit 2
printf 'FREE_HBM_MB=%s\n' "${free_mb}" >> "${prefix}-pre.txt"
if (( free_mb < 100 )); then
    printf 'DEVICE_3_FREE_HBM_BELOW_100_MB\n' >&2
    exit 2
fi
rc=0
{
    date -Is
    printf 'COMMAND=%q 3 121 9216 0 %q 0 1 1 0 1\n' "${binary}" "${prefix}"
    "${binary}" 3 121 9216 0 "${prefix}" 0 1 1 0 1
    rc=$?
    printf 'EXIT_CODE=%s\n' "${rc}"
    date -Is
} > "${prefix}-run.log" 2>&1
snapshot > "${prefix}-post.txt"
cat "${prefix}-run.log"
if (( rc != 0 && rc != 3 )); then exit "${rc}"; fi
printf 'DIAGNOSTIC_RUN_FINISHED role=%s runner_exit=%s\n' "${role}" "${rc}"
