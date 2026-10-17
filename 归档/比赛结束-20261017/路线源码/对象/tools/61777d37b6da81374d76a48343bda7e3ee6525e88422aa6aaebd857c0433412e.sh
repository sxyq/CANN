#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

TEMPLATE_DIR="${ADDRMSNORM_TEMPLATE_DIR:-/home/data4t2/lelinfeng/addrmsnormbias_problem_1742_template}"
TOOLCHAIN_ENV="${CANN_ENV_SCRIPT:-/home/data4t2/lelinfeng/cann_game/.toolchains/cann-9.0/cann-9.0.0/set_env.sh}"

if [[ ! -f "${TEMPLATE_DIR}/main.asc" || ! -f "${TEMPLATE_DIR}/data_utils.h" ]]; then
    echo "missing direct-invocation template: ${TEMPLATE_DIR}" >&2
    exit 2
fi

if [[ ! -f main.asc ]]; then
    ln -s "${TEMPLATE_DIR}/main.asc" main.asc
fi
if [[ ! -f data_utils.h ]]; then
    ln -s "${TEMPLATE_DIR}/data_utils.h" data_utils.h
fi
if [[ ! -f kernel.asc ]]; then
    ln -s kernel.txt kernel.asc
fi

set +u
source "${TOOLCHAIN_ENV}"
set -u

rm -rf build
cmake -S . -B build -DNPU_ARCH="${NPU_ARCH:-dav-2201}"
cmake --build build --parallel 4
