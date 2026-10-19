#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")" && pwd)"
# Prefer workspace submission (local layout may nest support under workspaces or local record)
if [[ -f "$ROOT/../../../../workspaces/UB-LIVENESS-X/submission.asc" ]]; then
  WS="$ROOT/../../../../workspaces/UB-LIVENESS-X"
elif [[ -f "$ROOT/../../../workspaces/UB-LIVENESS-X/submission.asc" ]]; then
  WS="$ROOT/../../../workspaces/UB-LIVENESS-X"
elif [[ -f "$ROOT/../../workspaces/UB-LIVENESS-X/submission.asc" ]]; then
  WS="$ROOT/../../workspaces/UB-LIVENESS-X"
elif [[ -f "$ROOT/../../../UB-LIVENESS-X/submission.asc" ]]; then
  WS="$ROOT/../../../UB-LIVENESS-X"
elif [[ -f "$ROOT/submission.asc" ]]; then
  WS="$ROOT"
else
  # server3 layout: support lives under workspace
  if [[ -f "$ROOT/../submission.asc" ]]; then
    WS="$(cd "$ROOT/.." && pwd)"
  else
    echo "cannot find submission.asc near $ROOT" >&2
    exit 1
  fi
fi
RESULT_DIR="$ROOT/results"
mkdir -p "$RESULT_DIR"
echo "WS=$WS"

awk 'BEGIN{done=0}
/#ifndef UB_LIVENESS_ALIAS/{if(!done){print "#ifndef UB_LIVENESS_ALIAS"; print "#define UB_LIVENESS_ALIAS 1"; done=1; next}}
/#define UB_LIVENESS_ALIAS/{if(done){next}}
{print}
END{if(!done) exit 1}' "$WS/submission.asc" > "$ROOT/submission_alias1.asc"
awk 'BEGIN{done=0}
/#ifndef UB_LIVENESS_ALIAS/{if(!done){print "#ifndef UB_LIVENESS_ALIAS"; print "#define UB_LIVENESS_ALIAS 0"; done=1; next}}
/#define UB_LIVENESS_ALIAS/{if(done){next}}
{print}
END{if(!done) exit 1}' "$WS/submission.asc" > "$ROOT/submission_alias0.asc"
grep -n 'define UB_LIVENESS_ALIAS' "$ROOT/submission_alias1.asc" "$ROOT/submission_alias0.asc"

export ASCEND_HOME_PATH=${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}
source "${ASCEND_HOME_PATH}/bin/setenv.sh" 2>/dev/null || true
KIT="${ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
export CMAKE_PREFIX_PATH="${KIT}:${CMAKE_PREFIX_PATH:-}"
HCC="${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
export CPLUS_INCLUDE_PATH="${HCC}:${HCC}/aarch64-target-linux-gnu:${HCC}/backward:${CPLUS_INCLUDE_PATH:-}"

BUILD="$ROOT/build"
mkdir -p "$BUILD"
cd "$BUILD"
cmake .. \
  -DCMAKE_MODULE_PATH="${KIT}/ASC_CMake;${KIT}" \
  -DASC_DIR="${KIT}" \
  -DCMAKE_PREFIX_PATH="${KIT}" \
  -DCMAKE_CXX_FLAGS="-I${HCC} -I${HCC}/aarch64-target-linux-gnu -I${HCC}/backward" \
  -DSOC_VERSION=Ascend910B3 \
  -DNPU_ARCH=dav-2201 2>&1 | tee "$RESULT_DIR/configure.log"
cmake --build . -j"$(nproc)" 2>&1 | tee "$RESULT_DIR/build.log"
echo "BUILD_OK"
