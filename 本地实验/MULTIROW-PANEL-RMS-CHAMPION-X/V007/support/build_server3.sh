#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
RUN_ROOT="${ROOT}/.."
PARENT_SOURCE="${RUN_ROOT}/parent/submission.asc"
CANDIDATE_SOURCE="${RUN_ROOT}/candidate/submission.asc"
EXPECTED_PARENT_SHA="822546543fce0c38297a8f66e4a6d197438d1cf685f2d5778c35c1576bb9068b"
PARENT_SHA="${R4_PARENT_SOURCE_SHA256:?R4_PARENT_SOURCE_SHA256 is required}"
CANDIDATE_SHA="${R4_CANDIDATE_SOURCE_SHA256:?R4_CANDIDATE_SOURCE_SHA256 is required}"
CANN_ROOT="${CANN_ROOT:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
BUILD_DIR="${ROOT}/build"
CMAKE_PACKAGE_DIR="${CANN_ROOT}/aarch64-linux/tikcpp/ascendc_kernel_cmake"

actual_parent_sha="$(sha256sum "${PARENT_SOURCE}" | awk '{print $1}')"
actual_candidate_sha="$(sha256sum "${CANDIDATE_SOURCE}" | awk '{print $1}')"
[[ "${actual_parent_sha}" == "${EXPECTED_PARENT_SHA}" ]]
[[ "${actual_parent_sha}" == "${PARENT_SHA}" ]]
[[ "${actual_candidate_sha}" == "${CANDIDATE_SHA}" ]]
[[ -d "${CMAKE_PACKAGE_DIR}" ]]

printf 'ROUTE=MULTIROW-PANEL-RMS-CHAMPION-X\nPARENT_SOURCE_SHA256=%s\nCANDIDATE_SOURCE_SHA256=%s\n' \
    "${actual_parent_sha}" "${actual_candidate_sha}"
export ASCEND_HOME_PATH="${CANN_ROOT}"
if [[ -f "${CANN_ROOT}/set_env.sh" ]]; then
    source "${CANN_ROOT}/set_env.sh"
else
    export PATH="${CANN_ROOT}/compiler/ccec_compiler/bin:${CANN_ROOT}/tools/ccec_compiler/bin:${CANN_ROOT}/aarch64-linux/bin:${PATH}"
    export LD_LIBRARY_PATH="${CANN_ROOT}/aarch64-linux/lib64${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
    export CMAKE_PREFIX_PATH="${CMAKE_PACKAGE_DIR}${CMAKE_PREFIX_PATH:+:${CMAKE_PREFIX_PATH}}"
fi
export ASCEND_HOME_PATH="${CANN_ROOT}"
export ASCEND_CANN_PACKAGE_PATH="${CANN_ROOT}"
export R4_PARENT_SOURCE_SHA256="${actual_parent_sha}"
export R4_CANDIDATE_SOURCE_SHA256="${actual_candidate_sha}"
ASCEND_DRIVER_LIB_DIR="/usr/local/Ascend/driver/lib64/driver"
ASCEND_DRIVER_COMMON_LIB_DIR="/usr/local/Ascend/driver/lib64/common"
export LD_LIBRARY_PATH="${CANN_ROOT}/aarch64-linux/lib64:${CANN_ROOT}/lib64:${ASCEND_DRIVER_LIB_DIR}:${ASCEND_DRIVER_COMMON_LIB_DIR}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
GCC_VERSION="$(g++ -dumpversion)"
GCC_TARGET="$(gcc -dumpmachine)"
GCC_MULTIARCH="$(gcc -print-multiarch)"
export CPLUS_INCLUDE_PATH="/usr/include/c++/${GCC_VERSION}:/usr/include/${GCC_MULTIARCH}/c++/${GCC_VERSION}:/usr/include/c++/${GCC_VERSION}/backward${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"
export C_INCLUDE_PATH="/usr/include/${GCC_MULTIARCH}${C_INCLUDE_PATH:+:${C_INCLUDE_PATH}}"
export LIBRARY_PATH="/usr/lib/gcc/${GCC_TARGET}/${GCC_VERSION}:/usr/lib/${GCC_MULTIARCH}:/lib/${GCC_MULTIARCH}${LIBRARY_PATH:+:${LIBRARY_PATH}}"

cmake -S "${ROOT}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DASCEND_HOME_PATH="${CANN_ROOT}" \
    -DASCEND_CANN_PACKAGE_PATH="${CANN_ROOT}" \
    -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH:-${CMAKE_PACKAGE_DIR}}"
cmake --build "${BUILD_DIR}" --parallel 4

for artifact in \
    "${BUILD_DIR}/multirow_panel_rms_runner" \
    "${BUILD_DIR}/libmultirow_panel_rms_parent.so" \
    "${BUILD_DIR}/libmultirow_panel_rms_candidate.so"; do
    test -f "${artifact}"
    printf '\nARTIFACT=%s\n' "${artifact}"
    sha256sum "${artifact}"
    dependencies="$(ldd "${artifact}")"
    printf '%s\n' "${dependencies}"
    if grep -q 'not found' <<<"${dependencies}"; then
        echo "unresolved runtime dependency for ${artifact}" >&2
        exit 1
    fi
done
