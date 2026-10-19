#!/usr/bin/env bash
set -eo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "$ROOT"

TOOLKIT="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
set +u
if [[ -f "${TOOLKIT}/set_env.sh" ]]; then
  # shellcheck disable=SC1090
  source "${TOOLKIT}/set_env.sh"
elif [[ -f /usr/local/Ascend/ascend-toolkit/set_env.sh ]]; then
  source /usr/local/Ascend/ascend-toolkit/set_env.sh
fi
set -u
export ASCEND_CANN_PACKAGE_PATH="${ASCEND_HOME_PATH:-$TOOLKIT}"

REV="${1:-v001}"
BUILD="$ROOT/build-${REV}"
mkdir -p "$BUILD"

{
  echo "=== configure ${REV} $(date -u +%Y-%m-%dT%H:%M:%SZ) ==="
  cmake -S "$ROOT" -B "$BUILD" -DCMAKE_BUILD_TYPE=Release
} >"$ROOT/configure-${REV}-server3.log" 2>&1

{
  echo "=== build ${REV} $(date -u +%Y-%m-%dT%H:%M:%SZ) ==="
  if [[ "$REV" == "seed" ]]; then
    cmake --build "$BUILD" --target async_triple_x_seed_submission async_triple_x_seed_device -j4
  else
    cmake --build "$BUILD" --target async_triple_x_v001_submission async_triple_x_v001_device -j4
  fi
} >"$ROOT/compile-${REV}-server3.log" 2>&1

echo "BUILD_OK ${REV}"
find "$BUILD" -name '*.o' | head -20
