#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD_DIR="${H001_BUILD_DIR:-$ROOT/build}"
TOOLKIT=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
LOG_DIR="$ROOT/logs"
LOG_FILE="$LOG_DIR/compile-05.log"

mkdir -p "$LOG_DIR"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

if [ -f /usr/local/Ascend/ascend-toolkit/set_env.sh ]; then
  set +u
  . /usr/local/Ascend/ascend-toolkit/set_env.sh
  set -u
fi

SRC="$ROOT/src/add_rms_norm_bias_kernel.cpp"

{
  echo "=== H001 V002 build $(date -Iseconds) ==="
  echo "BUILD_DIR=$BUILD_DIR"
  echo "TOOLKIT=$TOOLKIT"
  echo
  echo "=== template preflight (npu_kernel_dev) ==="
  FIRST=$(head -n 1 "$SRC")
  LAST=$(tail -n 1 "$SRC")
  echo "first_line=$FIRST"
  echo "last_line=$LAST"
  echo "has_run_kernel=$(grep -c 'extern \"C\" void run_kernel' "$SRC" || true)"
  echo "has_vector_entry=$(grep -c '__global__ __vector__' "$SRC" || true)"
  echo "redefines_tensorinfo=$(grep -c 'struct TensorInfo' "$SRC" || true)"
  echo "has_type_traits=$(grep -c 'type_traits' "$SRC" || true)"
  echo "has_std_is_same=$(grep -c 'std::is_same' "$SRC" || true)"
  echo "has_brace_pad_init=$(grep -c 'DataCopyPadExtParams<.*>.*{' "$SRC" || true)"
  echo
  echo "=== cmake configure ==="
  cmake -S "$ROOT" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Release \
    -DCANN_ROOT="$TOOLKIT" \
    -DSOC_VERSION=Ascend910B3
  echo
  echo "=== device compile ==="
  cmake --build "$BUILD_DIR" --target h001_device -- -j2
  echo "DEVICE_COMPILE=PASS"
  echo
  echo "=== submission compile (mock judge.asc include) ==="
  cmake --build "$BUILD_DIR" --target h001_submission -- -j2
  echo "SUBMISSION_COMPILE=PASS"
  echo
  echo "=== full link ==="
  cmake --build "$BUILD_DIR" --target h001_link -- -j2
  echo "FULL_LINK=PASS"
  echo
  echo "=== artifacts ==="
  ls -la "$BUILD_DIR"/*.o "$BUILD_DIR"/*.so
  echo
  echo "ALL_STAGES=PASS"
} 2>&1 | tee "$LOG_FILE"
