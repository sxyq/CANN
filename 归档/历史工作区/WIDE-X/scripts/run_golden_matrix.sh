#!/bin/bash
set -eo pipefail
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
export PATH="${PATH:-/usr/bin:/bin}"
export PYTHONPATH="${PYTHONPATH:-}"
source /usr/local/Ascend/ascend-toolkit/set_env.sh
cd /home/data4t2/lelinfeng/phase4-workspaces/WIDE-X/build
mkdir -p input output
rc=0
for spec in "1 64 1" "1 64 0" "1 64 2" "3 64 1" "3 64 0" "3 64 2" "4 80 1" "2 256 0" "2 256 2" "2 511 1" "2 512 1" "6 2048 1" "8 1024 0" "4 1024 2" "2 4096 1" "1 32768 1" "1 32768 0"; do
  set -- $spec
  echo "=== r=$1 d=$2 dt=$3 ==="
  python3 ../scripts/gen_verify_multi.py gen "$1" "$2" "$3"
  rm -f output/output.bin
  timeout 30 ./wide_x_full_link "$1" "$2" "$3"
  python3 ../scripts/gen_verify_multi.py check "$1" "$2" "$3" || rc=1
done
echo EXIT:$rc
exit $rc
