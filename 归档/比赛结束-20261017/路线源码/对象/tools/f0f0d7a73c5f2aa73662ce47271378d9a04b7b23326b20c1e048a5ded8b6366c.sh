#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD_DIR="${H001_BUILD_DIR:-$ROOT/build}"
TOOLKIT=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
LOG_DIR="$ROOT/logs"
LOG_FILE="$LOG_DIR/compile-04.log"

mkdir -p "$LOG_DIR"
rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

if [ -f /usr/local/Ascend/ascend-toolkit/set_env.sh ]; then
  set +u
  . /usr/local/Ascend/ascend-toolkit/set_env.sh
  set -u
fi

{
  echo "=== H001 V001 build $(date -Iseconds) ==="
  echo "BUILD_DIR=$BUILD_DIR"
  echo "TOOLKIT=$TOOLKIT"
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
  echo "=== submission compile ==="
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
