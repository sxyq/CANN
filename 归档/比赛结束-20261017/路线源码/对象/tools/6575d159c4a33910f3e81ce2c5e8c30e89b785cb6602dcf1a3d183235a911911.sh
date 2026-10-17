#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
RESULT_DIR="$ROOT/results"
RUNNER_DIR="${RUNNER_DIR:-/home/data4t2/lelinfeng/phase4-workspaces/MIX-A/runner-v007}"
DEVICE_ID="${DEVICE_ID:-6}"
mkdir -p "$RESULT_DIR"

for pair in 01 02 03 04; do
    date -Is > "$RESULT_DIR/probe-pair-$pair.timestamp.txt"
    npu-smi info > "$RESULT_DIR/probe-pair-$pair.npu-smi.txt"
    if [[ "$pair" == 02 || "$pair" == 04 ]]; then
        "$RUNNER_DIR/build/mix_a_v007_probe" "$DEVICE_ID" 1 256 0 "$RESULT_DIR/probe-pair-$pair-v007"
        "$RUNNER_DIR/build/mix_a_v003_probe" "$DEVICE_ID" 1 256 0 "$RESULT_DIR/probe-pair-$pair-v003"
    else
        "$RUNNER_DIR/build/mix_a_v003_probe" "$DEVICE_ID" 1 256 0 "$RESULT_DIR/probe-pair-$pair-v003"
        "$RUNNER_DIR/build/mix_a_v007_probe" "$DEVICE_ID" 1 256 0 "$RESULT_DIR/probe-pair-$pair-v007"
    fi
done
