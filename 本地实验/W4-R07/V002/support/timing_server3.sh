#!/usr/bin/env bash
set -eo pipefail

R07_ROOT="${1:?revision directory is required}"
R07_PHASE="${2:?build or profile is required}"
R07_CANN="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
source "${R07_CANN}/aarch64-linux/script/set_env.sh"
set -u
export LD_LIBRARY_PATH="${R07_ROOT}/build:${R07_CANN}/aarch64-linux/lib64:${R07_CANN}/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
R07_RESULT="${R07_ROOT}/results/timing-attribution-20261008"
mkdir -p "${R07_RESULT}"

date -Is
stat -c '%n size=%s mtime=%y' "${R07_ROOT}/build/w4r07_runner" \
    "${R07_ROOT}/build/libw4r07_parent.so" "${R07_ROOT}/build/libw4r07_candidate.so"

if test "${R07_PHASE}" = build; then
    npu-smi info -t usages -i 2 > "${R07_RESULT}/build-usages.txt"
    R07_FREE="$(awk '/HBM Capacity\(MB\)/{c=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/{r=$NF} END{if(c==""||r=="")exit 1; printf "%d",c*(100-r)/100}' "${R07_RESULT}/build-usages.txt")"
    printf 'DEVICE=2 FREE_HBM_MB=%s\n' "${R07_FREE}"
    test "${R07_FREE}" -ge 100
    set -x
    g++ -O3 -DNDEBUG -std=gnu++17 \
        -I"${R07_ROOT}/support" -I"${R07_CANN}/aarch64-linux/include" \
        "${R07_ROOT}/support/runner_main.cpp" \
        "${R07_ROOT}/build/libw4r07_parent.so" "${R07_ROOT}/build/libw4r07_candidate.so" \
        "${R07_CANN}/aarch64-linux/lib64/libascendcl.so" -ldl -lm \
        -Wl,-rpath,"${R07_ROOT}/build:${R07_CANN}/aarch64-linux/lib64:${R07_CANN}/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common" \
        -o "${R07_ROOT}/build/w4r07_timing_host"
    set +x
    ldd "${R07_ROOT}/build/w4r07_timing_host"
elif test "${R07_PHASE}" = profile; then
    npu-smi info -t usages -i 2 > "${R07_RESULT}/pre-usages.txt"
    npu-smi info > "${R07_RESULT}/pre-processes.txt"
    cat /proc/loadavg > "${R07_RESULT}/pre-loadavg.txt"
    R07_FREE="$(awk '/HBM Capacity\(MB\)/{c=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/{r=$NF} END{if(c==""||r=="")exit 1; printf "%d",c*(100-r)/100}' "${R07_RESULT}/pre-usages.txt")"
    printf 'DEVICE=2 FREE_HBM_MB=%s\n' "${R07_FREE}"
    test "${R07_FREE}" -ge 100
    set +e
    set -x
    "${R07_CANN}/tools/profiler/bin/msprof" \
        --application="${R07_ROOT}/build/w4r07_timing_host 2 12288 fp16 attribution - 45 8 8 ${R07_RESULT}/samples.tsv" \
        --output="${R07_RESULT}/profile" --task-time=on --ascendcl=on --runtime-api=on \
        --ai-core=off --aicpu=off
    R07_RC=$?
    set +x
    set -e
    npu-smi info -t usages -i 2 > "${R07_RESULT}/post-usages.txt"
    npu-smi info > "${R07_RESULT}/post-processes.txt"
    cat /proc/loadavg > "${R07_RESULT}/post-loadavg.txt"
    printf 'PROFILE_RC=%s\n' "${R07_RC}"
    test "${R07_RC}" -eq 0
else
    printf 'Unsupported phase: %s\n' "${R07_PHASE}" >&2
    exit 2
fi

stat -c '%n size=%s mtime=%y' "${R07_ROOT}/build/w4r07_runner" \
    "${R07_ROOT}/build/libw4r07_parent.so" "${R07_ROOT}/build/libw4r07_candidate.so"
date -Is
