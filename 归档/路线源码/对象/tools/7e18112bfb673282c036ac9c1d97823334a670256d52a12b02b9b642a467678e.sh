#!/usr/bin/env bash
set -euo pipefail
umask 027
R10_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
R10_CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
R10_OUTPUT="${R10_ROOT}/results/timing-attribution-20261008"
set +u
source "${R10_CANN}/aarch64-linux/script/set_env.sh"
set -u
export ASCEND_HOME_PATH="$R10_CANN"
export LD_LIBRARY_PATH="/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}"
mkdir -p "$R10_OUTPUT"

context() {
    date -u +%Y-%m-%dT%H:%M:%SZ
    hostname
    npu-smi info -t usages -i 4
    cat /proc/loadavg
    stat -c 'ORIGINAL_ELF=%n size=%s mtime=%y inode=%i' \
        "${R10_ROOT}/build/r10_probe" "${R10_ROOT}/build/libr10_parent.so" "${R10_ROOT}/build/libr10_candidate.so"
}

resource() {
    local r10_free
    r10_free="$(npu-smi info -t usages -i 4 | awk '/HBM Capacity\(MB\)/ {c=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/ {r=$NF} END {if(c=="" || r=="") exit 1; printf "%d\n", c*(100-r)/100}')"
    printf 'FREE_HBM_MB=%s\n' "$r10_free"
    test "$r10_free" -ge 100
}

context
resource
case "$1" in
    build)
        test ! -e "${R10_ROOT}/build/r10_probe_timing"
        /usr/bin/c++ -std=gnu++17 \
            -I"${R10_CANN}/aarch64-linux/include" -I"${R10_CANN}/include" \
            -I"${R10_CANN}/compiler/tikcpp/tikcfw" \
            "${R10_ROOT}/support/probe.cpp" -o "${R10_ROOT}/build/r10_probe_timing" \
            -L"${R10_CANN}/aarch64-linux/lib64" -L"${R10_CANN}/lib64" \
            -Wl,-rpath,"${R10_CANN}/aarch64-linux/lib64:${R10_CANN}/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common:${R10_ROOT}/build" \
            "${R10_ROOT}/build/libr10_parent.so" "${R10_ROOT}/build/libr10_candidate.so" \
            -lascendcl -ldl -lm -lascendc_runtime -lascendcl -lruntime -lregister \
            -lerror_manager -lprofapi -lge_common_base -lascendalog -lmmpa -ldl -lascend_dump -lc_sec
        readelf -d "${R10_ROOT}/build/r10_probe_timing" | awk '/NEEDED|RUNPATH/ {print}'
        printf 'HOST_COMPILE=PASS\nKERNEL_REBUILT=NO\n'
        ;;
    capture)
        for r10_shape in '128 target' '48 control'; do
            read -r r10_rows r10_name <<< "$r10_shape"
            resource
            test ! -e "${R10_OUTPUT}/${r10_name}-capture"
            printf 'COMMAND=r10_probe_timing diagnose 4 %s 12288 2 %s/%s\n' "$r10_rows" "$R10_OUTPUT" "$r10_name"
            "${R10_CANN}/tools/profiler/bin/msprof" \
                --output="${R10_OUTPUT}/${r10_name}-capture" \
                --application="${R10_ROOT}/build/r10_probe_timing diagnose 4 ${r10_rows} 12288 2 ${R10_OUTPUT}/${r10_name}" \
                --task-time=on --aic-mode=task-based --ai-core=off --ascendcl=on --runtime-api=on --aicpu=off
            context
        done
        ;;
    *) exit 2 ;;
esac
context
printf 'STAGE_COMPLETE=%s\nRUNNING_DEVICE_OPERATION=NONE\n' "$1"
