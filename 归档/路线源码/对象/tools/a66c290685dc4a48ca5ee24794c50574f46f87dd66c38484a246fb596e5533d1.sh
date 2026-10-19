#!/usr/bin/env bash
set -uo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLKIT_ROOT="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002"
ASCEND_ROOT="${TOOLKIT_ROOT}/aarch64-linux"
HCC_ROOT="${TOOLKIT_ROOT}/toolkit/toolchain/hcc/aarch64-target-linux-gnu"
LOG_DIR="${ROOT}/logs"
BUILD_DIR="${ROOT}/build"
if [[ -e "${LOG_DIR}" || -e "${BUILD_DIR}" ]]; then
    printf 'ERROR=attempt output path already exists; refusing to overwrite: %s\n' "${ROOT}" >&2
    exit 90
fi
mkdir -p "${LOG_DIR}"

set +u
export ASCEND_HOME_PATH="${TOOLKIT_ROOT}"
source "${ASCEND_ROOT}/script/set_env.sh"
set -u

HCC_PATHS="${HCC_ROOT}/include:${HCC_ROOT}/include/c++/7.3.0:${HCC_ROOT}/include/c++/7.3.0/aarch64-target-linux-gnu:${HCC_ROOT}/include/c++/7.3.0/backward"
export CPATH="${HCC_PATHS}${CPATH:+:${CPATH}}"
export CPLUS_INCLUDE_PATH="${HCC_PATHS}${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"

printf 'COMMAND=cmake -S %q -B %q -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_BUILD_TYPE=Release\n' \
    "${ROOT}" "${BUILD_DIR}" > "${LOG_DIR}/configure.log"
cmake -S "${ROOT}" -B "${BUILD_DIR}" \
    -DCMAKE_VERBOSE_MAKEFILE=ON -DCMAKE_BUILD_TYPE=Release \
    >> "${LOG_DIR}/configure.log" 2>&1
configure_exit=$?
printf 'EXIT_CODE=%s\n' "${configure_exit}" >> "${LOG_DIR}/configure.log"

candidate_build_exit=125
parent_build_exit=125
candidate_run_exit=125
parent_run_exit=125
if [[ ${configure_exit} -eq 0 ]]; then
    printf 'COMMAND=cmake --build %q --target async_triple_candidate_probe --verbose -j2\n' \
        "${BUILD_DIR}" > "${LOG_DIR}/candidate-build.log"
    cmake --build "${BUILD_DIR}" --target async_triple_candidate_probe --verbose -j2 \
        >> "${LOG_DIR}/candidate-build.log" 2>&1
    candidate_build_exit=$?
    printf 'EXIT_CODE=%s\n' "${candidate_build_exit}" >> "${LOG_DIR}/candidate-build.log"

    printf 'COMMAND=cmake --build %q --target async_triple_parent_probe --verbose -j2\n' \
        "${BUILD_DIR}" > "${LOG_DIR}/parent-build.log"
    cmake --build "${BUILD_DIR}" --target async_triple_parent_probe --verbose -j2 \
        >> "${LOG_DIR}/parent-build.log" 2>&1
    parent_build_exit=$?
    printf 'EXIT_CODE=%s\n' "${parent_build_exit}" >> "${LOG_DIR}/parent-build.log"
fi

if [[ ${candidate_build_exit} -eq 0 ]]; then
    printf 'COMMAND=%q\n' "${BUILD_DIR}/async_triple_candidate_probe" > "${LOG_DIR}/candidate.command.txt"
    "${BUILD_DIR}/async_triple_candidate_probe" \
        > "${LOG_DIR}/candidate.stdout.log" 2> "${LOG_DIR}/candidate.stderr.log"
    candidate_run_exit=$?
fi

if [[ ${parent_build_exit} -eq 0 ]]; then
    printf 'COMMAND=%q\n' "${BUILD_DIR}/async_triple_parent_probe" > "${LOG_DIR}/parent.command.txt"
    "${BUILD_DIR}/async_triple_parent_probe" \
        > "${LOG_DIR}/parent.stdout.log" 2> "${LOG_DIR}/parent.stderr.log"
    parent_run_exit=$?
fi

{
    printf 'HOSTNAME=%s\n' "$(hostname)"
    printf 'DATE=%s\n' "$(date -Is)"
    printf 'TOOLKIT_ROOT=%s\n' "${TOOLKIT_ROOT}"
    printf 'ASCEND_HOME_PATH=%s\n' "${ASCEND_HOME_PATH:-<unset>}"
    printf 'CPLUS_INCLUDE_PATH=%s\n' "${CPLUS_INCLUDE_PATH}"
    printf 'CONFIGURE_EXIT=%s\n' "${configure_exit}"
    printf 'CANDIDATE_BUILD_EXIT=%s\n' "${candidate_build_exit}"
    printf 'PARENT_BUILD_EXIT=%s\n' "${parent_build_exit}"
    printf 'CANDIDATE_RUN_EXIT=%s\n' "${candidate_run_exit}"
    printf 'PARENT_RUN_EXIT=%s\n' "${parent_run_exit}"
} > "${LOG_DIR}/summary.log"

cat "${LOG_DIR}/summary.log"
if [[ ${configure_exit} -ne 0 || ${candidate_build_exit} -ne 0 || ${parent_build_exit} -ne 0 ]]; then
    exit 2
fi
if [[ ${candidate_run_exit} -ne 0 || ${parent_run_exit} -ne 0 ]]; then
    exit 3
fi
