#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
args=(--mode correctness)
while (($#)); do
    if [[ "$1" == "--mode" ]]; then
        (($# >= 2)) || { echo "--mode requires a value" >&2; exit 2; }
        shift 2
        continue
    fi
    args+=("$1")
    shift
done
exec "${ROOT}/build/route_runner" "${args[@]}"
