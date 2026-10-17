#!/usr/bin/env bash
set -uo pipefail
export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
export PYTHONPATH="${PYTHONPATH:-}"
set +u
source /usr/local/Ascend/ascend-toolkit/set_env.sh >/dev/null 2>&1 || true
set -u
export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
PROBE=/home/data4t2/lelinfeng/phase4-workspaces/ALIGN-TAIL-X/probe
RES=$PROBE/results
mkdir -p "$RES"
date -Is > "$RES/correctness-start.timestamp.txt"
npu-smi info > "$RES/npu-smi-pre.txt" || true
FAIL=0
for shape in "1 64" "1 67" "1 100" "2 129" "2 256" "1 257" "4 100" "1 4100"; do
  set -- $shape
  rows=$1; width=$2
  for dtype in 0 1 2; do
    tag="r${rows}_d${width}_t${dtype}"
    if ! "$PROBE/build/atx_parent_probe" 6 "$rows" "$width" "$dtype" "$RES/parent_$tag" >"$RES/parent_$tag.log" 2>&1; then
      echo "PARENT_FAIL $tag"; FAIL=1; cat "$RES/parent_$tag.log"
    fi
    if ! "$PROBE/build/atx_v001_probe" 6 "$rows" "$width" "$dtype" "$RES/v001_$tag" >"$RES/v001_$tag.log" 2>&1; then
      echo "V001_FAIL $tag"; FAIL=1; cat "$RES/v001_$tag.log"
    fi
    python3 -c "
import struct,sys
def load(p):
    with open(p,'rb') as f: b=f.read()
    return list(struct.unpack('<%df'%(len(b)//4), b))
a=load('$RES/parent_${tag}-output.bin')
b=load('$RES/v001_${tag}-output.bin')
assert len(a)==len(b), (len(a),len(b))
m=max(abs(x-y) for x,y in zip(a,b))
print('PAIR $tag n=%d pair_max_abs=%.9g'%(len(a),m))
" || { echo "PAIR_FAIL $tag"; FAIL=1; }
    echo "---- $tag ----"
    echo -n "parent: "; awk 'NR==2{printf "med=%s bad=%s max_abs=%s\n",$7,$9,$8}' "$RES/parent_$tag.tsv"
    echo -n "v001:   "; awk 'NR==2{printf "med=%s bad=%s max_abs=%s\n",$7,$9,$8}' "$RES/v001_$tag.tsv"
  done
done
date -Is > "$RES/correctness-end.timestamp.txt"
echo "CORRECTNESS_LOOP_DONE FAIL=$FAIL"
exit $FAIL
