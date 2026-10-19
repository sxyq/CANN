#!/usr/bin/env bash

set +e

source /home/data4t2/lelinfeng/cann_game/.toolchains/cann-9.0/cann-9.0.0/set_env.sh
cd /home/data4t2/lelinfeng/cann/实验/V005/工程 || exit 2

echo "=== V005 PROBE SUITE START ==="
date -u '+%Y-%m-%dT%H:%M:%SZ'
echo "DEVICE=${PROBE_DEVICE:-5}"
npu-smi info | sed -n '1,180p'

run_probe() {
    local width="$1"
    local rows="$2"
    local rank="$3"
    local dtype="$4"
    local seed="$5"

    echo "--- PROBE width=${width} rows=${rows} rank=${rank} dtype=${dtype} seed=${seed} ---"
    PROBE_D="$width" PROBE_ROWS="$rows" PROBE_DTYPE="$dtype" PROBE_SEED="$seed" \
        python3 gen_probe.py
    local data_status=$?
    echo "DATA_STATUS=${data_status}"
    if [ "$data_status" -ne 0 ]; then
        return "$data_status"
    fi

    PROBE_D="$width" PROBE_ROWS="$rows" PROBE_RANK="$rank" PROBE_DTYPE="$dtype" \
        timeout 30 ./add_rms_norm_bias_custom
    local run_status=$?
    echo "RUN_STATUS=${run_status}"
    return "$run_status"
}

run_suite() {
    run_probe 64 1 2 fp16 101 || return $?
    run_probe 96 1 2 fp16 102 || return $?
    run_probe 67 1 2 fp16 103 || return $?
    run_probe 129 1 2 fp16 104 || return $?
    run_probe 192 1 2 fp16 105 || return $?
    run_probe 576 1 2 fp16 106 || return $?
    run_probe 1000 1 2 fp16 107 || return $?
    run_probe 1024 1 2 fp16 108 || return $?
    run_probe 2048 1 2 fp16 109 || return $?
    run_probe 4096 1 2 fp16 110 || return $?
    run_probe 8192 1 2 fp16 111 || return $?
    run_probe 32768 1 2 fp16 112 || return $?
    run_probe 64 2 2 fp16 113 || return $?
    run_probe 1000 2 2 fp16 114 || return $?
    run_probe 4096 8 3 fp16 115 || return $?
    run_probe 8192 8 4 fp16 116 || return $?
}

run_suite
suite_status=$?
echo "SUITE_STATUS=${suite_status}"

echo "=== V005 PROBE SUITE END ==="
date -u '+%Y-%m-%dT%H:%M:%SZ'
df -h /home/data4t2/lelinfeng/cann
du -sh /home/data4t2/lelinfeng/cann/实验/V005/* 2>/dev/null
exit "$suite_status"
