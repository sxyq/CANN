#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD="${ROOT}/build"
ASCEND_HOME_PATH="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
export ASCEND_HOME_PATH
export PATH="${ASCEND_HOME_PATH}/compiler/ccec_compiler/bin:${PATH}"
export CPLUS_INCLUDE_PATH="${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward:${CPLUS_INCLUDE_PATH:-}"

mkdir -p "${BUILD}"
{
    echo "UTC=$(date -u +%Y-%m-%dT%H:%M:%SZ)"
    echo "DEVICE=${ASCEND_DEVICE_ID:-3}"
    echo "SOURCE_SHA256=$(sha256sum "${ROOT}/../submission.asc" | awk '{print $1}')"
    echo "COMPILER=$(command -v bisheng)"
    cmake -S "${ROOT}" -B "${BUILD}" \
        -DCMAKE_MODULE_PATH="${ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake/ASC_CMake;${ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake" \
        -DCMAKE_PREFIX_PATH="${ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake" \
        -DNPU_ARCH=dav-2201
    cmake --build "${BUILD}" --target dtype_lowp_fastpath_v001_device -j"$(nproc)"
    echo DEVICE_COMPILE=PASS
    cmake --build "${BUILD}" --target dtype_lowp_fastpath_v001_submission -j"$(nproc)"
    echo SUBMISSION_COMPILE=PASS
    cmake --build "${BUILD}" --target dtype_lowp_fastpath_v001_full_link -j"$(nproc)"
    echo FULL_LINK=PASS
} 2>&1 | tee "${ROOT}/compile.log"
