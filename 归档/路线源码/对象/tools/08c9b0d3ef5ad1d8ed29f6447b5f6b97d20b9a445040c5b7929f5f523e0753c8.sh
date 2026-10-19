set -uo pipefail
CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
source "$CANN/bin/setenv.bash" >/dev/null 2>&1 || source "$CANN/set_env.sh" >/dev/null 2>&1 || true
export ASCEND_HOME_PATH="$CANN"
export CMAKE_PREFIX_PATH="$CANN/aarch64-linux/tikcpp/ascendc_kernel_cmake:${CMAKE_PREFIX_PATH:-}"
LLD="$CANN/tools/ccec_compiler/bin/ld.lld"
[ -x "$LLD" ] || LLD="$CANN/aarch64-linux/ccec_compiler/bin/ld.lld"
WORKDIR=/home/data4t2/lelinfeng/phase4-workspaces/ASYNC-OVERLAP-CHAMPION-X/V001
cd "$WORKDIR" || exit 1
rm -rf build
mkdir -p build
echo "===== CONFIGURE ====="
cmake -S . -B build > build/cfg.log 2>&1
CFG_RC=$?
echo "CONFIGURE_RC=$CFG_RC"
if [ $CFG_RC -ne 0 ]; then tail -30 build/cfg.log; exit 1; fi
echo "===== COMPILE device ====="
cmake --build build --target device -j2 > build/device.log 2>&1
DEV_RC=$?
echo "COMPILE_DEVICE_RC=$DEV_RC"
echo "===== COMPILE submission ====="
cmake --build build --target submission -j2 > build/submission.log 2>&1
SUB_RC=$?
echo "COMPILE_SUBMISSION_RC=$SUB_RC"
echo "===== LINK ====="
dev_obj=$(find build -path '*device.dir*' -name '*.o' | head -1)
sub_obj=$(find build -path '*submission.dir*' -name '*.o' | head -1)
echo "DEV_OBJ=$dev_obj"
echo "SUB_OBJ=$sub_obj"
LINK_DEV_RC=2
LINK_SUB_RC=2
if [ -n "$dev_obj" ] && [ -n "$sub_obj" ]; then
  "$LLD" -m aicorelinux -Ttext=0 -static -o build/device.alink "$dev_obj" > build/link-dev.log 2>&1
  LINK_DEV_RC=$?
  "$LLD" -m aicorelinux -Ttext=0 -static -o build/submission.alink "$sub_obj" > build/link-sub.log 2>&1
  LINK_SUB_RC=$?
fi
echo "LINK_DEV_RC=$LINK_DEV_RC"
echo "LINK_SUB_RC=$LINK_SUB_RC"
echo "===== FULL_LINK ====="
cmake --build build --target full_link -j2 > build/full_link.log 2>&1
FULL_RC=$?
echo "FULL_LINK_RC=$FULL_RC"
echo "===== ARTIFACTS ====="
ls -la build/*.alink build/full_link 2>/dev/null
echo "===== EXECUTABLE SHA ====="
sha256sum build/device.alink build/submission.alink build/full_link 2>/dev/null
if [ $DEV_RC -ne 0 ]; then echo "--- device.log tail ---"; tail -40 build/device.log; fi
if [ $SUB_RC -ne 0 ]; then echo "--- submission.log tail ---"; tail -40 build/submission.log; fi
if [ $FULL_RC -ne 0 ]; then echo "--- full_link.log tail ---"; tail -40 build/full_link.log; fi
echo BUILD_SCRIPT_DONE
