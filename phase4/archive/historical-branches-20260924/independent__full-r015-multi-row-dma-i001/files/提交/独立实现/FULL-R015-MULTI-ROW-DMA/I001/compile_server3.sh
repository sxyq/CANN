#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
REMOTE_ROOT="/home/data4t2/lelinfeng/cann/提交/独立实现/FULL-R015-MULTI-ROW-DMA/I001"
BUILD_DIR="$REMOTE_ROOT/build"
LOG_DIR="$REMOTE_ROOT/日志"
mkdir -p "$BUILD_DIR" "$LOG_DIR" "$REMOTE_ROOT/op_kernel" "$REMOTE_ROOT/op_host"
copy_if_needed() {
  if [[ "$1" != "$2" ]]; then
    cp "$1" "$2"
  fi
}
copy_if_needed "$ROOT/op_kernel/kernel.txt" "$REMOTE_ROOT/op_kernel/kernel.asc"
copy_if_needed "$ROOT/op_kernel/tiling.h" "$REMOTE_ROOT/op_kernel/tiling.h"
copy_if_needed "$ROOT/op_host/main.txt" "$REMOTE_ROOT/op_host/main.asc"
copy_if_needed "$ROOT/CMakeLists.txt" "$REMOTE_ROOT/CMakeLists.txt"
{
  date
  hostname
  df -h /home/data4t2/lelinfeng/cann
  set +u
  source /home/data4t2/lelinfeng/cann_game/.toolchains/cann-9.0/cann-9.0.0/set_env.sh
  set -u
  command -v bisheng
  command -v ccec
  command -v msopgen
  command -v npu-smi
  npu-smi info | sed -n '1,20p'
  cmake -S "$REMOTE_ROOT" -B "$BUILD_DIR"
  cmake --build "$BUILD_DIR" -j2
} > "$LOG_DIR/compile.log" 2>&1
