#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 3 ]]; then
    echo "usage: $0 <parent-runner> <candidate-runner> <device-id>" >&2
    exit 2
fi

parent_runner=$1
candidate_runner=$2
device_id=$3

for round in 0 1 2 3 4; do
    if (( round % 2 == 0 )); then
        order=(parent candidate)
    else
        order=(candidate parent)
    fi
    for side in "${order[@]}"; do
        if [[ $side == parent ]]; then
            runner=$parent_runner
        else
            runner=$candidate_runner
        fi
        echo "BLOCK round=$round side=$side"
        "$runner" --suite benchmark --device "$device_id" --dtype fp16 \
            --rows 128 --width 16384 --warmup 20 --samples 8
    done
done
