#!/usr/bin/env bash
set -euo pipefail
umask 027
R10_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
R10_STAGE="$1"
R10_CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
set +u
source "${R10_CANN}/aarch64-linux/script/set_env.sh"
set -u
export ASCEND_HOME_PATH="$R10_CANN"
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"
export LD_LIBRARY_PATH="/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}"

context() {
    date -u +%Y-%m-%dT%H:%M:%SZ
    hostname
    npu-smi info -t usages -i 4
    cat /proc/loadavg
    ps -eo pid,comm | awk '$2 ~ /[Vv][Ll][Ll][Mm]|EngineCor/ {print}'
}
context
R10_FREE_MB="$(npu-smi info -t usages -i 4 | awk '/HBM Capacity\(MB\)/ {c=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/ {r=$NF} END {if(c=="" || r=="") exit 1; printf "%d\n", c*(100-r)/100}')"
printf 'ROUTE=W4-R10\nREVISION=V002\nSTAGE=%s\nFREE_HBM_MB=%s\n' "$R10_STAGE" "$R10_FREE_MB"
test "$R10_FREE_MB" -ge 100
mkdir -p "${R10_ROOT}/results"
case "$R10_STAGE" in
    build)
        cmake -S "$R10_ROOT" -B "${R10_ROOT}/build" -DCMAKE_BUILD_TYPE=Release
        cmake --build "${R10_ROOT}/build" --parallel 1
        printf 'COMPILE=PASS\n'
        ;;
    correctness)
        R10_RESULT=0
        for spec in '128 12288 2 target' '48 12288 2 single-batch' '127 12288 2 indivisible' '120 12288 2 partial-batch' '128 8192 2 short-d' '1 32768 2 resident-one' '12 8192 0 fp32' '2 12288 1 fp16'; do
            read -r rows width dtype name <<< "$spec"
            printf 'COMMAND=r10_probe correctness 4 %s %s %s %s\n' "$rows" "$width" "$dtype" "$name"
            "${R10_ROOT}/build/r10_probe" correctness 4 "$rows" "$width" "$dtype" "${R10_ROOT}/results/correctness-${name}" || R10_RESULT=$?
        done
        printf 'CORRECTNESS_RC=%s\n' "$R10_RESULT"
        test "$R10_RESULT" -eq 0
        ;;
    profile-target|profile-control)
        if [[ "$R10_STAGE" == profile-target ]]; then R10_ROWS=128; else R10_ROWS=48; fi
        printf 'COMMAND=msprof task-time only; r10_probe measure 4 %s 12288 2\n' "$R10_ROWS"
        "${R10_CANN}/tools/profiler/bin/msprof" \
            --output="${R10_ROOT}/results/${R10_STAGE}-capture" \
            --application="${R10_ROOT}/build/r10_probe measure 4 ${R10_ROWS} 12288 2 ${R10_ROOT}/results/${R10_STAGE}" \
            --task-time=on --aic-mode=task-based --ai-core=off --ascendcl=on --runtime-api=on --aicpu=off
        ;;
    *) exit 2 ;;
esac
context
printf 'STAGE_COMPLETE=%s\nRUNNING_DEVICE_OPERATION=NONE\n' "$R10_STAGE"
