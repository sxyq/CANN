#!/usr/bin/env bash
set -euo pipefail

R09_STAGE="${1:?compile, correctness, same, or local required}"
shift
R09_REVISION_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
R09_CANN_ROOT=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
R09_DEVICE=3
export ASCEND_HOME_PATH="${R09_CANN_ROOT}"
export ASCEND_CANN_PACKAGE_PATH="${R09_CANN_ROOT}"
export PATH="${R09_CANN_ROOT}/compiler/ccec_compiler/bin:${R09_CANN_ROOT}/aarch64-linux/bin:${PATH}"
export LD_LIBRARY_PATH="${R09_CANN_ROOT}/aarch64-linux/lib64:${R09_CANN_ROOT}/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"
export C_INCLUDE_PATH="/usr/include/aarch64-linux-gnu${C_INCLUDE_PATH:+:${C_INCLUDE_PATH}}"
export LIBRARY_PATH="/usr/lib/gcc/aarch64-linux-gnu/11:/usr/lib/aarch64-linux-gnu:/lib/aarch64-linux-gnu${LIBRARY_PATH:+:${LIBRARY_PATH}}"

record_context() {
    date -u +%FT%TZ
    R09_USAGE="$(npu-smi info -t usages -i "${R09_DEVICE}")"
    printf '%s\n' "${R09_USAGE}"
    R09_FREE_MB="$(awk '/HBM Capacity\(MB\)/ {c=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/ {r=$NF} END {if(c=="" || r=="") exit 1; printf "%d\n",c*(100-r)/100}' <<<"${R09_USAGE}")"
    printf 'DEVICE_ID=%s FREE_HBM_MB=%s\n' "${R09_DEVICE}" "${R09_FREE_MB}"
    cat /proc/loadavg
    npu-smi info -t proc-mem -i "${R09_DEVICE}" || true
}

record_context
if (( R09_FREE_MB < 100 )); then
    printf 'RESOURCE_RESULT=INSUFFICIENT_FREE_HBM\n'
    exit 20
fi
printf 'ROUTE=W4-R09 REVISION=V001 STAGE=%s CANN=%s\n' "${R09_STAGE}" "${R09_CANN_ROOT}"

if [[ "${R09_STAGE}" == compile ]]; then
    bisheng --version
    cmake -S "${R09_REVISION_DIR}/support" -B "${R09_REVISION_DIR}/build" \
        -DCMAKE_BUILD_TYPE=Release -DASCEND_HOME_PATH="${R09_CANN_ROOT}"
    cmake --build "${R09_REVISION_DIR}/build" --parallel 2
else
    printf 'RUNNER_ARGUMENTS='
    printf '%q ' --mode "${R09_STAGE}" --device "${R09_DEVICE}" "$@"
    printf '\n'
    if "${R09_REVISION_DIR}/build/w4r09_v001_paired_runner" \
        --mode "${R09_STAGE}" --device "${R09_DEVICE}" "$@"; then
        R09_RC=0
    else
        R09_RC=$?
    fi
    record_context
    printf 'RUNNER_EXIT=%s\n' "${R09_RC}"
    exit "${R09_RC}"
fi
