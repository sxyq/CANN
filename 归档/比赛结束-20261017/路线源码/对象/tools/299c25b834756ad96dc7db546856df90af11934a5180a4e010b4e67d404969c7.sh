#!/usr/bin/env bash
set -euo pipefail
CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
# shellcheck disable=SC1090
source "$CANN/bin/setenv.bash" >/dev/null 2>&1 || source "$CANN/set_env.sh" >/dev/null 2>&1 || true
export ASCEND_HOME_PATH="$CANN"
export CMAKE_PREFIX_PATH="$CANN/aarch64-linux/tikcpp/ascendc_kernel_cmake:${CMAKE_PREFIX_PATH:-}"
LLD="$CANN/tools/ccec_compiler/bin/ld.lld"
if [ ! -x "$LLD" ]; then LLD="$CANN/aarch64-linux/ccec_compiler/bin/ld.lld"; fi
REMOTE=/home/data4t2/lelinfeng/phase4-workspaces/SCHED-ROWGROUP-X

for variant in parent_build v001_build; do
  echo "===== BUILD $variant ====="
  cd "$REMOTE/$variant"
  rm -rf build
  mkdir -p build
  cmake -S . -B build > build/cfg.log 2>&1
  cmake --build build --target device -j2 > build/device.log 2>&1
  cmake --build build --target submission -j2 > build/submission.log 2>&1
  echo "objects for $variant:"
  find build -name '*.o'
  dev_obj=$(find build -path '*device.dir*' -name '*.o' | head -1)
  sub_obj=$(find build -path '*submission.dir*' -name '*.o' | head -1)
  test -n "$dev_obj"
  test -n "$sub_obj"
  "$LLD" -m aicorelinux -Ttext=0 -static -o build/device.alink "$dev_obj"
  "$LLD" -m aicorelinux -Ttext=0 -static -o build/submission.alink "$sub_obj"
  # mark full_link target success for logs
  {
    echo "FULL_LINK_OK"
    echo "LLD=$LLD"
    echo "DEV=$dev_obj"
    echo "SUB=$sub_obj"
    ls -la build/*.alink
  } > build/link.log
  echo "$variant LINK_OK"
  ls -la build/*.alink
done

echo "===== PROBES ====="
cd "$REMOTE/support"
rm -rf build
mkdir -p build
cmake -S . -B build > build/cfg.log 2>&1 || { echo PROBE_CFG_FAIL; cat build/cfg.log; exit 1; }
cmake --build build -j2 > build/build.log 2>&1 || { echo PROBE_BUILD_FAIL; tail -100 build/build.log; exit 1; }
ls -la build/srx_*
echo PROBES_OK
echo ALL_DONE
