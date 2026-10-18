#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
ENV_SCRIPT="/usr/local/Ascend/ascend-toolkit/set_env.sh"
test -r "${ENV_SCRIPT}" || { echo "missing CANN environment script: ${ENV_SCRIPT}" >&2; exit 1; }
# The versioned install is selected by the canonical environment script.
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
export PYTHONPATH="${PYTHONPATH:-}"
source "${ENV_SCRIPT}"

: "${ASCEND_HOME_PATH:?ASCEND_HOME_PATH was not set by ${ENV_SCRIPT}}"
: "${RUNNER_PARENT_SOURCE:?RUNNER_PARENT_SOURCE is required}"
: "${RUNNER_CANDIDATE_SOURCE:?RUNNER_CANDIDATE_SOURCE is required}"
: "${RUNNER_PARENT_SHA256:?RUNNER_PARENT_SHA256 is required}"
: "${RUNNER_CANDIDATE_SHA256:?RUNNER_CANDIDATE_SHA256 is required}"
: "${RUNNER_ASCENDC_ARCH:?RUNNER_ASCENDC_ARCH is required (for example dav-c220)}"

[[ "${RUNNER_PARENT_SOURCE}" == /* && "${RUNNER_CANDIDATE_SOURCE}" == /* ]] || {
    echo "parent and candidate source paths must be absolute" >&2
    exit 2
}
test -f "${RUNNER_PARENT_SOURCE}" || { echo "parent source is not a regular file: ${RUNNER_PARENT_SOURCE}" >&2; exit 1; }
test -f "${RUNNER_CANDIDATE_SOURCE}" || { echo "candidate source is not a regular file: ${RUNNER_CANDIDATE_SOURCE}" >&2; exit 1; }
[[ "${RUNNER_PARENT_SHA256}" =~ ^[[:xdigit:]]{64}$ ]] || { echo "invalid RUNNER_PARENT_SHA256" >&2; exit 2; }
[[ "${RUNNER_CANDIDATE_SHA256}" =~ ^[[:xdigit:]]{64}$ ]] || { echo "invalid RUNNER_CANDIDATE_SHA256" >&2; exit 2; }

parent_sha="$(sha256sum "${RUNNER_PARENT_SOURCE}" | awk '{print $1}')"
candidate_sha="$(sha256sum "${RUNNER_CANDIDATE_SOURCE}" | awk '{print $1}')"
[[ "${parent_sha,,}" == "${RUNNER_PARENT_SHA256,,}" ]] || {
    echo "parent source SHA256 mismatch: expected=${RUNNER_PARENT_SHA256} actual=${parent_sha}" >&2
    exit 1
}
[[ "${candidate_sha,,}" == "${RUNNER_CANDIDATE_SHA256,,}" ]] || {
    echo "candidate source SHA256 mismatch: expected=${RUNNER_CANDIDATE_SHA256} actual=${candidate_sha}" >&2
    exit 1
}

toolkit_version_file="${ASCEND_HOME_PATH}/toolkit/version.info"
test -r "${toolkit_version_file}" || { echo "missing toolkit version file: ${toolkit_version_file}" >&2; exit 1; }
grep -q '^Version=8\.5\.T8\.0\.B060$' "${toolkit_version_file}" || {
    echo "CANN 8.5.T8.0.B060 is required; found:" >&2
    sed -n '1,3p' "${toolkit_version_file}" >&2
    exit 1
}

export RUNNER_PARENT_SOURCE RUNNER_CANDIDATE_SOURCE
export RUNNER_PARENT_SHA256 RUNNER_CANDIDATE_SHA256 RUNNER_ASCENDC_ARCH
HCC_ROOT="${ASCEND_HOME_PATH}/toolkit/toolchain/hcc"
HCC_CXX_INCLUDE_FLAGS="-I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0 -I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu -I${HCC_ROOT}/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
export CXXFLAGS="${CXXFLAGS:-} ${HCC_CXX_INCLUDE_FLAGS}"
export CFLAGS="${CFLAGS:-} ${HCC_CXX_INCLUDE_FLAGS}"
BUILD_DIR="${RUNNER_BUILD_DIR:-${ROOT}/build}"
cmake -S "${ROOT}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DASCEND_HOME_PATH="${ASCEND_HOME_PATH}" \
    -DCMAKE_CXX_FLAGS="${CXXFLAGS}" \
    -DCMAKE_C_FLAGS="${CFLAGS}" \
    -DRUNNER_PARENT_SOURCE="${RUNNER_PARENT_SOURCE}" \
    -DRUNNER_CANDIDATE_SOURCE="${RUNNER_CANDIDATE_SOURCE}" \
    -DRUNNER_PARENT_SHA256="${RUNNER_PARENT_SHA256}" \
    -DRUNNER_CANDIDATE_SHA256="${RUNNER_CANDIDATE_SHA256}" \
    -DRUNNER_ASCENDC_ARCH="${RUNNER_ASCENDC_ARCH}" \
    -DCMAKE_PREFIX_PATH="${ASCEND_HOME_PATH}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
cmake --build "${BUILD_DIR}" --parallel "${RUNNER_BUILD_JOBS:-4}"

for artifact in "${BUILD_DIR}/route_runner" "${BUILD_DIR}/libroute_runner_parent.so" "${BUILD_DIR}/libroute_runner_candidate.so"; do
    test -f "${artifact}" || { echo "missing build artifact: ${artifact}" >&2; exit 1; }
    printf 'artifact_sha256\t%s\t' "${artifact}"
    sha256sum "${artifact}" | awk '{print $1}'
done

identity_file="${BUILD_DIR}/runner_identity.tsv"
{
    printf 'field\tvalue\n'
    printf 'toolkit_version\t8.5.T8.0.B060\n'
    printf 'ascend_home_path\t%s\n' "${ASCEND_HOME_PATH}"
    printf 'ascendc_arch\t%s\n' "${RUNNER_ASCENDC_ARCH}"
    printf 'parent_source\t%s\n' "${RUNNER_PARENT_SOURCE}"
    printf 'parent_source_sha256\t%s\n' "${parent_sha}"
    printf 'candidate_source\t%s\n' "${RUNNER_CANDIDATE_SOURCE}"
    printf 'candidate_source_sha256\t%s\n' "${candidate_sha}"
    for artifact in "${BUILD_DIR}/route_runner" "${BUILD_DIR}/libroute_runner_parent.so" "${BUILD_DIR}/libroute_runner_candidate.so"; do
        printf 'artifact_sha256\t%s\t' "${artifact}"
        sha256sum "${artifact}" | awk '{print $1}'
    done
} > "${identity_file}"
printf 'runner_identity\t%s\n' "${identity_file}"
