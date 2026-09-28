#!/usr/bin/env bash
set -eo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLCHAIN_ENV="/home/data4t2/lelinfeng/cann_game/.toolchains/cann-9.0/cann-9.0.0/set_env.sh"
BUILD_DIR="${BUILD_DIR:-${SCRIPT_DIR}/build}"
NPU_ARCH="${NPU_ARCH:-dav-2201}"

if [[ ! -r "${TOOLCHAIN_ENV}" ]]; then
    echo "missing CANN environment: ${TOOLCHAIN_ENV}" >&2
    exit 2
fi
source "${TOOLCHAIN_ENV}"

for tool in cmake bisheng ccec; do
    command -v "${tool}" >/dev/null
done

[[ -f "${SCRIPT_DIR}/kernel.txt" ]]
[[ -f "${SCRIPT_DIR}/main.txt" ]]
[[ -f "${SCRIPT_DIR}/CMakeLists.txt" ]]

# The local worktree keeps the submission source as .txt; the server build
# directory materializes the CANN direct-invocation .asc entry points.
cp "${SCRIPT_DIR}/kernel.txt" "${SCRIPT_DIR}/kernel.asc"
cp "${SCRIPT_DIR}/main.txt" "${SCRIPT_DIR}/main.asc"

cmake -S "${SCRIPT_DIR}" -B "${BUILD_DIR}" -DSOC_ARCH="${NPU_ARCH}"
cmake --build "${BUILD_DIR}" --parallel "${BUILD_JOBS:-2}"
