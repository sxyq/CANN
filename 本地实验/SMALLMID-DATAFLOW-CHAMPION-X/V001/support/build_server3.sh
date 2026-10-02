#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
SUPPORT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
if [[ -z "${SMD_V001_OUTPUT:-}" ]]; then
    echo "SMD_V001_OUTPUT must point to a new per-attempt output directory" >&2
    exit 2
fi
OUTPUT="${SMD_V001_OUTPUT}"
BUILD="${OUTPUT}/build"
EXPECTED_SOURCE_SHA256="a689e5abc03d2770b277812d9a52ae0a1aaf910952b737525e1722f06bdb340a"
ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
export ASCEND_HOME_PATH
export PATH="${ASCEND_HOME_PATH}/bin:${ASCEND_HOME_PATH}/aarch64-linux/ccec_compiler/bin:${PATH}"

if [[ -f "${ASCEND_HOME_PATH}/bin/setenv.bash" ]]; then
    set +e
    set +u
    source "${ASCEND_HOME_PATH}/bin/setenv.bash"
    ENV_RC=$?
    set -e
    set -u
    if [[ "${ENV_RC}" -ne 0 ]]; then
        echo "CANN environment setup failed with return code ${ENV_RC}" >&2
        exit 2
    fi
elif [[ -f "${ASCEND_HOME_PATH}/set_env.sh" ]]; then
    set +e
    set +u
    source "${ASCEND_HOME_PATH}/set_env.sh"
    ENV_RC=$?
    set -e
    set -u
    if [[ "${ENV_RC}" -ne 0 ]]; then
        echo "CANN environment setup failed with return code ${ENV_RC}" >&2
        exit 2
    fi
else
    echo "CANN environment entry was not found under ${ASCEND_HOME_PATH}" >&2
    exit 2
fi

KIT="${ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
HCC="${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0"
CXX_COMPILER="${ASCEND_HOME_PATH}/toolkit/toolchain/hcc/bin/aarch64-target-linux-gnu-g++"
if [[ ! -x "${CXX_COMPILER}" ]]; then
    echo "AArch64 C++ compiler was not found: ${CXX_COMPILER}" >&2
    exit 2
fi
export CPLUS_INCLUDE_PATH="${HCC}:${HCC}/aarch64-target-linux-gnu:${HCC}/backward${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"
export C_INCLUDE_PATH="${HCC}:${HCC}/aarch64-target-linux-gnu${C_INCLUDE_PATH:+:${C_INCLUDE_PATH}}"
export CMAKE_PREFIX_PATH="${KIT}${CMAKE_PREFIX_PATH:+:${CMAKE_PREFIX_PATH}}"
export ASC_DIR="${KIT}"

if [[ -e "${OUTPUT}" ]]; then
    echo "per-attempt output directory already exists: ${OUTPUT}" >&2
    exit 2
fi
mkdir -p "${BUILD}"
SOURCE_SHA256="$(sha256sum "${ROOT}/submission.asc" | awk '{print $1}')"
echo "EXPECTED_SOURCE_SHA256=${EXPECTED_SOURCE_SHA256}"
echo "SOURCE_SHA256=${SOURCE_SHA256}"
if [[ "${SOURCE_SHA256}" != "${EXPECTED_SOURCE_SHA256}" ]]; then
    echo "source identity mismatch" >&2
    exit 3
fi
echo "ASCEND_HOME_PATH=${ASCEND_HOME_PATH}"
echo "SOC_VERSION=Ascend910B3 NPU_ARCH=dav-2201"
cmake -S "${SUPPORT}" -B "${BUILD}" \
    -DCMAKE_MODULE_PATH="${KIT}/ASC_CMake;${KIT}" \
    -DCMAKE_PREFIX_PATH="${KIT}" \
    -DCMAKE_CXX_COMPILER="${CXX_COMPILER}" \
    -DSOC_VERSION=Ascend910B3 \
    -DNPU_ARCH=dav-2201
cmake --build "${BUILD}" --target smd_v001_correctness --parallel "$(nproc)"
