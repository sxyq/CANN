#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
"${SCRIPT_DIR}/compile.sh"

if [[ "${RUN_KERNEL:-0}" == "1" ]]; then
    timeout 120 "${BUILD_DIR:-${SCRIPT_DIR}/build}/add_rms_norm_bias_custom"
fi
