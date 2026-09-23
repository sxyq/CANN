#!/usr/bin/env bash
set -e
LLD="${LLD:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/tools/ccec_compiler/bin/ld.lld}"
if [ ! -x "${LLD}" ]; then
  LLD=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/ccec_compiler/bin/ld.lld
fi
out="$1"
shift
echo "LLD=${LLD} OUT=${out}"
"${LLD}" -m aicorelinux -Ttext=0 -static -o "${out}" "$@"
