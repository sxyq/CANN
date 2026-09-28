#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
RESULT_DIR="$ROOT/results"
RUNNER_DIR="${RUNNER_DIR:-$ROOT/build}"
DEVICE_ID="${DEVICE_ID:-6}"
mkdir -p "$RESULT_DIR"

export ASCEND_HOME_PATH=${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}
source "${ASCEND_HOME_PATH}/bin/setenv.sh" 2>/dev/null || true
export LD_LIBRARY_PATH="${ASCEND_HOME_PATH}/lib64:${ASCEND_HOME_PATH}/aarch64-linux/lib64:${ASCEND_HOME_PATH}/runtime/lib64:${LD_LIBRARY_PATH:-}"

A1="$RUNNER_DIR/ub_liveness_alias1_probe"
A0="$RUNNER_DIR/ub_liveness_alias0_probe"
if [[ ! -x "$A1" || ! -x "$A0" ]]; then
  echo "missing probe binaries in $RUNNER_DIR" >&2
  exit 1
fi

date -Is > "$RESULT_DIR/load-snapshot.timestamp.txt"
npu-smi info > "$RESULT_DIR/load-snapshot.npu-smi.txt" || true

# Correctness battery (alias1 = candidate)
echo "=== CORRECTNESS alias1 ==="
set +e
"$A1" "$DEVICE_ID" 4 256 0 correct "$RESULT_DIR/correct-f32-d256-r4" 2
c1=$?
"$A1" "$DEVICE_ID" 6 1000 1 correct "$RESULT_DIR/correct-f16-d1000-r3" 3
c2=$?
"$A1" "$DEVICE_ID" 3 777 2 correct "$RESULT_DIR/correct-bf16-d777-r2" 2
c3=$?
"$A1" "$DEVICE_ID" 8 4096 0 correct "$RESULT_DIR/correct-f32-d4096-r2" 2
c4=$?
"$A1" "$DEVICE_ID" 5 64 1 correct "$RESULT_DIR/correct-f16-d64-r4" 4
c5=$?
"$A1" "$DEVICE_ID" 2 32768 1 correct "$RESULT_DIR/correct-f16-d32768-r2" 2
c6=$?
"$A1" "$DEVICE_ID" 4 128 2 correct "$RESULT_DIR/correct-bf16-d128-r3" 3
c7=$?
set -e
echo "correctness_exit_codes: $c1 $c2 $c3 $c4 $c5 $c6 $c7"

# Correctness alias0 (control must also be correct)
echo "=== CORRECTNESS alias0 ==="
set +e
"$A0" "$DEVICE_ID" 4 256 0 correct "$RESULT_DIR/c0-correct-f32-d256" 2
k1=$?
"$A0" "$DEVICE_ID" 6 1000 1 correct "$RESULT_DIR/c0-correct-f16-d1000" 3
k2=$?
set -e
echo "control_correctness_exit_codes: $k1 $k2"

# Paired probes: alternate order, 4 pairs, mid-D FP16 where param+tile matter
SHAPE_ROWS=8
SHAPE_D=4096
SHAPE_DT=1
for pair in 01 02 03 04; do
  date -Is > "$RESULT_DIR/probe-pair-$pair.timestamp.txt"
  npu-smi info > "$RESULT_DIR/probe-pair-$pair.npu-smi.txt" || true
  if [[ "$pair" == "02" || "$pair" == "04" ]]; then
    "$A1" "$DEVICE_ID" "$SHAPE_ROWS" "$SHAPE_D" "$SHAPE_DT" probe "$RESULT_DIR/probe-pair-$pair-alias1" 2
    "$A0" "$DEVICE_ID" "$SHAPE_ROWS" "$SHAPE_D" "$SHAPE_DT" probe "$RESULT_DIR/probe-pair-$pair-alias0" 2
  else
    "$A0" "$DEVICE_ID" "$SHAPE_ROWS" "$SHAPE_D" "$SHAPE_DT" probe "$RESULT_DIR/probe-pair-$pair-alias0" 2
    "$A1" "$DEVICE_ID" "$SHAPE_ROWS" "$SHAPE_D" "$SHAPE_DT" probe "$RESULT_DIR/probe-pair-$pair-alias1" 2
  fi
done

# Secondary shape FP32 D=1024
SHAPE_ROWS=4
SHAPE_D=1024
SHAPE_DT=0
for pair in 05 06; do
  date -Is > "$RESULT_DIR/probe-pair-$pair.timestamp.txt"
  npu-smi info > "$RESULT_DIR/probe-pair-$pair.npu-smi.txt" || true
  if [[ "$pair" == "06" ]]; then
    "$A1" "$DEVICE_ID" "$SHAPE_ROWS" "$SHAPE_D" "$SHAPE_DT" probe "$RESULT_DIR/probe-pair-$pair-alias1" 2
    "$A0" "$DEVICE_ID" "$SHAPE_ROWS" "$SHAPE_D" "$SHAPE_DT" probe "$RESULT_DIR/probe-pair-$pair-alias0" 2
  else
    "$A0" "$DEVICE_ID" "$SHAPE_ROWS" "$SHAPE_D" "$SHAPE_DT" probe "$RESULT_DIR/probe-pair-$pair-alias0" 2
    "$A1" "$DEVICE_ID" "$SHAPE_ROWS" "$SHAPE_D" "$SHAPE_DT" probe "$RESULT_DIR/probe-pair-$pair-alias1" 2
  fi
done

echo "PROBES_DONE"
