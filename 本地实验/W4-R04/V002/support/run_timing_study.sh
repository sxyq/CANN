#!/usr/bin/env bash
set -euo pipefail

R04_STUDY="${1:?study id required}"
R04_ROOT=/home/data4t2/lelinfeng/cann/server_runs/W4-R04/V002
R04_SDK=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
R04_RESULTS="${R04_ROOT}/results/timing-attribution-20261008"
R04_RAW="${R04_RESULTS}/samples-${R04_STUDY}.tsv"
R04_PROFILE="${R04_RESULTS}/profile-${R04_STUDY}"
mkdir -p "${R04_RESULTS}"
for R04_PATH in \
    "${R04_RAW}" "${R04_RAW}.reference.tsv" "${R04_PROFILE}" \
    "${R04_RESULTS}/${R04_STUDY}-start-utc.txt" \
    "${R04_RESULTS}/${R04_STUDY}-end-utc.txt" \
    "${R04_RESULTS}/${R04_STUDY}-pre-usages.txt" \
    "${R04_RESULTS}/${R04_STUDY}-post-usages.txt"; do
    test ! -e "${R04_PATH}"
done
if pgrep -x r04_timing_host >/dev/null; then
    printf 'OWNED_R04_PROCESS_ALREADY_RUNNING\n' >&2
    exit 6
fi

date -u +%Y-%m-%dT%H:%M:%SZ | tee "${R04_RESULTS}/${R04_STUDY}-start-utc.txt"
hostname
id -un
uptime
npu-smi info -t usages -i 1 | tee "${R04_RESULTS}/${R04_STUDY}-pre-usages.txt"
npu-smi info -t proc-mem -i 1
cat /proc/loadavg
R04_FREE="$(awk '/HBM Capacity/{capacity=$NF} /HBM Usage Rate/{rate=$NF} END{if(capacity=="")exit 1; printf "%d",capacity*(100-rate)/100}' "${R04_RESULTS}/${R04_STUDY}-pre-usages.txt")"
printf 'DEVICE=1 FREE_HBM_MB=%s\n' "${R04_FREE}"
test "${R04_FREE}" -ge 100

source "${R04_SDK}/set_env.sh" >/dev/null 2>&1 || true
export ASCEND_HOME_PATH="${R04_SDK}"
export LD_LIBRARY_PATH="${R04_SDK}/aarch64-linux/lib64:${R04_SDK}/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
"${R04_SDK}/tools/profiler/bin/msprof" \
    --application="${R04_ROOT}/build-server3-v002/r04_timing_host --device 1 --output ${R04_RAW}" \
    --output="${R04_PROFILE}" \
    --task-time=on --ascendcl=on --runtime-api=on --ai-core=off --aicpu=off

printf 'MSPROF_RC=0\n'
date -u +%Y-%m-%dT%H:%M:%SZ | tee "${R04_RESULTS}/${R04_STUDY}-end-utc.txt"
uptime
npu-smi info -t usages -i 1
npu-smi info -t proc-mem -i 1
cat /proc/loadavg
if pgrep -x r04_timing_host >/dev/null; then
    printf 'OWNED_R04_PROCESS_STILL_RUNNING\n' >&2
    exit 7
fi
