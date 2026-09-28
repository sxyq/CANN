#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
DEVICE_ID="${DEVICE_ID:-0}"
BUILD_DIR="$ROOT/build"
RESULT_DIR="$ROOT/artifacts"
mkdir -p "$RESULT_DIR"
cleanup() {
    rm -f "$RESULT_DIR/v003-output.bin" "$RESULT_DIR/mode-output.bin"
}
trap cleanup EXIT
rm -f "$RESULT_DIR/v003.tsv" "$RESULT_DIR/mode.tsv" "$RESULT_DIR/pair-guard.log"

source /usr/local/Ascend/ascend-toolkit/set_env.sh
export ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
export ASC_DIR="$ASCEND_HOME_PATH/aarch64-linux/tikcpp/ascendc_kernel_cmake"
export CMAKE_PREFIX_PATH="$ASC_DIR${CMAKE_PREFIX_PATH:+:$CMAKE_PREFIX_PATH}"
HCC_ROOT="$ASCEND_HOME_PATH/toolkit/toolchain/hcc"
export CPLUS_INCLUDE_PATH="$HCC_ROOT/aarch64-target-linux-gnu/include/c++/7.3.0:$HCC_ROOT/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:$HCC_ROOT/aarch64-target-linux-gnu/include/c++/7.3.0/backward:${CPLUS_INCLUDE_PATH:-}"
export C_INCLUDE_PATH="$HCC_ROOT/aarch64-target-linux-gnu/include:${C_INCLUDE_PATH:-}"
cmake -S "$ROOT" -B "$BUILD_DIR" -DCMAKE_BUILD_TYPE=Release -DSOC_VERSION=Ascend910B3 \
    2>&1 | tee "$RESULT_DIR/build.log"
cmake --build "$BUILD_DIR" --target mix_a_v003_runner mix_a_mode_runner -j2 \
    2>&1 | tee -a "$RESULT_DIR/build.log"

"$BUILD_DIR/mix_a_v003_runner" "$DEVICE_ID" "$RESULT_DIR/v003"
"$BUILD_DIR/mix_a_mode_runner" "$DEVICE_ID" "$RESULT_DIR/mode"

python3 "$ROOT/validate_pair.py" "$RESULT_DIR/v003-output.bin" "$RESULT_DIR/mode-output.bin" \
    | tee "$RESULT_DIR/pair-guard.log"
printf 'PAIR_ARTIFACTS=%s\n' "$RESULT_DIR"
