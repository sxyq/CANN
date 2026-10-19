#!/usr/bin/env bash
set -euo pipefail
umask 027
R05_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
R05_STAGE="$1"
R05_CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
set +u
source "${R05_CANN}/aarch64-linux/script/set_env.sh"
set -u
export LD_LIBRARY_PATH="/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}"
R05_HCC="${R05_CANN}/toolkit/toolchain/hcc"
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:${R05_HCC}/aarch64-target-linux-gnu/include/c++/7.3.0:${R05_HCC}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:${R05_HCC}/aarch64-target-linux-gnu/include/c++/7.3.0/backward:${CPLUS_INCLUDE_PATH:-}"
context() {
    date -u +%Y-%m-%dT%H:%M:%SZ
    hostname
    npu-smi info -t usages -i 2
    cat /proc/loadavg
    ps -eo pid,comm | awk '$2 ~ /[Vv][Ll][Ll][Mm]|EngineCor/ {print}'
}
context
R05_FREE_MB="$(npu-smi info -t usages -i 2 | awk '/HBM Capacity\(MB\)/ {c=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/ {r=$NF} END {if(c=="" || r=="") exit 1; printf "%d\n", c*(100-r)/100}')"
printf 'ROUTE=W4-R05\nDEVICE=2\nFREE_HBM_MB=%s\nSTAGE=%s\n' "$R05_FREE_MB" "$R05_STAGE"
test "$R05_FREE_MB" -ge 100
mkdir -p "${R05_ROOT}/results"
case "$R05_STAGE" in
    address)
        cmake -S "$R05_ROOT" -B "$R05_ROOT/build" -DR05_PARENT_DIR="$R05_ROOT/parent"
        cmake --build "$R05_ROOT/build" --target r05_address_probe --parallel 1
        "$R05_ROOT/build/r05_address_probe"
        ;;
    build-parent)
        cmake -S "$R05_ROOT" -B "$R05_ROOT/build" -DR05_PARENT_DIR="$R05_ROOT/parent"
        cmake --build "$R05_ROOT/build" --target r05_parent r05_probe --parallel 1
        printf 'COMPILE_PARENT=PASS\n'
        ;;
    build-candidate)
        cmake -S "$R05_ROOT" -B "$R05_ROOT/build" -DR05_PARENT_DIR="$R05_ROOT/parent"
        cmake --build "$R05_ROOT/build" --target r05_candidate --parallel 1
        printf 'COMPILE_CANDIDATE=PASS\n'
        ;;
    correctness)
        "$R05_ROOT/build/r05_probe" correctness 2 "$R05_ROOT/results/correctness"
        ;;
    parent|local)
        "${R05_CANN}/tools/profiler/bin/msprof" --ai-core=off --aic-mode=task-based --task-time=on \
            --ascendcl=on --runtime-api=on --aicpu=off \
            --output="$R05_ROOT/results/$R05_STAGE-profile" \
            --application="$R05_ROOT/build/r05_probe $R05_STAGE 2 $R05_ROOT/results/$R05_STAGE"
        ;;
    *) exit 2 ;;
esac
context
printf 'STAGE_COMPLETE=%s\nRUNNING_DEVICE_OPERATION=NONE\n' "$R05_STAGE"
