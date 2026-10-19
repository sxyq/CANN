#!/bin/bash
CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
source "$CANN/bin/setenv.bash" >/dev/null 2>&1
export ASCEND_HOME_PATH="$CANN"
export ASC_DIR="$CANN/aarch64-linux/tikcpp/ascendc_kernel_cmake"
export CMAKE_PREFIX_PATH="$ASC_DIR:${CMAKE_PREFIX_PATH:-}"
export CPLUS_INCLUDE_PATH="$CANN/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:$CANN/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:$CANN/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward:$CANN/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include:${CPLUS_INCLUDE_PATH:-}"
cd /home/data4t2/lelinfeng/phase4-workspaces/SCHED-CHAMPION-X/parent_probe
rm -rf build && mkdir -p build
cmake -S . -B build -DASC_DIR="$ASC_DIR" > build/cfg.log 2>&1
echo "CMAKE_RC=$?"
cmake --build build -j2 > build/build.log 2>&1
echo "BUILD_RC=$?"
ls -la build/scx_v001_probe 2>/dev/null
export LD_LIBRARY_PATH="$CANN/lib64:$CANN/runtime/lib64:$CANN/compiler/lib64:/usr/lib/aarch64-linux-gnu:$LD_LIBRARY_PATH"
echo "=== RUN ==="
./build/scx_v001_probe 4 2>&1
echo "CORR_RC=$?"
