#!/usr/bin/env bash
set -euo pipefail
umask 027
R12_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
R12_STAGE="$1"
R12_CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
set +u
source "${R12_CANN}/aarch64-linux/script/set_env.sh"
set -u
export LD_LIBRARY_PATH="/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}"
R12_HCC="${R12_CANN}/toolkit/toolchain/hcc"
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:${R12_HCC}/aarch64-target-linux-gnu/include/c++/7.3.0:${R12_HCC}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:${R12_HCC}/aarch64-target-linux-gnu/include/c++/7.3.0/backward:${CPLUS_INCLUDE_PATH:-}"

context() {
    date -u +%Y-%m-%dT%H:%M:%SZ
    hostname
    npu-smi info -t usages -i 4
    cat /proc/loadavg
    ps -eo pid,comm | awk '$2 ~ /[Vv][Ll][Ll][Mm]|EngineCor/ {print}'
}

context
R12_FREE_MB="$(npu-smi info -t usages -i 4 | awk '/HBM Capacity\(MB\)/ {c=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/ {r=$NF} END {if(c=="") exit 1; printf "%d\n", c*(100-r)/100}')"
printf 'FREE_HBM_MB=%s\nROUTE=W4-R12\nSTAGE=%s\n' "$R12_FREE_MB" "$R12_STAGE"
test "$R12_FREE_MB" -ge 100
mkdir -p "${R12_ROOT}/results"
case "$R12_STAGE" in
    build)
        cmake -S "$R12_ROOT" -B "${R12_ROOT}/build" -DR12_PARENT_DIR="$R12_ROOT"
        cmake --build "${R12_ROOT}/build" --target r12_parent_probe --parallel 1
        printf 'COMPILE=PASS\n'
        ;;
    event)
        printf 'COMMAND=%s/build/r12_parent_probe 4 %s/results/event\n' "$R12_ROOT" "$R12_ROOT"
        "${R12_ROOT}/build/r12_parent_probe" 4 "${R12_ROOT}/results/event"
        ;;
    profile)
        printf 'PROFILE=task-based task-time=on ai-core=off ascendcl=on runtime-api=on one_process=YES\n'
        timeout 120s "${R12_CANN}/tools/profiler/bin/msprof" \
            --output="${R12_ROOT}/results/kernel-task" \
            --application="${R12_ROOT}/build/r12_parent_probe 4 ${R12_ROOT}/results/profile" \
            --task-time=on --aic-mode=task-based --ai-core=off --ascendcl=on --runtime-api=on --aicpu=off
        ;;
    *) exit 2 ;;
esac
context
printf 'STAGE_COMPLETE=%s\nRUNNING_DEVICE_OPERATION=NONE\n' "$R12_STAGE"
