#!/usr/bin/env bash
set -euo pipefail

SMD_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
if [[ -z "${SMD_V001_OUTPUT:-}" ]]; then
    echo "SMD_V001_OUTPUT must point to the completed per-attempt output directory" >&2
    exit 2
fi
SMD_OUTPUT="${SMD_V001_OUTPUT}"
SMD_CORRECTNESS_BIN="${SMD_OUTPUT}/build/smd_v001_correctness"
SMD_DEVICE_ID=1
SMD_EXPECTED_SOURCE_SHA256="a689e5abc03d2770b277812d9a52ae0a1aaf910952b737525e1722f06bdb340a"
ASCEND_HOME_PATH="${ASCEND_HOME_PATH:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
export ASCEND_HOME_PATH

if [[ -f "${ASCEND_HOME_PATH}/bin/setenv.bash" ]]; then
    set +e
    set +u
    source "${ASCEND_HOME_PATH}/bin/setenv.bash"
    SMD_ENV_RC=$?
    set -e
    set -u
elif [[ -f "${ASCEND_HOME_PATH}/set_env.sh" ]]; then
    set +e
    set +u
    source "${ASCEND_HOME_PATH}/set_env.sh"
    SMD_ENV_RC=$?
    set -e
    set -u
else
    echo "CANN environment entry was not found under ${ASCEND_HOME_PATH}" >&2
    exit 2
fi
if [[ "${SMD_ENV_RC}" -ne 0 ]]; then
    echo "CANN environment setup failed with return code ${SMD_ENV_RC}" >&2
    exit 2
fi

SMD_CANN_LIB_DIR="${ASCEND_HOME_PATH}/aarch64-linux/lib64"
export LD_LIBRARY_PATH="${SMD_CANN_LIB_DIR}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"

if [[ ! -x "${SMD_CORRECTNESS_BIN}" ]]; then
    echo "correctness executable is missing: ${SMD_CORRECTNESS_BIN}" >&2
    exit 2
fi

SMD_SOURCE_SHA256="$(sha256sum "${SMD_ROOT}/submission.asc" | awk '{print $1}')"
echo "EXPECTED_SOURCE_SHA256=${SMD_EXPECTED_SOURCE_SHA256}"
echo "SOURCE_SHA256=${SMD_SOURCE_SHA256}"
if [[ "${SMD_SOURCE_SHA256}" != "${SMD_EXPECTED_SOURCE_SHA256}" ]]; then
    echo "source identity mismatch" >&2
    exit 3
fi
echo "DEVICE_ID=${SMD_DEVICE_ID} DTYPE=BF16 ROWS=2*vector_core_count"
for width in 2049 3073 4095; do
    "${SMD_CORRECTNESS_BIN}" "${SMD_DEVICE_ID}" "${width}"
done
