#!/usr/bin/env bash
set -eo pipefail
umask 027

R14_ROOT=/home/data4t2/lelinfeng/server_runs/W4-R14/param-mte2-fingerprint-20261008/parent-probe
R14_OUTPUT="$R14_ROOT/task-time-20261008"
cd "$R14_ROOT"
source /usr/local/Ascend/ascend-toolkit/set_env.sh

# Reuse the existing executable; only the profiler wrapper is new.
# Per shape: one reference call, 45 warmups, two blocks of 21 P/P pairs.
# Capture 80x8192 then 120x6144 once each, without sample removal or replay.
mkdir -m 750 "$R14_OUTPUT"
{
    date -u +%Y-%m-%dT%H:%M:%SZ
    hostname
    id -un
    printf 'ROUTE=W4-R14\nDEVICE=3\nONLINE=PAUSED\nPUSH=NO\n'
    printf 'RUNNER=%s/build/r14_parent_probe\nASCEND_HOME_PATH=%s\n' "$R14_ROOT" "$ASCEND_HOME_PATH"
    stat -c '%n %s bytes %y' build/r14_parent_probe parent_probe.asc
    npu-smi info -t board -i 3
    df -h .
} > "$R14_OUTPUT/environment.txt"

for R14_CASE in '80 8192' '120 6144'; do
    read -r R14_ROWS R14_WIDTH <<< "$R14_CASE"
    R14_CASE_DIR="$R14_OUTPUT/${R14_ROWS}x${R14_WIDTH}_fp32"
    mkdir -m 750 "$R14_CASE_DIR"
    date -u +%Y-%m-%dT%H:%M:%SZ > "$R14_CASE_DIR/pre.time.txt"
    npu-smi info -t usages -i 3 > "$R14_CASE_DIR/pre.usages.txt"
    npu-smi info -t proc-mem -i 3 > "$R14_CASE_DIR/pre.processes.txt"
    cat /proc/loadavg > "$R14_CASE_DIR/pre.load.txt"
    R14_FREE_MB=$(awk '/HBM Capacity\(MB\)/{c=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/{u=$NF} END{if(c=="" || u=="")exit 1; print int(c*(100-u)/100)}' "$R14_CASE_DIR/pre.usages.txt")
    printf 'DEVICE=3 FREE_HBM_MB=%s SHAPE=%sx%s FP32\n' "$R14_FREE_MB" "$R14_ROWS" "$R14_WIDTH" | tee "$R14_CASE_DIR/resource.txt"
    test "$R14_FREE_MB" -ge 100
    R14_APP="$R14_ROOT/build/r14_parent_probe 3 $R14_ROWS $R14_WIDTH 0 45 21 2"
    printf 'msprof --output=%s/capture --application="%s" --task-time=on --aic-mode=task-based --ai-core=off --ascendcl=on --runtime-api=on --aicpu=off\n' "$R14_CASE_DIR" "$R14_APP" > "$R14_CASE_DIR/command.txt"
    set +e
    msprof --output="$R14_CASE_DIR/capture" --application="$R14_APP" \
        --task-time=on --aic-mode=task-based --ai-core=off \
        --ascendcl=on --runtime-api=on --aicpu=off \
        > "$R14_CASE_DIR/profile.log" 2>&1
    R14_RC=$?
    set -e
    printf 'PROFILE_RC=%s\n' "$R14_RC" | tee "$R14_CASE_DIR/exit.txt"
    date -u +%Y-%m-%dT%H:%M:%SZ > "$R14_CASE_DIR/post.time.txt"
    npu-smi info -t usages -i 3 > "$R14_CASE_DIR/post.usages.txt"
    npu-smi info -t proc-mem -i 3 > "$R14_CASE_DIR/post.processes.txt"
    cat /proc/loadavg > "$R14_CASE_DIR/post.load.txt"
    cat "$R14_CASE_DIR/profile.log"
    test "$R14_RC" -eq 0
done

{
    date -u +%Y-%m-%dT%H:%M:%SZ
    npu-smi info -t usages -i 3
    cat /proc/loadavg
    set +e
    pgrep -af '[r]14_parent_probe|[r]14_param_mte2_fingerprint'
    R14_PROCESS_RC=$?
    set -e
    printf 'RUNNER_PROCESS_QUERY_RC=%s\n' "$R14_PROCESS_RC"
    test "$R14_PROCESS_RC" -eq 1
    printf 'RUNNING_DEVICE_OPERATION=NONE\n'
} > "$R14_OUTPUT/final-state.txt"
