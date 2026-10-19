#!/bin/sh
set -eu

ROOT=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD_DIR="$ROOT/build"
TOOLKIT=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"
if [ -f /usr/local/Ascend/ascend-toolkit/set_env.sh ]; then
  # The toolkit environment selects the same compiler family used by server3.
  set +u
  . /usr/local/Ascend/ascend-toolkit/set_env.sh
  set -u
fi
cmake -S "$ROOT" -B "$BUILD_DIR" \
  -DCMAKE_BUILD_TYPE=Release \
  -DCANN_ROOT="$TOOLKIT" \
  -DSOC_VERSION=Ascend910B3
cmake --build "$BUILD_DIR" --target h001_kernel -- -j2
