#!/usr/bin/env bash
set -eo pipefail

route_root=/home/data4t2/lelinfeng/cann/w4/W4-R01/V001
case "${2:-parent-only}" in
    parent-only)
        study_root=$route_root/task-time-device0-20261008
        profile_prefix=pp
        run_mode=task-study
        comparison=parent-parent
        repeats=42
        raw_prefix=pp
        ;;
    paired)
        study_root=$route_root/task-compare-device0-20261008
        profile_prefix=comparison
        run_mode=task-compare
        comparison=parent-candidate
        repeats=32
        raw_prefix=pcstudy
        ;;
    *) exit 2 ;;
esac
cd "$route_root"
umask 027
mkdir -p "$study_root"
source /usr/local/Ascend/ascend-toolkit/set_env.sh

context() {
    local prefix=$1
    date -u +%Y-%m-%dT%H:%M:%SZ > "$study_root/$prefix.context.txt"
    hostname >> "$study_root/$prefix.context.txt"
    cat /proc/loadavg >> "$study_root/$prefix.context.txt"
    npu-smi info -t usages -i 0 > "$study_root/$prefix.usages.txt"
    npu-smi info > "$study_root/$prefix.npu.txt"
    local free_mb
    free_mb=$(awk '/HBM Capacity\(MB\)/ {c=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/ {u=$NF}
        END {if (c == "" || u == "") exit 1; printf "%d", c*(100-u)/100}' "$study_root/$prefix.usages.txt")
    printf 'FREE_HBM_MB=%s\n' "$free_mb" >> "$study_root/$prefix.context.txt"
    test "$free_mb" -ge 100
}

case "${1:-}" in
    compile)
        context compile-pre
        stat -c '%y %s %n' support/build/libw4r01_v001_parent.so support/build/libw4r01_v001_candidate.so \
            > "$study_root/libraries-before.txt"
        /usr/bin/c++ --version > "$study_root/compiler.txt"
        command=(/usr/bin/c++ -O2 -std=c++17
            -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/include -Isupport
            support/runner_main.cpp -Lsupport/build -lw4r01_v001_parent -lw4r01_v001_candidate
            -L/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64 -lascendcl -ldl -lm
            '-Wl,-rpath,$ORIGIN:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common'
            -o support/build/w4r01_v001_paired_runner)
        printf '%q ' "${command[@]}" > "$study_root/compile-command.txt"
        printf '\n' >> "$study_root/compile-command.txt"
        date -u +%Y-%m-%dT%H:%M:%SZ > "$study_root/compile.log"
        set +e
        "${command[@]}" >> "$study_root/compile.log" 2>&1
        status=$?
        set -e
        printf 'RETURN_CODE=%d\n' "$status" >> "$study_root/compile.log"
        date -u +%Y-%m-%dT%H:%M:%SZ >> "$study_root/compile.log"
        cat "$study_root/compile.log"
        exit "$status"
        ;;
    collect)
        if [ -e "$study_root/$profile_prefix-profile" ]; then
            printf 'Existing capture retained; refusing another collection at this path.\n' >&2
            exit 2
        fi
        context "$profile_prefix-pre"
        application="$route_root/support/build/w4r01_v001_paired_runner --mode $run_mode --comparison $comparison --device 0 --rows 16 --width 16384 --blocks 8 --dtype fp16 --warmups 45 --repeats $repeats --output $study_root/$raw_prefix"
        command=(msprof --ai-core=off --aic-mode=task-based --task-time=on --ascendcl=on
            --runtime-api=on --aicpu=off "--output=$study_root/$profile_prefix-profile" "--application=$application")
        printf '%q ' "${command[@]}" > "$study_root/collect-command.txt"
        printf '\n' >> "$study_root/collect-command.txt"
        date -u +%Y-%m-%dT%H:%M:%SZ > "$study_root/collect-session.log"
        set +e
        "${command[@]}" > "$study_root/$profile_prefix-profile.log" 2>&1
        status=$?
        set -e
        printf 'RETURN_CODE=%d\n' "$status" >> "$study_root/collect-session.log"
        date -u +%Y-%m-%dT%H:%M:%SZ >> "$study_root/collect-session.log"
        context "$profile_prefix-post"
        stat -c '%y %s %n' support/build/libw4r01_v001_parent.so support/build/libw4r01_v001_candidate.so \
            > "$study_root/libraries-after.txt"
        cat "$study_root/collect-session.log"
        exit "$status"
        ;;
    *)
        printf 'Usage: bash support/collect_task_time.sh compile|collect [parent-only|paired]\n' >&2
        exit 2
        ;;
esac
