#!/usr/bin/env bash
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
LOCAL_SRC="${ROOT}/phase4/workspaces/DTYPE-SPECIAL-X/V001"
LOCAL_LOG="${ROOT}/phase4/local/DTYPE-SPECIAL-X/V001/logs"
REMOTE_HOST="cann-server3"
REMOTE_ROOT="/home/data4t2/lelinfeng/phase4-workspaces/DTYPE-SPECIAL-X/V001"
ATTEMPT_ID="source-env-$(date -u +%Y%m%dT%H%M%SZ)"

mkdir -p "${LOCAL_LOG}"
ssh "${REMOTE_HOST}" "mkdir -p '${REMOTE_ROOT}/src' '${REMOTE_ROOT}/build' '${REMOTE_ROOT}/logs'"
scp "${LOCAL_SRC}/submission.asc" "${REMOTE_HOST}:${REMOTE_ROOT}/src/submission.asc"
scp "${LOCAL_SRC}/support/compile_adapter.asc" "${REMOTE_HOST}:${REMOTE_ROOT}/src/compile_adapter.asc"
scp "${LOCAL_SRC}/support/CMakeLists.txt" "${REMOTE_HOST}:${REMOTE_ROOT}/src/CMakeLists.txt"

ssh "${REMOTE_HOST}" "source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh; env_status=\$?; set -u; cfg=NOT_RUN; build=NOT_RUN; if test \$env_status -eq 0 && test -n \"\${ASCEND_HOME_PATH:-}\"; then export ASCEND_OPP_PATH=\$ASCEND_HOME_PATH/opp; cd '${REMOTE_ROOT}'; sha256sum src/submission.asc >logs/${ATTEMPT_ID}-source.sha256; cmake -S src -B build -DCMAKE_BUILD_TYPE=Release >logs/${ATTEMPT_ID}-configure.log 2>&1; cfg=\$?; if test \$cfg -eq 0; then cmake --build build --target dtype_special_x_v001 --verbose -j2 >logs/${ATTEMPT_ID}-build-link.log 2>&1; build=\$?; fi; if test -x build/dtype_special_x_v001; then sha256sum build/dtype_special_x_v001 >logs/${ATTEMPT_ID}-binary.sha256; ls -l build/dtype_special_x_v001 >logs/${ATTEMPT_ID}-binary.stat; fi; fi; printf 'ENV_SOURCE=%s\\nASCEND_HOME_PATH=%s\\nCONFIGURE=%s\\nBUILD_LINK=%s\\n' \"\$env_status\" \"\${ASCEND_HOME_PATH:-UNSET}\" \"\$cfg\" \"\$build\" >logs/${ATTEMPT_ID}-status.txt; exit 0"

for suffix in source.sha256 configure.log build-link.log binary.sha256 binary.stat status.txt; do
    name="${ATTEMPT_ID}-${suffix}"
    if ssh "${REMOTE_HOST}" "test -f '${REMOTE_ROOT}/logs/${name}'"; then
        scp "${REMOTE_HOST}:${REMOTE_ROOT}/logs/${name}" "${LOCAL_LOG}/server3-${name}"
    fi
done
cat "${LOCAL_LOG}/server3-${ATTEMPT_ID}-status.txt"
printf 'REMOTE_ROOT=%s\n' "${REMOTE_ROOT}"
