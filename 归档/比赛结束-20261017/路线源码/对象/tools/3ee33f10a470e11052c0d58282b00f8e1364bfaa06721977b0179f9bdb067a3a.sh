#!/usr/bin/env bash
# SHAPE-TILING-CHAMPION-X V001 — paired timing build + same-binary + interleaved P/C.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CANN_ROOT="${CANN_ROOT:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
BUILD_DIR="${BUILD_DIR:-${ROOT}/build}"
CMAKE_PACKAGE_DIR="${CANN_ROOT}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
DEVICE="${DEVICE:-4}"
STAGE="${STAGE:-all}"   # all | build | same-binary | paired

PARENT_SHA="a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3"
CANDIDATE_SHA="aed967513cf9c284f8a3226c790dca57a1a6f2fd19048830469291f924ea704b"

actual_parent_sha="$(sha256sum "${ROOT}/../parent/submission.asc" | awk '{print $1}')"
actual_candidate_sha="$(sha256sum "${ROOT}/../../submission.asc" | awk '{print $1}')"
[[ "${actual_parent_sha}" == "${PARENT_SHA}" ]] || { echo "PARENT_SHA_MISMATCH" >&2; exit 3; }
[[ "${actual_candidate_sha}" == "${CANDIDATE_SHA}" ]] || { echo "CANDIDATE_SHA_MISMATCH" >&2; exit 3; }

echo "ROUTE=SHAPE-TILING-CHAMPION-X"
echo "REVISION=V002"
echo "PARENT_SOURCE_SHA256=${actual_parent_sha}"
echo "CANDIDATE_SOURCE_SHA256=${actual_candidate_sha}"
echo "HOST=$(hostname)"
echo "DEVICE=${DEVICE}"
echo "STAGE=${STAGE}"
echo "=== LOAD SNAPSHOT (pre) ==="
npu-smi info | sed -n '1,32p'

export ASCEND_HOME_PATH="${CANN_ROOT}"
export ASCEND_CANN_PACKAGE_PATH="${CANN_ROOT}"
if [[ -f "${CANN_ROOT}/set_env.sh" ]]; then
    # shellcheck disable=SC1091
    source "${CANN_ROOT}/set_env.sh"
fi
ASCEND_DRIVER_LIB_DIR="/usr/local/Ascend/driver/lib64/driver"
ASCEND_DRIVER_COMMON_LIB_DIR="/usr/local/Ascend/driver/lib64/common"
# /usr/lib/aarch64-linux-gnu must lead: the HCC toolchain libstdc++ is too old
# for the host g++-11-built runner (GLIBCXX_3.4.29).
export LD_LIBRARY_PATH="/usr/lib/aarch64-linux-gnu:${CANN_ROOT}/aarch64-linux/lib64:${CANN_ROOT}/lib64:${ASCEND_DRIVER_LIB_DIR}:${ASCEND_DRIVER_COMMON_LIB_DIR}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
GCC_VERSION="$(g++ -dumpversion)"
GCC_TARGET="$(gcc -dumpmachine)"
GCC_MULTIARCH="$(gcc -print-multiarch)"
export CPLUS_INCLUDE_PATH="/usr/include/c++/${GCC_VERSION}:/usr/include/${GCC_MULTIARCH}/c++/${GCC_VERSION}:/usr/include/c++/${GCC_VERSION}/backward${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"
export C_INCLUDE_PATH="/usr/include/${GCC_MULTIARCH}${C_INCLUDE_PATH:+:${C_INCLUDE_PATH}}"
export LIBRARY_PATH="/usr/lib/gcc/${GCC_TARGET}/${GCC_VERSION}:/usr/lib/${GCC_MULTIARCH}:/lib/${GCC_MULTIARCH}${LIBRARY_PATH:+:${LIBRARY_PATH}}"

if [[ "${STAGE}" == "all" || "${STAGE}" == "build" ]]; then
    echo "=== BUILD PAIRED RUNNER ==="
    cmake -S "${ROOT}" -B "${BUILD_DIR}" \
        -DCMAKE_BUILD_TYPE=Release \
        -DASCEND_HOME_PATH="${CANN_ROOT}" \
        -DASCEND_CANN_PACKAGE_PATH="${CANN_ROOT}" \
        -DCMAKE_PREFIX_PATH="${CMAKE_PACKAGE_DIR}"
    cmake --build "${BUILD_DIR}" --parallel 4
    test -f "${BUILD_DIR}/paired_runner"
    echo "BUILD=PASS"
    sha256sum "${BUILD_DIR}/paired_runner"
fi

RUNNER="${BUILD_DIR}/paired_runner"

if [[ "${STAGE}" == "all" || "${STAGE}" == "same-binary" ]]; then
    echo "=== SAME-BINARY QUALIFICATION (parent only) ==="
    "${RUNNER}" --same-binary "${DEVICE}"
fi

if [[ "${STAGE}" == "all" || "${STAGE}" == "paired" ]]; then
    echo "=== INTERLEAVED P/C ==="
    "${RUNNER}" --paired "${DEVICE}"
fi

echo "=== LOAD SNAPSHOT (post) ==="
npu-smi info | sed -n '1,32p'
echo "RUN_COMPLETE"
