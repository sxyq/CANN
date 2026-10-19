#!/usr/bin/env bash
set -euo pipefail

R04_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
R04_MODE="${1:?stage required}"
R04_SDK=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
R04_DEVICE=3
R04_STAMP="$(date -u +%Y%m%dT%H%M%SZ)-$$"
R04_LOG="${R04_ROOT}/logs/${R04_MODE}-${R04_STAMP}.log"
R04_RAW="${R04_ROOT}/logs/${R04_MODE}-${R04_STAMP}-raw.tsv"
R04_PROBE="${R04_ROOT}/build-server3-v002/r04_probe"

case "${R04_MODE}" in
    compile|correctness|same-binary|local) ;;
    *) printf 'Unknown stage\n' >&2; exit 2 ;;
esac
mkdir -p "${R04_ROOT}/logs"
test ! -e "${R04_LOG}"
source "${R04_SDK}/set_env.sh" >/dev/null 2>&1 || true
export ASCEND_HOME_PATH="${R04_SDK}"
export LD_LIBRARY_PATH="${R04_SDK}/lib64:${R04_SDK}/aarch64-linux/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common${LD_LIBRARY_PATH:+:${LD_LIBRARY_PATH}}"
exec > >(tee "${R04_LOG}") 2>&1

r04_context() {
    printf 'CONTEXT=%s\n' "$1"
    date -u +%Y-%m-%dT%H:%M:%SZ
    hostname
    id -un
    uptime
    R04_USAGE="$(npu-smi info -t usages -i "${R04_DEVICE}")"
    printf '%s\n' "${R04_USAGE}"
    R04_FREE_HBM_MB="$(printf '%s\n' "${R04_USAGE}" | awk '
        /HBM Capacity\(MB\)/ { capacity = $NF }
        /^[[:space:]]*HBM Usage Rate\(%\)/ { rate = $NF }
        END {
            if (capacity == "" || rate == "") exit 1
            printf "%d", capacity * (100 - rate) / 100
        }')"
    printf 'FREE_HBM_MB=%s\n' "${R04_FREE_HBM_MB}"
    npu-smi info -t proc-mem -i "${R04_DEVICE}" || true
}

printf 'ROUTE=W4-R04 REVISION=V002 STAGE=%s DEVICE=3\n' "${R04_MODE}"
printf 'DIRECT_PARENT=R31B-V011 ONLINE=PAUSED PUSH=NO\n'
r04_context before
if [ "${R04_FREE_HBM_MB}" -lt 100 ]; then
    printf 'RESOURCE_RESULT=FREE_HBM_BELOW_100_MB\n'
    exit 3
fi

if [ "${R04_MODE}" != compile ]; then
    test -f "${R04_ROOT}/compile.passed"
fi
if [ "${R04_MODE}" = same-binary ] || [ "${R04_MODE}" = local ]; then
    test -f "${R04_ROOT}/correctness.passed"
fi
if [ "${R04_MODE}" = local ]; then
    test -f "${R04_ROOT}/same-binary.passed"
fi

set +e
if [ "${R04_MODE}" = compile ]; then
    bash "${R04_ROOT}/support/build_on_server3.sh"
elif [ "${R04_MODE}" = correctness ]; then
    "${R04_PROBE}" --mode correctness --device "${R04_DEVICE}"
else
    "${R04_PROBE}" --mode "${R04_MODE}" --device "${R04_DEVICE}" --output "${R04_RAW}"
fi
R04_RC=$?
set -e
if [ "${R04_RC}" -eq 0 ]; then
    touch "${R04_ROOT}/${R04_MODE}.passed"
fi
r04_context after
printf 'STAGE=%s EXIT_CODE=%s LOG=%s RAW=%s\n' "${R04_MODE}" "${R04_RC}" "${R04_LOG}" "${R04_RAW}"
exit "${R04_RC}"
