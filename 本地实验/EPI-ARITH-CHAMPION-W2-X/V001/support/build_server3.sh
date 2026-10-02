#!/usr/bin/env bash
set -euo pipefail

SUPPORT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
V001_DIR="$(cd "${SUPPORT_DIR}/.." && pwd)"
REPO_ROOT="$(cd "${SUPPORT_DIR}/../../../../" && pwd)"
PARENT_SOURCE="${SUPPORT_DIR}/../../../../线上结果/R31B/V011/submission.asc"
CANDIDATE_SOURCE="${V001_DIR}/submission.asc"
PARENT_SHA="a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3"
CANDIDATE_SHA="9a28f5cd8703dc4ff5c46fba09f59a537132594e5a879a50a17349086bfe4d59"
CANN_ROOT="${CANN_ROOT:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
RUN_ID="${EPI_RUN_ID:-}"
OUTPUT_ROOT="${EPI_SERVER_OUTPUT_ROOT:-/home/data4t2/lelinfeng/cann/server_runs/EPI-ARITH-CHAMPION-W2-X/V001}"
SERVER_ROOT="/home/data4t2/lelinfeng/cann"

[[ "${RUN_ID}" =~ ^[0-9]{8}T[0-9]{6}Z$ ]] || {
    echo "EPI_RUN_ID must use UTC format YYYYMMDDTHHMMSSZ" >&2
    exit 2
}
RUN_DIR="${OUTPUT_ROOT}/${RUN_ID}"
mkdir -p "${OUTPUT_ROOT}"
mkdir "${RUN_DIR}"
mkdir "${RUN_DIR}/logs"
BUILD_LOG="${RUN_DIR}/logs/build.log"
exec > >(tee "${BUILD_LOG}") 2>&1

record_server_snapshot() {
    local phase="$1"
    printf '\n=== %s ===\n' "${phase}"
    date -u '+SNAPSHOT_UTC=%Y-%m-%dT%H:%M:%SZ'
    hostname
    echo "ASSIGNED_DEVICE=7"
    npu-smi info
    ps -eo pid,ppid,user,stat,comm
    df -h "${SERVER_ROOT}"
    du -sh "${SERVER_ROOT}"/*
}

check_disk_space() {
    local available_kb
    available_kb="$(df -Pk "${SERVER_ROOT}" | awk 'NR == 2 {print $4}')"
    [[ "${available_kb}" =~ ^[0-9]+$ ]]
    printf 'AVAILABLE_DISK_KB=%s REQUIRED_DISK_KB=15728640\n' "${available_kb}"
    (( available_kb >= 15728640 ))
}

finish_build() {
    local rc=$?
    trap - EXIT
    record_server_snapshot BUILD_POST || true
    exit "${rc}"
}
trap finish_build EXIT

echo "ROUTE=EPI-ARITH-CHAMPION-W2-X REVISION=V001 RUN_ID=${RUN_ID}"
echo "REPO_ROOT=${REPO_ROOT}"
echo "RUN_DIR=${RUN_DIR}"
date -u '+BUILD_STARTED=%Y-%m-%dT%H:%M:%SZ'
record_server_snapshot BUILD_PRE
if ! check_disk_space; then
    echo "BUILD=INCOMPLETE reason=AVAILABLE_DISK_BELOW_15_GIB"
    exit 2
fi

PARENT_ACTUAL_SHA="$(sha256sum "${PARENT_SOURCE}" | awk '{print $1}')"
CANDIDATE_ACTUAL_SHA="$(sha256sum "${CANDIDATE_SOURCE}" | awk '{print $1}')"
echo "PARENT_SOURCE_SHA256=${PARENT_ACTUAL_SHA}"
echo "CANDIDATE_SOURCE_SHA256=${CANDIDATE_ACTUAL_SHA}"
[[ "${PARENT_ACTUAL_SHA}" == "${PARENT_SHA}" ]]
[[ "${CANDIDATE_ACTUAL_SHA}" == "${CANDIDATE_SHA}" ]]
[[ -d "${CANN_ROOT}/aarch64-linux/tikcpp/ascendc_kernel_cmake" ]]
uname -m
sed -n '1,3p' "${CANN_ROOT}/compiler/version.info"
g++ --version | head -n 1

export ASCEND_HOME_PATH="${CANN_ROOT}"
export ASCEND_CANN_PACKAGE_PATH="${CANN_ROOT}"
export CMAKE_PREFIX_PATH="${CANN_ROOT}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
export PATH="${CANN_ROOT}/compiler/ccec_compiler/bin:${CANN_ROOT}/tools/ccec_compiler/bin:${CANN_ROOT}/aarch64-linux/bin:${PATH}"
export LD_LIBRARY_PATH="${CANN_ROOT}/aarch64-linux/lib64:${CANN_ROOT}/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"

GCC_VERSION="$(g++ -dumpversion)"
GCC_TARGET="$(gcc -dumpmachine)"
GCC_MULTIARCH="$(gcc -print-multiarch)"
export CPLUS_INCLUDE_PATH="/usr/include/c++/${GCC_VERSION}:/usr/include/${GCC_MULTIARCH}/c++/${GCC_VERSION}:/usr/include/c++/${GCC_VERSION}/backward${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"
export C_INCLUDE_PATH="/usr/include/${GCC_MULTIARCH}${C_INCLUDE_PATH:+:${C_INCLUDE_PATH}}"
export LIBRARY_PATH="/usr/lib/gcc/${GCC_TARGET}/${GCC_VERSION}:/usr/lib/${GCC_MULTIARCH}:/lib/${GCC_MULTIARCH}${LIBRARY_PATH:+:${LIBRARY_PATH}}"

BUILD_DIR="${RUN_DIR}/build"
cmake -S "${SUPPORT_DIR}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DASCEND_HOME_PATH="${CANN_ROOT}" \
    -DCMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH}"
cmake --build "${BUILD_DIR}" --parallel 4

for artifact in \
    "${BUILD_DIR}/epi_arith_v001_correctness" \
    "${BUILD_DIR}/libepi_arith_v001_parent.so" \
    "${BUILD_DIR}/libepi_arith_v001_candidate.so"; do
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

printf 'NPU_CORRECTNESS=NOT_RUN\nTIMING=NOT_RUN\n'
date -u '+BUILD_FINISHED=%Y-%m-%dT%H:%M:%SZ'
