#!/usr/bin/env bash
set -uo pipefail
W4_R13_PROBE_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
W4_R13_EVIDENCE="${W4_R13_PROBE_ROOT}/evidence/param-reuse-01"
W4_R13_TOOLKIT=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
export LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6
export LD_LIBRARY_PATH="${W4_R13_TOOLKIT}/aarch64-linux/lib64:${W4_R13_TOOLKIT}/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
mkdir -p "${W4_R13_EVIDENCE}"

snapshot() {
    date -Is
    npu-smi info -t usages -i 1
    cat /proc/loadavg
}

run_case() {
    local variant="$1" role="$2" width="$3"
    local prefix="${W4_R13_EVIDENCE}/${role}-1x${width}"
    if [[ -e "${prefix}-run.log" ]]; then
        printf 'Existing evidence retained: %s\n' "${prefix}-run.log" >&2
        return 2
    fi
    snapshot > "${prefix}-pre.txt"
    local free_mb
    free_mb="$(awk '/HBM Capacity\(MB\)/ {c=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/ {r=$NF} END {if (c == "" || r == "") exit 1; printf "%d", c*(100-r)/100}' "${prefix}-pre.txt")" || return 2
    printf 'FREE_HBM_MB=%s\n' "${free_mb}" >> "${prefix}-pre.txt"
    if (( free_mb < 100 )); then
        printf 'DEVICE_1_FREE_HBM_BELOW_100_MB\n' >&2
        return 2
    fi
    local binary="${W4_R13_PROBE_ROOT}/build/w4r13_${variant}_probe"
    local rc=0
    {
        date -Is
        printf 'COMMAND=%q 1 1 %s 0 %q 0 1 1 0 1\n' "${binary}" "${width}" "${prefix}"
        "${binary}" 1 1 "${width}" 0 "${prefix}" 0 1 1 0 1
        rc=$?
        printf 'EXIT_CODE=%s\n' "${rc}"
        date -Is
    } > "${prefix}-run.log" 2>&1
    snapshot > "${prefix}-post.txt"
    cat "${prefix}-run.log"
    if (( rc != 0 && rc != 3 )); then return "${rc}"; fi
}

run_case ref_parent parent-current 9216 || exit $?
run_case ref_reuse diag 9216 || exit $?
run_case ref_parent parent-current 10240 || exit $?
run_case ref_reuse diag 10240 || exit $?
run_case ref_reuse diag 8192 || exit $?
printf 'ALL_FIVE_DIAGNOSTIC_RUNS_FINISHED\n'
