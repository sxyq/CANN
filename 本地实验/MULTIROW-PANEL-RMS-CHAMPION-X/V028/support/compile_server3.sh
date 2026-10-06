#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
PROJECT_ROOT="$(cd "${ROOT}/../../../.." && pwd)"
PARENT_SOURCE="${PROJECT_ROOT}/本地实验/MULTIROW-PANEL-RMS-CHAMPION-X/V020/submission.asc"
CANDIDATE_SOURCE="${ROOT}/../submission.asc"
EXPECTED_PARENT_SHA="d9cf609280990526650e8962476f22e06beb7d8ea11e2914f86d33eb43ae96bb"
REMOTE_ROOT="/home/data4t2/lelinfeng/cann/server_runs/MULTIROW-PANEL-RMS-CHAMPION-X/V028"
RUN_ID="MPR4-V028-COMPILE-$(date -u +%Y%m%dT%H%M%SZ)"

parent_sha="$(shasum -a 256 "${PARENT_SOURCE}" | awk '{print $1}')"
candidate_sha="$(shasum -a 256 "${CANDIDATE_SOURCE}" | awk '{print $1}')"
[[ "${parent_sha}" == "${EXPECTED_PARENT_SHA}" ]]

printf 'RUN_ID=%s\nPARENT_SOURCE_SHA256=%s\nCANDIDATE_SOURCE_SHA256=%s\n' \
    "${RUN_ID}" "${parent_sha}" "${candidate_sha}"
ssh -o BatchMode=yes -o ConnectTimeout=10 cann-server3 \
    "mkdir -p '${REMOTE_ROOT}/parent' '${REMOTE_ROOT}/candidate' '${REMOTE_ROOT}/support'"
scp -p "${PARENT_SOURCE}" "cann-server3:${REMOTE_ROOT}/parent/submission.asc"
scp -p "${CANDIDATE_SOURCE}" "cann-server3:${REMOTE_ROOT}/candidate/submission.asc"
scp -p "${ROOT}/CMakeLists.txt" "${ROOT}/runner_main.cpp" "${ROOT}/local_abi_shim.h" \
    "${ROOT}/parent_kernel.asc" "${ROOT}/candidate_kernel.asc" \
    "${ROOT}/build_server3.sh" "cann-server3:${REMOTE_ROOT}/support/"

ssh -o BatchMode=yes -o ConnectTimeout=10 cann-server3 \
    "R4_PARENT_SOURCE_SHA256='${parent_sha}' R4_CANDIDATE_SOURCE_SHA256='${candidate_sha}' bash '${REMOTE_ROOT}/support/build_server3.sh' > '${REMOTE_ROOT}/${RUN_ID}.log' 2>&1; status=\$?; cat '${REMOTE_ROOT}/${RUN_ID}.log'; exit \$status"
printf 'REMOTE_BUILD_LOG=%s/%s.log\n' "${REMOTE_ROOT}" "${RUN_ID}"
