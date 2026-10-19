#!/usr/bin/env bash
set -eo pipefail

action="${1:?build or profile}"
phase="${2:-$action}"
route_root=/home/data4t2/lelinfeng/w4-r08/V001
result_dir="$route_root/results/timing-attribution-20261008"
sdk=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
cd "$route_root"
source "$sdk/aarch64-linux/script/set_env.sh"
set -u
export ASCEND_OPP_PATH="$sdk/opp"
export LD_LIBRARY_PATH="$sdk/lib64:$sdk/aarch64-linux/lib64:/usr/lib/aarch64-linux-gnu"
mkdir -p "$result_dir"

snapshot() {
    date -u +%Y-%m-%dT%H:%M:%SZ
    hostname
    id -un
    npu-smi info -t usages -i 2
    uptime
}

snapshot | tee "$result_dir/$phase-pre.txt"
free_mb=$(awk '/HBM Capacity\(MB\)/ {c=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/ {r=$NF} END {if(c=="") exit 1; printf "%d", c*(100-r)/100}' "$result_dir/$phase-pre.txt")
printf 'DEVICE=2 FREE_HBM_MB=%s CPU_AFFINITY=72\n' "$free_mb"
if [ "$free_mb" -lt 100 ]; then exit 3; fi

if [ "$action" = build ]; then
    old_object=build/CMakeFiles/w4r08_paired_runner.dir/paired_runner.asc.o
    test -f "$old_object"
    test -x build/w4r08_paired_runner
    if [ ! -f "$result_dir/original-before.txt" ]; then
        stat -c '%n %s %y' build/w4r08_paired_runner "$old_object" > "$result_dir/original-before.txt"
    fi
    compiler="$sdk/compiler/ccec_compiler/bin/bisheng"
    set -x
    /usr/bin/g++ -x c++ -DW4R08_HOST_ONLY -std=c++17 -O2 -fPIC \
        -I"$sdk/aarch64-linux/include" \
        -c src/paired_runner.asc -o build/w4r08_timing_host.o
    "$compiler" build/w4r08_timing_host.o "$old_object" -Wl,--wrap=main \
        -o build/w4r08_timing_host -L/usr/lib/gcc/aarch64-linux-gnu/11 \
        -L"$sdk/lib64" -L"$sdk/aarch64-linux/lib64" \
        -Wl,-rpath,/usr/lib/gcc/aarch64-linux-gnu/11 \
        -Wl,-rpath,"$sdk/lib64" -Wl,-rpath,"$sdk/aarch64-linux/lib64" \
        -lascendcl -ltiling_api -lregister -lplatform -lunified_dlog -ldl -lm -lgraph_base
    set +x
    stat -c '%n %s %y' build/w4r08_paired_runner "$old_object" > "$result_dir/original-after-build.txt"
    cmp "$result_dir/original-before.txt" "$result_dir/original-after-build.txt"
    printf 'COMPILE=PASS KERNEL_COMPILE=NOT_EXECUTED ORIGINAL_SIZE_MTIME=UNCHANGED\n'
elif [ "$action" = profile ]; then
    test -x build/w4r08_timing_host
    test ! -e "$result_dir/pp.raw.tsv"
    test ! -e "$result_dir/pp-$phase"
    preflight_utc=$(date -u +%Y-%m-%dT%H:%M:%SZ)
    application="/usr/bin/taskset -c 72 $route_root/build/w4r08_timing_host --mode paired-pp --device 2 --rows 17 --width 257 --dtype 0 --owner W4-R08 --lease-id W4-R08-V001-timing-20261008 --preflight-utc $preflight_utc --warmup 45 --samples 31 --pairs 4 --out-prefix $result_dir/pp"
    printf '%s\n' "$application" | tee "$result_dir/pp-$phase-command.txt"
    "$sdk/tools/profiler/bin/msprof" --application="$application" \
        --output="$result_dir/pp-$phase" --ai-core=off --aic-mode=task-based \
        --task-time=on --ascendcl=on --runtime-api=on --aicpu=off \
        2>&1 | tee "$result_dir/pp-$phase.log"
    printf 'PROFILE=COMPLETE\n'
else
    printf 'Unsupported action: %s\n' "$action" >&2
    exit 2
fi

snapshot | tee "$result_dir/$phase-post.txt"
date -u +%Y-%m-%dT%H:%M:%SZ
