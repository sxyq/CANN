#!/usr/bin/env bash
set -euo pipefail
CANN="${CANN:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
LLD="${CANN}/tools/ccec_compiler/bin/ld.lld"
if [ ! -x "$LLD" ]; then
  LLD="${CANN}/aarch64-linux/ccec_compiler/bin/ld.lld"
fi
BUILD="$1"
OUTDIR="$2"
mkdir -p "$OUTDIR"
dev_obj=$(find "$BUILD/CMakeFiles/device.dir" -name '*.o' | head -1)
sub_obj=$(find "$BUILD/CMakeFiles/submission.dir" -name '*.o' | head -1)
if [ -z "$dev_obj" ] || [ -z "$sub_obj" ]; then
  echo "MISSING_OBJECTS dev='$dev_obj' sub='$sub_obj'" >&2
  find "$BUILD" -name '*.o' >&2 || true
  exit 2
fi
echo "LLD=$LLD"
echo "DEV_OBJ=$dev_obj"
echo "SUB_OBJ=$sub_obj"
"$LLD" -m aicorelinux -Ttext=0 -static -o "$OUTDIR/device.alink" "$dev_obj"
"$LLD" -m aicorelinux -Ttext=0 -static -o "$OUTDIR/submission.alink" "$sub_obj"
ls -la "$OUTDIR"/*.alink
echo FULL_LINK_OK
