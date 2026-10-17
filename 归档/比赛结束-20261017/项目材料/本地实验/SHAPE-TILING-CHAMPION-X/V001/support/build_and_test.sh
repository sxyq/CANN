#!/usr/bin/env bash
# SHAPE-TILING-CHAMPION-X V001 — server3 build + NPU correctness.
# Run from the support/ directory on server3 after rsync.
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
CANDIDATE_SOURCE="${ROOT}/../submission.asc"
CANDIDATE_SHA="592c310f728a38f8353fde9d1ad12f495645d619522147e0b3ca397b2357e51b"
CANN_ROOT="${CANN_ROOT:-/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002}"
BUILD_DIR="${BUILD_DIR:-${ROOT}/build}"
CMAKE_PACKAGE_DIR="${CANN_ROOT}/aarch64-linux/tikcpp/ascendc_kernel_cmake"
DEVICE="${DEVICE:-0}"

actual_candidate_sha="$(sha256sum "${CANDIDATE_SOURCE}" | awk '{print $1}')"
if [[ "${actual_candidate_sha}" != "${CANDIDATE_SHA}" ]]; then
    echo "SOURCE_SHA_MISMATCH actual=${actual_candidate_sha} expected=${CANDIDATE_SHA}" >&2
    exit 3
fi
[[ -d "${CMAKE_PACKAGE_DIR}" ]]

echo "ROUTE=SHAPE-TILING-CHAMPION-X"
echo "REVISION=V001"
echo "CANDIDATE_SOURCE_SHA256=${actual_candidate_sha}"
echo "HOST=$(hostname)"
echo "CANN_ROOT=${CANN_ROOT}"
echo "DEVICE=${DEVICE}"
echo "BUILD_DIR=${BUILD_DIR}"

export ASCEND_HOME_PATH="${CANN_ROOT}"
export ASCEND_CANN_PACKAGE_PATH="${CANN_ROOT}"
if [[ -f "${CANN_ROOT}/set_env.sh" ]]; then
    # shellcheck disable=SC1091
    source "${CANN_ROOT}/set_env.sh"
fi
ASCEND_DRIVER_LIB_DIR="/usr/local/Ascend/driver/lib64/driver"
ASCEND_DRIVER_COMMON_LIB_DIR="/usr/local/Ascend/driver/lib64/common"
export LD_LIBRARY_PATH="${CANN_ROOT}/aarch64-linux/lib64:${CANN_ROOT}/lib64:${ASCEND_DRIVER_LIB_DIR}:${ASCEND_DRIVER_COMMON_LIB_DIR}${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
GCC_VERSION="$(g++ -dumpversion)"
GCC_TARGET="$(gcc -dumpmachine)"
GCC_MULTIARCH="$(gcc -print-multiarch)"
export CPLUS_INCLUDE_PATH="/usr/include/c++/${GCC_VERSION}:/usr/include/${GCC_MULTIARCH}/c++/${GCC_VERSION}:/usr/include/c++/${GCC_VERSION}/backward${CPLUS_INCLUDE_PATH:+:${CPLUS_INCLUDE_PATH}}"
export C_INCLUDE_PATH="/usr/include/${GCC_MULTIARCH}${C_INCLUDE_PATH:+:${C_INCLUDE_PATH}}"
export LIBRARY_PATH="/usr/lib/gcc/${GCC_TARGET}/${GCC_VERSION}:/usr/lib/${GCC_MULTIARCH}:/lib/${GCC_MULTIARCH}${LIBRARY_PATH:+:${LIBRARY_PATH}}"

echo "=== CONFIGURE + BUILD ==="
cmake -S "${ROOT}" -B "${BUILD_DIR}" \
    -DCMAKE_BUILD_TYPE=Release \
    -DASCEND_HOME_PATH="${CANN_ROOT}" \
    -DASCEND_CANN_PACKAGE_PATH="${CANN_ROOT}" \
    -DCMAKE_PREFIX_PATH="${CMAKE_PACKAGE_DIR}"
cmake --build "${BUILD_DIR}" --parallel 4
echo "BUILD=PASS"

for artifact in "${BUILD_DIR}/libshape_tiling_v001.so" "${BUILD_DIR}/libshape_tiling_v001_parent.so" \
                "${BUILD_DIR}/correctness_probe" "${BUILD_DIR}/correctness_probe_parent"; do
    test -f "${artifact}"
    echo "ARTIFACT=${artifact}"
    sha256sum "${artifact}"
done

echo "=== NPU CORRECTNESS (candidate vs parent on identical cases) ==="
CASES=(
    "0 4 12288"
    "0 4 16384"
    "0 4 8192"
    "0 4 32768"
    "0 2 12288"
    "0 2 16384"
    "1 4 12288"
    "1 4 16384"
    "1 4 32768"
    "2 4 12288"
    "2 4 16384"
    "2 4 32768"
)
cand_failures=0
parent_failures=0
for case_spec in "${CASES[@]}"; do
    read -r dtype rows width <<<"${case_spec}"
    echo "--- dtype=${dtype} rows=${rows} width=${width} ---"
    cand_rc=0
    parent_rc=0
    "${BUILD_DIR}/correctness_probe" "${dtype}" "${rows}" "${width}" "${DEVICE}" 0 0 | sed 's/^/CAND /' || cand_rc=$?
    "${BUILD_DIR}/correctness_probe_parent" "${dtype}" "${rows}" "${width}" "${DEVICE}" 0 0 | sed 's/^/PARENT /' || parent_rc=$?
    echo "CAND_RC=${cand_rc} PARENT_RC=${parent_rc}"
    if [[ "${cand_rc}" -ne 0 ]]; then
        cand_failures=$((cand_failures + 1))
    fi
    if [[ "${parent_rc}" -ne 0 ]]; then
        parent_failures=$((parent_failures + 1))
    fi
done

echo "=== SUMMARY ==="
echo "CANDIDATE_FAILURES=${cand_failures}"
echo "PARENT_FAILURES=${parent_failures}"
echo "CASES_TOTAL=${#CASES[@]}"

echo "=== PARENT_VS_CANDIDATE OUTPUT DELTA (OFAT numeric safety) ==="
DUMP_DIR="${ROOT}/dumps"
mkdir -p "${DUMP_DIR}"
for case_spec in "${CASES[@]}"; do
    read -r dtype rows width <<<"${case_spec}"
    tag="d${dtype}_r${rows}_w${width}"
    "${BUILD_DIR}/correctness_probe" "${dtype}" "${rows}" "${width}" "${DEVICE}" 0 0 \
        "${DUMP_DIR}/cand_${tag}.txt" >/dev/null 2>&1 || true
    "${BUILD_DIR}/correctness_probe_parent" "${dtype}" "${rows}" "${width}" "${DEVICE}" 0 0 \
        "${DUMP_DIR}/parent_${tag}.txt" >/dev/null 2>&1 || true
    if [[ -f "${DUMP_DIR}/cand_${tag}.txt" && -f "${DUMP_DIR}/parent_${tag}.txt" ]]; then
        python3 - "${DUMP_DIR}/parent_${tag}.txt" "${DUMP_DIR}/cand_${tag}.txt" "${tag}" <<'PY'
import sys
p_path, c_path, tag = sys.argv[1], sys.argv[2], sys.argv[3]
with open(p_path) as f:
    parent = [float(x) for x in f if x.strip()]
with open(c_path) as f:
    cand = [float(x) for x in f if x.strip()]
if len(parent) != len(cand):
    print(f"DELTA {tag} LENGTH_MISMATCH parent={len(parent)} cand={len(cand)}")
    sys.exit(0)
max_abs = 0.0
max_rel = 0.0
worst = 0
diffs = 0
for i, (p, c) in enumerate(zip(parent, cand)):
    d = abs(c - p)
    if d > 0:
        diffs += 1
    if d > max_abs:
        max_abs = d
        worst = i
    denom = max(abs(p), 1e-6)
    r = d / denom
    if r > max_rel:
        max_rel = r
print(f"DELTA {tag} n={len(parent)} changed={diffs} max_abs={max_abs:.8g} max_rel={max_rel:.8g} worst_idx={worst}")
PY
    else
        echo "DELTA ${tag} DUMP_MISSING"
    fi
done

if [[ "${cand_failures}" -eq 0 ]]; then
    echo "NPU_CORRECTNESS=PASS_${#CASES[@]}_OF_${#CASES[@]}"
    exit 0
fi
if [[ "${cand_failures}" -eq "${parent_failures}" ]]; then
    echo "NPU_CORRECTNESS=COMMON_MODE_WITH_PARENT (candidate inherits parent failures; see per-case CAND_RC/PARENT_RC and PARENT_VS_CANDIDATE deltas)"
    exit 1
fi
echo "NPU_CORRECTNESS=CANDIDATE_REGRESSION"
exit 1
