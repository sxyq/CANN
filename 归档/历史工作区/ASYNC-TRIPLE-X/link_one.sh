#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
LLD="${LLD:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/ccec_compiler/bin/ld.lld}"
if [[ ! -x "$LLD" ]]; then
  LLD=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/tools/ccec_compiler/bin/ld.lld
fi
REV="${1:?usage: link_one.sh REV OBJ...}"
shift
OUT="$ROOT/${REV}.alink"
echo "LLD=$LLD OUT=$OUT"
"$LLD" -m aicorelinux -Ttext=0 -static -o "$OUT" "$@"
echo "LINK_OK $OUT"
shasum -a 256 "$OUT"
