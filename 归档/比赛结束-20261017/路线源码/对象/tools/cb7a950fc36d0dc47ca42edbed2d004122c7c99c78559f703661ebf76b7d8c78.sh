#!/usr/bin/env bash
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
WORKSPACE="${ROOT}/phase4/workspaces/DTYPE-SPECIAL-X/V001"
LOCAL_LOG="${ROOT}/phase4/local/DTYPE-SPECIAL-X/V001/logs"
REMOTE_HOST="cann-server3"
REMOTE_ROOT="/home/data4t2/lelinfeng/phase4-workspaces/DTYPE-SPECIAL-X/V001"
ATTEMPT_ID="unified-build-$(date -u +%Y%m%dT%H%M%SZ)-$$"
PARENT_SHA="a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3"
CANDIDATE_SHA="e2717055199f541d98d85ceef2e011e52e2749d932ca954dc887868de7db880f"
PARENT_ADAPTER_SHA="$(shasum -a 256 "${WORKSPACE}/support/compile_adapter_parent.asc" | awk '{print $1}')"
CANDIDATE_ADAPTER_SHA="$(shasum -a 256 "${WORKSPACE}/support/compile_adapter.asc" | awk '{print $1}')"
RUNNER_SHA="$(shasum -a 256 "${WORKSPACE}/support/unified_runner.asc" | awk '{print $1}')"
CMAKE_SHA="$(shasum -a 256 "${WORKSPACE}/support/CMakeLists.txt" | awk '{print $1}')"

mkdir -p "${LOCAL_LOG}"
test "$(shasum -a 256 "${WORKSPACE}/support/parent_submission.asc" | awk '{print $1}')" = "${PARENT_SHA}"
test "$(shasum -a 256 "${WORKSPACE}/submission.asc" | awk '{print $1}')" = "${CANDIDATE_SHA}"

{
    printf 'route=DTYPE-SPECIAL-X\nrevision=V001\n'
    printf 'branch=%s\n' "$(git -C "${ROOT}" branch --show-current)"
    printf 'source_commit=%s\n' "$(git -C "${ROOT}" rev-parse HEAD)"
    printf 'parent_source_path=phase4/workspaces/DTYPE-SPECIAL-X/V001/support/parent_submission.asc\n'
    printf 'parent_source_sha256=%s\n' "${PARENT_SHA}"
    printf 'candidate_source_path=phase4/workspaces/DTYPE-SPECIAL-X/V001/submission.asc\n'
    printf 'candidate_source_sha256=%s\n' "${CANDIDATE_SHA}"
    printf 'runner_source_path=phase4/workspaces/DTYPE-SPECIAL-X/V001/support/unified_runner.asc\n'
    printf 'runner_source_sha256=%s\n' "${RUNNER_SHA}"
    printf 'parent_compile_adapter_path=phase4/workspaces/DTYPE-SPECIAL-X/V001/support/compile_adapter_parent.asc\n'
    printf 'parent_compile_adapter_sha256=%s\n' "${PARENT_ADAPTER_SHA}"
    printf 'candidate_compile_adapter_path=phase4/workspaces/DTYPE-SPECIAL-X/V001/support/compile_adapter.asc\n'
    printf 'candidate_compile_adapter_sha256=%s\n' "${CANDIDATE_ADAPTER_SHA}"
    printf 'cmake_source_path=phase4/workspaces/DTYPE-SPECIAL-X/V001/support/CMakeLists.txt\n'
    printf 'cmake_source_sha256=%s\n' "${CMAKE_SHA}"
} > "${LOCAL_LOG}/${ATTEMPT_ID}-source-identity.txt"

ssh "${REMOTE_HOST}" "mkdir -p '${REMOTE_ROOT}/src/support' '${REMOTE_ROOT}/build' '${REMOTE_ROOT}/logs'"
REMOTE_CANDIDATE_SHA="$(ssh "${REMOTE_HOST}" "if test -f '${REMOTE_ROOT}/src/submission.asc'; then sha256sum '${REMOTE_ROOT}/src/submission.asc' | awk '{print \$1}'; else printf MISSING; fi")"
if [[ "${REMOTE_CANDIDATE_SHA}" != "${CANDIDATE_SHA}" && "${REMOTE_CANDIDATE_SHA}" != MISSING ]]; then
    printf 'remote Candidate source differs from V001; refusing to overwrite it: %s\n' \
        "${REMOTE_CANDIDATE_SHA}" >&2
    exit 2
fi
if [[ "${REMOTE_CANDIDATE_SHA}" == MISSING ]]; then
    scp "${WORKSPACE}/submission.asc" "${REMOTE_HOST}:${REMOTE_ROOT}/src/submission.asc"
fi
REMOTE_PARENT_SHA="$(ssh "${REMOTE_HOST}" "if test -f '${REMOTE_ROOT}/src/support/parent_submission.asc'; then sha256sum '${REMOTE_ROOT}/src/support/parent_submission.asc' | awk '{print \$1}'; else printf MISSING; fi")"
if [[ "${REMOTE_PARENT_SHA}" != "${PARENT_SHA}" && "${REMOTE_PARENT_SHA}" != MISSING ]]; then
    printf 'remote Parent source differs from the declared Direct Parent; refusing to overwrite it: %s\n' \
        "${REMOTE_PARENT_SHA}" >&2
    exit 2
fi
if [[ "${REMOTE_PARENT_SHA}" == MISSING ]]; then
    scp "${WORKSPACE}/support/parent_submission.asc" "${REMOTE_HOST}:${REMOTE_ROOT}/src/support/parent_submission.asc"
fi
scp "${WORKSPACE}/support/unified_runner.asc" "${REMOTE_HOST}:${REMOTE_ROOT}/src/support/unified_runner.asc"
scp "${WORKSPACE}/support/compile_adapter.asc" "${REMOTE_HOST}:${REMOTE_ROOT}/src/compile_adapter.asc"
scp "${WORKSPACE}/support/compile_adapter_parent.asc" "${REMOTE_HOST}:${REMOTE_ROOT}/src/support/compile_adapter_parent.asc"
scp "${WORKSPACE}/support/CMakeLists.txt" "${REMOTE_HOST}:${REMOTE_ROOT}/src/CMakeLists.txt"

ssh "${REMOTE_HOST}" bash -s -- "${REMOTE_ROOT}" "${PARENT_SHA}" "${CANDIDATE_SHA}" \
    "${PARENT_ADAPTER_SHA}" "${CANDIDATE_ADAPTER_SHA}" "${RUNNER_SHA}" "${CMAKE_SHA}" <<'VERIFY_SOURCES'
set -euo pipefail
remote_root="$1"
shift
cd "$remote_root"
verify_source() {
    local path="$1"
    local expected="$2"
    local actual
    actual="$(sha256sum "$path" | awk '{print $1}')"
    if [[ "$actual" != "$expected" ]]; then
        printf 'source identity mismatch: %s expected=%s actual=%s\n' \
            "$path" "$expected" "$actual" >&2
        exit 2
    fi
}
verify_source src/support/parent_submission.asc "$1"
verify_source src/submission.asc "$2"
verify_source src/support/compile_adapter_parent.asc "$3"
verify_source src/compile_adapter.asc "$4"
verify_source src/support/unified_runner.asc "$5"
verify_source src/CMakeLists.txt "$6"
VERIFY_SOURCES

ssh "${REMOTE_HOST}" bash -s -- "${ATTEMPT_ID}" "${REMOTE_ROOT}" <<'REMOTE_BUILD'
set +e
attempt_id="$1"
remote_root="$2"
source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh
env_status=$?
set -u
cfg=NOT_RUN
build=NOT_RUN
if test "$env_status" -eq 0 && test -n "${ASCEND_HOME_PATH:-}"; then
    export ASCEND_OPP_PATH="$ASCEND_HOME_PATH/opp"
    export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward:/usr/include/aarch64-linux-gnu${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"
    export C_INCLUDE_PATH="/usr/include:/usr/include/aarch64-linux-gnu${C_INCLUDE_PATH:+:$C_INCLUDE_PATH}"
    cd "$remote_root"
    sha256sum src/submission.asc src/compile_adapter.asc src/support/parent_submission.asc src/support/compile_adapter_parent.asc src/support/unified_runner.asc src/CMakeLists.txt >"logs/${attempt_id}-source.sha256"
    cmake -S src -B build -DCMAKE_BUILD_TYPE=Release >"logs/${attempt_id}-configure.log" 2>&1
    cfg=$?
    if test "$cfg" -eq 0; then
        cmake --build build --target dtype_special_x_v001_parent dtype_special_x_v001 dtype_special_x_v001_unified_runner --verbose -j2 >"logs/${attempt_id}-build-link.log" 2>&1
        build=$?
    fi
    for target in dtype_special_x_v001_parent dtype_special_x_v001 dtype_special_x_v001_unified_runner; do
        if test -x "build/$target"; then
            sha256sum "build/$target" >"logs/${attempt_id}-${target}.sha256"
            ls -l "build/$target" >"logs/${attempt_id}-${target}.stat"
        fi
    done
fi
printf 'ENV_SOURCE=%s\nASCEND_HOME_PATH=%s\nCONFIGURE=%s\nALL_BUILD_LINK=%s\nRUNNER_EXECUTED=NO\n' \
    "$env_status" "${ASCEND_HOME_PATH:-UNSET}" "$cfg" "$build" >"$remote_root/logs/${attempt_id}-status.txt"
exit 0
REMOTE_BUILD

for suffix in source.sha256 configure.log build-link.log status.txt \
    dtype_special_x_v001_parent.sha256 dtype_special_x_v001_parent.stat \
    dtype_special_x_v001.sha256 dtype_special_x_v001.stat \
    dtype_special_x_v001_unified_runner.sha256 dtype_special_x_v001_unified_runner.stat; do
    name="${ATTEMPT_ID}-${suffix}"
    if ssh "${REMOTE_HOST}" "test -f '${REMOTE_ROOT}/logs/${name}'"; then
        scp "${REMOTE_HOST}:${REMOTE_ROOT}/logs/${name}" "${LOCAL_LOG}/unified-${name}"
    fi
done

cat "${LOCAL_LOG}/unified-${ATTEMPT_ID}-status.txt"
if ! rg -q '^ALL_BUILD_LINK=0$' "${LOCAL_LOG}/unified-${ATTEMPT_ID}-status.txt"; then
    exit 1
fi
PARENT_EXE_SHA="$(awk '{print $1}' "${LOCAL_LOG}/unified-${ATTEMPT_ID}-dtype_special_x_v001_parent.sha256")"
CANDIDATE_EXE_SHA="$(awk '{print $1}' "${LOCAL_LOG}/unified-${ATTEMPT_ID}-dtype_special_x_v001.sha256")"
RUNNER_EXE_SHA="$(awk '{print $1}' "${LOCAL_LOG}/unified-${ATTEMPT_ID}-dtype_special_x_v001_unified_runner.sha256")"
IDENTITY_PATH="${LOCAL_LOG}/unified-${ATTEMPT_ID}-build-identity.txt"
{
    printf 'route=DTYPE-SPECIAL-X\nrevision=V001\n'
    printf 'parent_source_path=%s/support/parent_submission.asc\n' "${WORKSPACE}"
    printf 'parent_source_sha256=%s\n' "${PARENT_SHA}"
    printf 'parent_executable_path=%s/build/dtype_special_x_v001_parent\n' "${REMOTE_ROOT}"
    printf 'parent_executable_sha256=%s\n' "${PARENT_EXE_SHA}"
    printf 'candidate_source_path=%s/submission.asc\n' "${WORKSPACE}"
    printf 'candidate_source_sha256=%s\n' "${CANDIDATE_SHA}"
    printf 'candidate_compile_adapter_path=%s/support/compile_adapter.asc\n' "${WORKSPACE}"
    printf 'candidate_compile_adapter_sha256=%s\n' "${CANDIDATE_ADAPTER_SHA}"
    printf 'candidate_executable_path=%s/build/dtype_special_x_v001\n' "${REMOTE_ROOT}"
    printf 'candidate_executable_sha256=%s\n' "${CANDIDATE_EXE_SHA}"
    printf 'parent_compile_adapter_path=%s/support/compile_adapter_parent.asc\n' "${WORKSPACE}"
    printf 'parent_compile_adapter_sha256=%s\n' "${PARENT_ADAPTER_SHA}"
    printf 'runner_source_sha256=%s\n' "${RUNNER_SHA}"
    printf 'cmake_source_sha256=%s\n' "${CMAKE_SHA}"
    printf 'paired_runner_source_path=%s/support/unified_runner.asc\n' "${WORKSPACE}"
    printf 'paired_runner_executable_path=%s/build/dtype_special_x_v001_unified_runner\n' "${REMOTE_ROOT}"
    printf 'paired_runner_executable_sha256=%s\n' "${RUNNER_EXE_SHA}"
    printf 'compile_link_log=%s/unified-%s-build-link.log\n' "${LOCAL_LOG}" "${ATTEMPT_ID}"
    printf 'remote_source_sha_log=%s/unified-%s-source.sha256\n' "${LOCAL_LOG}" "${ATTEMPT_ID}"
} > "${IDENTITY_PATH}"
printf 'ATTEMPT_ID=%s\nBUILD_IDENTITY=%s\n' "${ATTEMPT_ID}" "${IDENTITY_PATH}"
