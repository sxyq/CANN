#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
LOCAL_SRC="${ROOT}/本地实验/W4-R08/V001"
LOCAL_LOG="${LOCAL_SRC}/logs"
REMOTE_HOST="cann-server3"
REMOTE_ROOT="/home/data4t2/lelinfeng/w4-r08/V001"
DEVICE="${1:-3}"
ATTEMPT_ID="correctness-$(date -u +%Y%m%dT%H%M%SZ)"

mkdir -p "${LOCAL_LOG}"
scp "${LOCAL_SRC}/support/npu_correctness.cpp" "${REMOTE_HOST}:${REMOTE_ROOT}/src/npu_correctness.cpp"

ssh "${REMOTE_HOST}" bash -s -- "${REMOTE_ROOT}" "${ATTEMPT_ID}" "${DEVICE}" <<'REMOTE_RUN'
set +e
remote_root="$1"
attempt_id="$2"
device="$3"
source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh
env_status=$?
set -u
npu_build=NOT_RUN
correctness=NOT_RUN
free_hbm=UNKNOWN
if test "$env_status" -eq 0 && test -n "${ASCEND_HOME_PATH:-}"; then
    export ASCEND_OPP_PATH="$ASCEND_HOME_PATH/opp"
    export CPLUS_INCLUDE_PATH="/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward:/usr/include/aarch64-linux-gnu${CPLUS_INCLUDE_PATH:+:$CPLUS_INCLUDE_PATH}"
    export C_INCLUDE_PATH="/usr/include:/usr/include/aarch64-linux-gnu${C_INCLUDE_PATH:+:$C_INCLUDE_PATH}"
    export LD_LIBRARY_PATH="/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:/usr/lib/aarch64-linux-gnu"
    cd "$remote_root" || exit 0
    free_hbm="$(npu-smi info -t usages -i "$device" 2>/dev/null | awk '/HBM Capacity\(MB\)/{c=$NF} /^[[:space:]]*HBM Usage Rate\(%\)/{r=$NF} END{ if(c==""||r=="") print "UNKNOWN"; else printf "%d", c*(100-r)/100 }')"
    npu-smi info -t usages -i "$device" >"logs/${attempt_id}-preflight-usages.txt" 2>&1
    npu-smi info >"logs/${attempt_id}-preflight-npu-smi.txt" 2>&1
    printf 'preflight_utc=%s\ndevice=%s\nfree_hbm_mb=%s\n' "$(date -u +%Y-%m-%dT%H:%M:%SZ)" "$device" "$free_hbm" >"logs/${attempt_id}-preflight.txt"
    cmake -S src -B build -DCMAKE_BUILD_TYPE=Release >"logs/${attempt_id}-configure.log" 2>&1
    if test $? -eq 0; then
        cmake --build build --target w4r08_v001_npu_correctness --verbose -j2 >"logs/${attempt_id}-npu-build.log" 2>&1
        npu_build=$?
    fi
    if test "$npu_build" -eq 0 && test -x build/w4r08_v001_npu_correctness; then
        sha256sum build/w4r08_v001_npu_correctness >"logs/${attempt_id}-binary.sha256"
        ./build/w4r08_v001_npu_correctness "$device" >"logs/${attempt_id}-correctness.log" 2>&1
        correctness=$?
    fi
    npu-smi info -t usages -i "$device" >"logs/${attempt_id}-postflight-usages.txt" 2>&1
fi
printf 'ENV_SOURCE=%s\nDEVICE=%s\nFREE_HBM_MB=%s\nNPU_BUILD=%s\nCORRECTNESS_RC=%s\n' \
    "$env_status" "$device" "$free_hbm" "$npu_build" "$correctness" >"logs/${attempt_id}-status.txt"
exit 0
REMOTE_RUN

for suffix in preflight.txt preflight-usages.txt preflight-npu-smi.txt configure.log \
    npu-build.log correctness.log binary.sha256 postflight-usages.txt status.txt; do
    name="${ATTEMPT_ID}-${suffix}"
    if ssh "${REMOTE_HOST}" "test -f '${REMOTE_ROOT}/logs/${name}'"; then
        scp "${REMOTE_HOST}:${REMOTE_ROOT}/logs/${name}" "${LOCAL_LOG}/${name}"
    fi
done

cat "${LOCAL_LOG}/${ATTEMPT_ID}-status.txt"
printf '%s\n' '--- correctness tail:'
tail -6 "${LOCAL_LOG}/${ATTEMPT_ID}-correctness.log" 2>/dev/null || true
printf 'ATTEMPT_ID=%s\n' "${ATTEMPT_ID}"
