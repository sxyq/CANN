#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd -- "$(dirname -- "${BASH_SOURCE[0]}")" && pwd -P)"
REPO_ROOT="$(cd -- "${SCRIPT_DIR}/../../.." && pwd -P)"
CANN_ROOT="${CANN_ROOT:-/home/data4t2/lelinfeng/cann_game/.toolchains/cann-9.0/cann-9.0.0}"
CANDIDATE="${1:?candidate path is required}"
RUN_ROOT="${2:?run root is required}"
ROUTE_NAME="${3:?route name is required}"
PROBE_D_VALUE="${PROBE_D:-64}"
PROBE_ROWS_VALUE="${PROBE_ROWS:-1}"
PROBE_RANK_VALUE="${PROBE_RANK:-2}"
PROBE_DTYPE_VALUE="${PROBE_DTYPE:-fp16}"
PROBE_DEVICE_VALUE="${PROBE_DEVICE:-4}"

case "${CANDIDATE}" in
    /*) ;;
    *) CANDIDATE="${REPO_ROOT}/${CANDIDATE}" ;;
esac
case "${RUN_ROOT}" in
    /*) ;;
    *) RUN_ROOT="${REPO_ROOT}/${RUN_ROOT}" ;;
esac

[[ -f "${CANDIDATE}" ]] || { printf '%s\n' "[ERROR] candidate not found: ${CANDIDATE}" >&2; exit 2; }
[[ -f "${SCRIPT_DIR}/CMakeLists.txt" ]] || { printf '%s\n' "[ERROR] CMakeLists.txt missing" >&2; exit 2; }
[[ -f "${SCRIPT_DIR}/main.asc" ]] || { printf '%s\n' "[ERROR] main.asc missing" >&2; exit 2; }
[[ -f "${SCRIPT_DIR}/data_utils.h" ]] || { printf '%s\n' "[ERROR] data_utils.h missing" >&2; exit 2; }
[[ -f "${SCRIPT_DIR}/gen_probe.py" ]] || { printf '%s\n' "[ERROR] gen_probe.py missing" >&2; exit 2; }
[[ -f "${CANN_ROOT}/set_env.sh" ]] || { printf '%s\n' "[ERROR] CANN_ROOT invalid: ${CANN_ROOT}" >&2; exit 2; }

mkdir -p "${RUN_ROOT}/input" "${RUN_ROOT}/output" "${RUN_ROOT}/build" "${RUN_ROOT}/log"
cp "${CANDIDATE}" "${RUN_ROOT}/kernel.asc"
cp "${SCRIPT_DIR}/CMakeLists.txt" "${RUN_ROOT}/CMakeLists.txt"
cp "${SCRIPT_DIR}/main.asc" "${RUN_ROOT}/main.asc"
cp "${SCRIPT_DIR}/data_utils.h" "${RUN_ROOT}/data_utils.h"
cp "${SCRIPT_DIR}/gen_probe.py" "${RUN_ROOT}/gen_probe.py"

SOURCE_SHA="$(shasum -a 256 "${CANDIDATE}" | awk '{print $1}')"
RUN_SHA="$(shasum -a 256 "${RUN_ROOT}/kernel.asc" | awk '{print $1}')"
[[ "${SOURCE_SHA}" == "${RUN_SHA}" ]] || { printf '%s\n' '[ERROR] kernel copy SHA mismatch' >&2; exit 3; }

{
    printf 'ROUTE=%s\n' "${ROUTE_NAME}"
    printf 'CANDIDATE=%s\n' "${CANDIDATE}"
    printf 'CANDIDATE_SHA256=%s\n' "${SOURCE_SHA}"
    printf 'RUN_ROOT=%s\n' "${RUN_ROOT}"
    printf 'CANN_ROOT=%s\n' "${CANN_ROOT}"
    printf 'PROBE_D=%s PROBE_ROWS=%s PROBE_RANK=%s PROBE_DTYPE=%s PROBE_DEVICE=%s\n' \
        "${PROBE_D_VALUE}" "${PROBE_ROWS_VALUE}" "${PROBE_RANK_VALUE}" \
        "${PROBE_DTYPE_VALUE}" "${PROBE_DEVICE_VALUE}"
} | tee "${RUN_ROOT}/log/parameters.log"

source "${CANN_ROOT}/set_env.sh"
command -v bisheng | tee "${RUN_ROOT}/log/toolchain.log"
command -v ccec | tee -a "${RUN_ROOT}/log/toolchain.log"
command -v npu-smi | tee -a "${RUN_ROOT}/log/toolchain.log"

python3 "${RUN_ROOT}/gen_probe.py"
cmake -S "${RUN_ROOT}" -B "${RUN_ROOT}/build" \
    -DCMAKE_BUILD_TYPE=Release \
    2>&1 | tee "${RUN_ROOT}/log/configure.log"
cmake --build "${RUN_ROOT}/build" --parallel 1 \
    2>&1 | tee "${RUN_ROOT}/log/compile.log"

(cd "${RUN_ROOT}" && \
    PROBE_D="${PROBE_D_VALUE}" \
    PROBE_ROWS="${PROBE_ROWS_VALUE}" \
    PROBE_RANK="${PROBE_RANK_VALUE}" \
    PROBE_DTYPE="${PROBE_DTYPE_VALUE}" \
    PROBE_DEVICE="${PROBE_DEVICE_VALUE}" \
    ./build/add_rms_norm_bias_custom \
    2>&1 | tee "${RUN_ROOT}/log/run.log")
