#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/../../../.." && pwd)"
LOCAL_SRC="${ROOT}/本地实验/W4-R08/V001"
LOCAL_LOG="${LOCAL_SRC}/logs"
REMOTE_HOST="cann-server3"
REMOTE_ROOT="/home/data4t2/lelinfeng/w4-r08/V001"
MODE="$1"; shift
SHAPE="$1"; shift
ROWS="${SHAPE%%x*}"; WIDTH="${SHAPE##*x}"
DEVICE="$1"; shift
IMPL=""
SHAPE_INDEX=""
QUAL_FILE=""
WARMUP=45
SAMPLES=31
BLOCKS=2
PAIRS=21
CPU_AFFINITY="${W4R08_CPU_AFFINITY:-}"
while (($#)); do
    case "$1" in
        --implementation) IMPL="$2"; shift 2 ;;
        --qualification-file) QUAL_FILE="$2"; shift 2 ;;
        --warmup) WARMUP="$2"; shift 2 ;;
        --samples) SAMPLES="$2"; shift 2 ;;
        --blocks) BLOCKS="$2"; shift 2 ;;
        --pairs) PAIRS="$2"; shift 2 ;;
        *) printf 'unknown argument: %s\n' "$1" >&2; exit 2 ;;
    esac
done

OWNER="W4-R08"
LEASE_ID="W4-R08-$(date -u +%Y%m%dT%H%M%SZ)-$$"
RUN_ID="local-${MODE}-$(printf '%s' "${SHAPE}" | tr 'x' '-')-$(date -u +%Y%m%dT%H%M%SZ)"
REMOTE_PREFIX="${REMOTE_ROOT}/logs/${RUN_ID}"
REMOTE_BINARY="${REMOTE_ROOT}/build/w4r08_paired_runner"
LOCAL_PREFIX="${LOCAL_LOG}/${RUN_ID}"

RUNNER_SHA="$(ssh "${REMOTE_HOST}" "sha256sum '${REMOTE_BINARY}'" | awk '{print $1}' | tr -d ' ')"
if [[ ! "${RUNNER_SHA}" =~ ^[0-9a-f]{64}$ ]]; then
    printf 'paired runner binary is missing or has no SHA-256 identity\n' >&2
    exit 2
fi

{
    printf 'route=W4-R08\nrevision=V001\nmode=%s\n' "${MODE}"
    printf 'device=%s\nshape=%s\nimplementation=%s\n' "${DEVICE}" "${SHAPE}" "${IMPL}"
    printf 'runner_binary=%s\nrunner_sha256=%s\n' "${REMOTE_BINARY}" "${RUNNER_SHA}"
    printf 'remote_preflight_begin=\n'
    ssh "${REMOTE_HOST}" "printf 'preflight_utc='; date -u +%Y-%m-%dT%H:%M:%SZ; npu-smi info -t usages -i '${DEVICE}'; printf '%s\\n' '--- device processes ---'; npu-smi info 2>/dev/null | sed -n '/Process id/,\$p' | sed -n '/ ${DEVICE} /,+1p'; printf '%s\\n' '--- host load ---'; uptime; printf '%s\\n' '--- acl processes ---'; pgrep -af 'VLLM|EngineCor|w4r08' || true"
} > "${LOCAL_PREFIX}-preflight.txt"

PREFLIGHT_UTC="$(awk -F= '/^preflight_utc=/{print $2; exit}' "${LOCAL_PREFIX}-preflight.txt")"
REMOTE_QUAL=""
if [[ "${MODE}" == "paired" ]]; then
    REMOTE_QUAL="${REMOTE_ROOT}/logs/${RUN_ID}-qualification.txt"
    scp "${QUAL_FILE}" "${REMOTE_HOST}:${REMOTE_QUAL}"
fi

ARGS=(--mode "${MODE}" --device "${DEVICE}" --rows "${ROWS}" --width "${WIDTH}"
      --dtype 0 --owner "${OWNER}" --lease-id "${LEASE_ID}"
      --preflight-utc "${PREFLIGHT_UTC}" --runner-sha256 "${RUNNER_SHA}"
      --out-prefix "${REMOTE_PREFIX}" --warmup "${WARMUP}" --samples "${SAMPLES}")
if [[ "${MODE}" == "noise-floor" ]]; then
    ARGS+=(--implementation "${IMPL}" --blocks "${BLOCKS}")
else
    ARGS+=(--pairs "${PAIRS}" --qualification-file "${REMOTE_QUAL}")
fi

printf '%q ' "${ARGS[@]}" > "${LOCAL_PREFIX}-command.txt"
printf '\n' >> "${LOCAL_PREFIX}-command.txt"

printf -v REMOTE_COMMAND '%q ' "${REMOTE_BINARY}" "${ARGS[@]}"
AFFINITY_PREFIX=""
if [[ -n "${CPU_AFFINITY}" ]]; then
    AFFINITY_PREFIX="taskset -c ${CPU_AFFINITY} "
    printf 'cpu_affinity=%s\n' "${CPU_AFFINITY}" >> "${LOCAL_PREFIX}.log"
fi
set +e
ssh "${REMOTE_HOST}" "source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh && export ASCEND_OPP_PATH=\$ASCEND_HOME_PATH/opp && export LD_LIBRARY_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:/usr/lib/aarch64-linux-gnu && exec ${AFFINITY_PREFIX}${REMOTE_COMMAND}" 2>&1 | tee "${LOCAL_PREFIX}.log"
RUN_STATUS=${PIPESTATUS[0]}
set -e

for suffix in raw.tsv jitter.txt pairs.tsv; do
    if ssh "${REMOTE_HOST}" "test -f '${REMOTE_PREFIX}.${suffix}'"; then
        scp "${REMOTE_HOST}:${REMOTE_PREFIX}.${suffix}" "${LOCAL_PREFIX}.${suffix}"
    fi
done
{
    ssh "${REMOTE_HOST}" "printf 'postflight_utc='; date -u +%Y-%m-%dT%H:%M:%SZ; npu-smi info -t usages -i '${DEVICE}'; printf '%s\\n' '--- device processes ---'; npu-smi info 2>/dev/null | sed -n '/Process id/,\$p' | sed -n '/ ${DEVICE} /,+1p'; uptime"
} > "${LOCAL_PREFIX}-postflight.txt"
printf 'runner_exit_status=%s\nowner=%s\nlease_id=%s\npreflight_utc=%s\n' \
    "${RUN_STATUS}" "${OWNER}" "${LEASE_ID}" "${PREFLIGHT_UTC}" >> "${LOCAL_PREFIX}.log"

if [[ "${MODE}" == "noise-floor" && "${IMPL}" == "parent" ]]; then
    {
        printf 'route=W4-R08\nrevision=V001\nmode=noise-floor\nimplementation=parent\n'
        printf 'shape=%s\ndtype=fp32\ndevice=%s\nowner=%s\nlease_id=%s\n' \
            "${SHAPE}" "${DEVICE}" "${OWNER}" "${LEASE_ID}"
        printf 'preflight_utc=%s\nrunner_sha256=%s\n' "${PREFLIGHT_UTC}" "${RUNNER_SHA}"
        printf 'parent_source_sha256=%s\n' "a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3"
        printf 'candidate_source_sha256=%s\n' "dc2cd6b8e24acaa8a56774d2c3fec93a525ee8540d46e35b145dc09ef55333fb"
        grep -E '^(qualification|block_median_drift)=' "${LOCAL_PREFIX}.jitter.txt" 2>/dev/null || true
    } > "${LOCAL_PREFIX}-qualification.txt"
    printf 'QUALIFICATION_FILE=%s\n' "${LOCAL_PREFIX}-qualification.txt"
fi

printf 'RUNNER_EXIT=%s\nRUN_ID=%s\nLOCAL_PREFIX=%s\n' "${RUN_STATUS}" "${RUN_ID}" "${LOCAL_PREFIX}"
exit "${RUN_STATUS}"
