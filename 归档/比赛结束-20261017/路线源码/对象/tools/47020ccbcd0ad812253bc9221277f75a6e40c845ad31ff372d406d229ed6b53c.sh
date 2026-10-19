#!/bin/bash
CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
source "$CANN/bin/setenv.bash" >/dev/null 2>&1
export ASCEND_HOME_PATH="$CANN"
export ASC_DIR="$CANN/aarch64-linux/tikcpp/ascendc_kernel_cmake"
export CMAKE_PREFIX_PATH="$ASC_DIR:${CMAKE_PREFIX_PATH:-}"
export CPLUS_INCLUDE_PATH="$CANN/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:$CANN/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:$CANN/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward:$CANN/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include:${CPLUS_INCLUDE_PATH:-}"
cd /home/data4t2/lelinfeng/phase4-workspaces/SCHED-CHAMPION-X/V001/support/timing
rm -rf build && mkdir -p build
echo "=== CMAKE ==="
cmake -S . -B build -DASC_DIR="$ASC_DIR" 2>&1 | tail -8
echo "CMAKE_RC=$?"
echo "=== BUILD ==="
cmake --build build -j2 2>&1 | tail -15
echo "BUILD_RC=$?"
ls -la build/scx_ref_*_probe 2>/dev/null
echo "=== DONE ==="
