#!/usr/bin/env bash
set -euo pipefail

ROUTE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
TOOLCHAIN_ROOT="${CANN_TOOLCHAIN_ROOT:-/home/data4t2/lelinfeng/cann_game/.toolchains/cann-9.0/cann-9.0.0}"
BUILD_DIR="${BUILD_DIR:-${ROUTE_DIR}/build}"

# The project-local toolchain appends to these variables while nounset is active.
export LD_LIBRARY_PATH="${LD_LIBRARY_PATH:-}"
export PYTHONPATH="${PYTHONPATH:-}"
export CMAKE_PREFIX_PATH="${CMAKE_PREFIX_PATH:-}"
export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:${CPLUS_INCLUDE_PATH:-}"
export LIBRARY_PATH="/usr/lib/gcc/aarch64-linux-gnu/11:/usr/lib/aarch64-linux-gnu:${LIBRARY_PATH:-}"
source "${TOOLCHAIN_ROOT}/set_env.sh"

# The Mac worktree keeps source text as .txt; server compilation materializes .asc.
cp "${ROUTE_DIR}/op_kernel/addrmsnormbias_kernel.txt" \
   "${ROUTE_DIR}/op_kernel/addrmsnormbias_kernel.asc"
cp "${ROUTE_DIR}/op_host/addrmsnormbias_host.txt" \
   "${ROUTE_DIR}/op_host/addrmsnormbias_host.asc"

cmake -S "${ROUTE_DIR}" -B "${BUILD_DIR}" -DCMAKE_BUILD_TYPE=Release
cmake --build "${BUILD_DIR}" -- -j2
