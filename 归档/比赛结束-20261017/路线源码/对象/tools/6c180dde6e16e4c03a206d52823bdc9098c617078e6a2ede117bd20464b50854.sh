#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
LOCAL_SRC="${ROOT}/本地实验/W4-R08/V001"
LOCAL_LOG="${LOCAL_SRC}/logs"
REMOTE_HOST="cann-server3"
REMOTE_ROOT="/home/data4t2/lelinfeng/w4-r08/V001"
ATTEMPT_ID="paired-build-$(date -u +%Y%m%dT%H%M%SZ)"
PARENT_SHA="a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3"
CANDIDATE_SHA="$(shasum -a 256 "${LOCAL_SRC}/submission.asc" | awk '{print $1}')"

mkdir -p "${LOCAL_LOG}"
test "$(shasum -a 256 "${LOCAL_SRC}/support/parent_submission.asc" | awk '{print $1}')" = "${PARENT_SHA}"

ssh "${REMOTE_HOST}" "mkdir -p '${REMOTE_ROOT}/src' '${REMOTE_ROOT}/build' '${REMOTE_ROOT}/logs'"
scp "${LOCAL_SRC}/submission.asc" "${REMOTE_HOST}:${REMOTE_ROOT}/src/submission.asc"
scp "${LOCAL_SRC}/support/parent_submission.asc" "${REMOTE_HOST}:${REMOTE_ROOT}/src/parent_submission.asc"
scp "${LOCAL_SRC}/support/paired_runner.asc" "${REMOTE_HOST}:${REMOTE_ROOT}/src/paired_runner.asc"
scp "${LOCAL_SRC}/support/qualification_validation.hpp" "${REMOTE_HOST}:${REMOTE_ROOT}/src/qualification_validation.hpp"
scp "${LOCAL_SRC}/support/CMakeLists.txt" "${REMOTE_HOST}:${REMOTE_ROOT}/src/CMakeLists.txt"

ssh "${REMOTE_HOST}" bash -s -- "${REMOTE_ROOT}" "${ATTEMPT_ID}" "${PARENT_SHA}" "${CANDIDATE_SHA}" <<'REMOTE_BUILD'
set +e
remote_root="$1"
attempt_id="$2"
parent_sha="$3"
candidate_sha="$4"
source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh
env_status=$?
set -u
cfg=NOT_RUN
build=NOT_RUN
identity=NOT_RUN
if test "$env_status" -eq 0 && test -n "${ASCEND_HOME_PATH:-}"; then
    export ASCEND_OPP_PATH="$ASCEND_HOME_PATH/opp"
    export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward:/usr/include/aarch64-linux-gnu${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"
    export C_INCLUDE_PATH="/usr/include:/usr/include/aarch64-linux-gnu${C_INCLUDE_PATH:+:$C_INCLUDE_PATH}"
    export LD_LIBRARY_PATH="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:/usr/lib/aarch64-linux-gnu"
    cd "$remote_root" || exit 0
    actual_parent="$(sha256sum src/parent_submission.asc | awk '{print $1}')"
    actual_candidate="$(sha256sum src/submission.asc | awk '{print $1}')"
    if test "$actual_parent" = "$parent_sha" && test "$actual_candidate" = "$candidate_sha"; then identity=PASS; else identity=FAIL; fi
    {
        printf 'parent_source_sha256_expected=%s\n' "$parent_sha"
        printf 'parent_source_sha256_actual=%s\n' "$actual_parent"
        printf 'candidate_source_sha256_expected=%s\n' "$candidate_sha"
        printf 'candidate_source_sha256_actual=%s\n' "$actual_candidate"
        printf 'source_identity=%s\n' "$identity"
        sha256sum src/paired_runner.asc src/qualification_validation.hpp src/CMakeLists.txt
    } >"logs/${attempt_id}-source.sha256"
    cmake -S src -B build -DCMAKE_BUILD_TYPE=Release >"logs/${attempt_id}-configure.log" 2>&1
    cfg=$?
    if test "$cfg" -eq 0; then
        cmake --build build --target w4r08_paired_runner --verbose -j2 >"logs/${attempt_id}-build-link.log" 2>&1
        build=$?
    fi
    if test -x build/w4r08_paired_runner; then
        sha256sum build/w4r08_paired_runner >"logs/${attempt_id}-binary.sha256"
        ls -l build/w4r08_paired_runner >"logs/${attempt_id}-binary.stat"
    fi
fi
printf 'ENV_SOURCE=%s\nASCEND_HOME_PATH=%s\nSOURCE_IDENTITY=%s\nCONFIGURE=%s\nBUILD_LINK=%s\n' \
    "$env_status" "${ASCEND_HOME_PATH:-UNSET}" "$identity" "$cfg" "$build" \
    >"logs/${attempt_id}-status.txt"
exit 0
REMOTE_BUILD

for suffix in source.sha256 configure.log build-link.log binary.sha256 binary.stat status.txt; do
    name="${ATTEMPT_ID}-${suffix}"
    if ssh "${REMOTE_HOST}" "test -f '${REMOTE_ROOT}/logs/${name}'"; then
        scp "${REMOTE_HOST}:${REMOTE_ROOT}/logs/${name}" "${LOCAL_LOG}/${name}"
    fi
done

cat "${LOCAL_LOG}/${ATTEMPT_ID}-status.txt"
cat "${LOCAL_LOG}/${ATTEMPT_ID}-source.sha256"
printf 'ATTEMPT_ID=%s\n' "${ATTEMPT_ID}"
