#!/usr/bin/env bash
set -eo pipefail
source /usr/local/Ascend/ascend-toolkit/set_env.sh
cd /home/data4t2/lelinfeng/server_runs/W4-R14/param-mte2-fingerprint-20261008
export CPLUS_INCLUDE_PATH="$ASCEND_HOME_PATH/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:$ASCEND_HOME_PATH/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"
r14_free_mb=$(npu-smi info -t usages -i 0 | awk '/HBM Capacity\(MB\)/{cap=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/{used=$NF} END{if(cap=="")exit 1; print int(cap*(100-used)/100)}')
printf 'FREE_HBM_MB=%s\n' "$r14_free_mb"
test "$r14_free_mb" -ge 100
r14_asc_dir=$(sed -n 's/^ASC_DIR:PATH=//p' build-attempt8/CMakeCache.txt)
cmake -S parent-probe -B parent-probe/build -DASC_DIR="$r14_asc_dir" -DR14_SUPPORT_ROOT=/home/data4t2/lelinfeng/server_runs/W4-R14/param-mte2-fingerprint-20261008
cmake --build parent-probe/build --parallel 2
