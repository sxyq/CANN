#!/bin/bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
cd "${SCRIPT_DIR}"

TOOLKIT_ROOT="${ASCEND_HOME_PATH:-/home/data4t2/lelinfeng/cann_game/.toolchains/cann-9.0/cann-9.0.0}"
source "${TOOLKIT_ROOT}/set_env.sh"

cp kernel.txt kernel.asc
cmake -S . -B build -DNPU_ARCH=dav-2201
cmake --build build -j4
