#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUTPUT="${SMD_V001_OUTPUT:-${ROOT}/output}"
BIN="${OUTPUT}/build/smd_v001_correctness"
DEVICE_ID=1
EXPECTED_SOURCE_SHA256="a689e5abc03d2770b277812d9a52ae0a1aaf910952b737525e1722f06bdb340a"

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
