#!/bin/bash
# SEQ-FUSE-2 DIV_FEASIBILITY_PROBE build (probe only).
set -uo pipefail
export ASCEND_HOME_PATH=${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}
export PATH="${ASCEND_HOME_PATH}/bin:${ASCEND_HOME_PATH}/aarch64-linux/ccec_compiler/bin:${PATH}"
set +u; source "${ASCEND_HOME_PATH}/bin/setenv.bash" 2>/dev/null || true; set -u
ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="${ROOT}/build-divprobe"
HCC="${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
KIT="${ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
export CPLUS_INCLUDE_PATH="${HCC}:${HCC}/aarch64-target-linux-gnu:${HCC}/backward:${CPLUS_INCLUDE_PATH:-}"
export C_INCLUDE_PATH="${HCC}:${HCC}/aarch64-target-linux-gnu:${C_INCLUDE_PATH:-}"
export CMAKE_PREFIX_PATH="${KIT}:${CMAKE_PREFIX_PATH:-}"
export ASC_DIR="${KIT}"
{
  echo "=== div_probe build $(date -u +%Y-%m-%dT%H:%M:%SZ) ==="
  cmake -S "${ROOT}" -B "${BUILD}" \
    -DCMAKE_MODULE_PATH="${KIT}/ASC_CMake;${KIT}" \
    -DASC_DIR="${KIT}" -DCMAKE_PREFIX_PATH="${KIT}" \
    -DCMAKE_CXX_FLAGS="-I${HCC} -I${HCC}/aarch64-target-linux-gnu -I${HCC}/backward" \
    -DCMAKE_C_FLAGS="-I${HCC} -I${HCC}/aarch64-target-linux-gnu" \
    -DSOC_VERSION=Ascend910B3 -DNPU_ARCH=dav-2201
  echo "CMAKE_RC=$?"
  cmake --build "${BUILD}" --target vmx_div_probe -j"$(nproc)"
  echo "BUILD_RC=$?"
  if [ -f "${BUILD}/vmx_div_probe" ]; then
    echo "PROBE_SHA=$(sha256sum "${BUILD}/vmx_div_probe" | awk '{print $1}')"
  else
    echo "PROBE_SHA=MISSING"
  fi
} 2>&1 | tee "${ROOT}/divprobe-build.log"
