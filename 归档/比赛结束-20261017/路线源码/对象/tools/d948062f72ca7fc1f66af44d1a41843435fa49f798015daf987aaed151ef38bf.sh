#!/usr/bin/env bash
set -euo pipefail

if [[ $# -ne 2 || ( "$1" != "--plan-only" && "$1" != "--run" ) ]]; then
    echo "usage: bash run-exact-correctness.sh --plan-only|--run DEVICE_ID" >&2
    exit 64
fi

MODE="$1"
DEVICE_ID="$2"
if [[ ! "${DEVICE_ID}" =~ ^[0-9]+$ ]]; then
    echo "DEVICE_ID must be a nonnegative integer" >&2
    exit 64
fi

R4_ROOT="${MODE_X_R015C_R4_ROOT:-/home/data4t2/lelinfeng/MODE-X-R015C_runs/R015C-r4}"
SOURCE_DIR="${R4_ROOT}/source"
EXECUTABLE="${R4_ROOT}/build/row_copy_reference"
ENV_SCRIPT="${CANN_ENV_SCRIPT:-/usr/local/Ascend/ascend-toolkit/set_env.sh}"

EXPECTED_HOST="fbd3147221a36756d9fd65656635a4d5c92e8ad8be77405ed75c8de97e8726de"
EXPECTED_KERNEL="9367db4ebb4edf6b7bf6cde97f4846f987e6ab230c2aeee44d942d5e881a1c74"
EXPECTED_TILING="0939ba8498426fcd77645d826fcc45a8eb65c78996ad0ce1da1a00d1ac01a250"
EXPECTED_EXECUTABLE="21934a8cf15508b365e252810c8f858b4fb105c5cc5000f460d61c511a37e188"

verify_identity() {
    local label="$1"
    local path="$2"
    local expected="$3"
    local actual
    if [[ ! -f "${path}" ]]; then
        echo "IDENTITY_FAIL ${label} missing=${path}" >&2
        exit 2
    fi
    actual="$(sha256sum "${path}" | awk '{print $1}')"
    printf 'IDENTITY %s path=%s sha256=%s\n' "${label}" "${path}" "${actual}"
    if [[ "${actual}" != "${expected}" ]]; then
        printf 'IDENTITY_FAIL %s expected=%s actual=%s\n' "${label}" "${expected}" "${actual}" >&2
        exit 2
    fi
}

verify_identity host "${SOURCE_DIR}/op_host/row_copy_host.asc" "${EXPECTED_HOST}"
verify_identity kernel "${SOURCE_DIR}/op_kernel/row_copy_kernel.asc" "${EXPECTED_KERNEL}"
verify_identity tiling "${SOURCE_DIR}/op_kernel/row_copy_tiling.h" "${EXPECTED_TILING}"
verify_identity executable "${EXECUTABLE}" "${EXPECTED_EXECUTABLE}"

[[ -f "${ENV_SCRIPT}" ]] || { printf 'CANN environment script not found: %s\n' "${ENV_SCRIPT}" >&2; exit 2; }
set +u
source "${ENV_SCRIPT}"
set -u

shapes=("2 256" "5 4096" "3 8192")
for shape in "${shapes[@]}"; do
    read -r rows cols <<< "${shape}"
    printf 'COMMAND: timeout 60s env ASCEND_DEVICE_ID=%s ASCEND_SLOG_PRINT_TO_STDOUT=1 %q --device-probe %s %s %s\n' \
        "${DEVICE_ID}" "${EXECUTABLE}" "${rows}" "${cols}" "${DEVICE_ID}"
done

if [[ "${MODE}" == "--plan-only" ]]; then
    echo "NPU_ACCESS=NOT_RUN"
    exit 0
fi

echo "NPU_ACCESS=TARGETED_CORRECTNESS_ONLY"
for shape in "${shapes[@]}"; do
    read -r rows cols <<< "${shape}"
    timeout 60s env \
        "ASCEND_DEVICE_ID=${DEVICE_ID}" \
        ASCEND_SLOG_PRINT_TO_STDOUT=1 \
        "${EXECUTABLE}" --device-probe "${rows}" "${cols}" "${DEVICE_ID}"
done
