#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [[ -z "${SMD_V001_OUTPUT:-}" ]]; then
    echo "SMD_V001_OUTPUT must point to the completed per-attempt output directory" >&2
    exit 2
fi
OUTPUT="${SMD_V001_OUTPUT}"
BIN="${OUTPUT}/build/smd_v001_correctness"
DEVICE_ID=1
EXPECTED_SOURCE_SHA256="a689e5abc03d2770b277812d9a52ae0a1aaf910952b737525e1722f06bdb340a"
ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
export ASCEND_HOME_PATH

if [[ -f "${ASCEND_HOME_PATH}/bin/setenv.bash" ]]; then
    set +e
    set +u
    source "${ASCEND_HOME_PATH}/bin/setenv.bash"
    ENV_RC=$?
    set -e
    set -u
elif [[ -f "${ASCEND_HOME_PATH}/set_env.sh" ]]; then
    set +e
    set +u
    source "${ASCEND_HOME_PATH}/set_env.sh"
    ENV_RC=$?
    set -e
    set -u
else
    echo "CANN environment entry was not found under ${ASCEND_HOME_PATH}" >&2
    exit 2
fi
if [[ "${ENV_RC}" -ne 0 ]]; then
    echo "CANN environment setup failed with return code ${ENV_RC}" >&2
    exit 2
fi

if [[ ! -x "${BIN}" ]]; then
    echo "correctness executable is missing: ${BIN}" >&2
    exit 2
fi

SOURCE_SHA256="$(sha256sum "${ROOT}/submission.asc" | awk '{print $1}')"
echo "EXPECTED_SOURCE_SHA256=${EXPECTED_SOURCE_SHA256}"
echo "SOURCE_SHA256=${SOURCE_SHA256}"
if [[ "${SOURCE_SHA256}" != "${EXPECTED_SOURCE_SHA256}" ]]; then
    echo "source identity mismatch" >&2
    exit 3
fi
echo "DEVICE_ID=${DEVICE_ID} DTYPE=BF16 ROWS=2*vector_core_count"
for width in 2049 3073 4095; do
    "${BIN}" "${DEVICE_ID}" "${width}"
done
