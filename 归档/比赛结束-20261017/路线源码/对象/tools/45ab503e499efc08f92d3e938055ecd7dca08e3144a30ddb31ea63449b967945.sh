#!/usr/bin/env bash
set -euo pipefail

ROOT="$(git rev-parse --show-toplevel)"
LOCAL_LOG="${ROOT}/phase4/local/DTYPE-SPECIAL-X/V001/logs"
LEASE_LEDGER="/Users/sunyiyang/Desktop/Project/cann/phase4/control/server3-device-leases.tsv"
REMOTE_HOST="cann-server3"
REMOTE_ROOT="/home/data4t2/lelinfeng/phase4-workspaces/DTYPE-SPECIAL-X/V001"
MODE=""
DEVICE=""
LEASE_ID=""
OWNER=""
IMPLEMENTATION=""
SHAPE_INDEX=""
WARMUP=10
SAMPLES=31
BLOCKS=2
PAIRS=4
QUALIFICATION_FILE=""

while (($#)); do
    case "$1" in
        --mode) MODE="$2"; shift 2 ;;
        --device) DEVICE="$2"; shift 2 ;;
        --lease-id) LEASE_ID="$2"; shift 2 ;;
        --owner) OWNER="$2"; shift 2 ;;
        --implementation) IMPLEMENTATION="$2"; shift 2 ;;
        --shape-index) SHAPE_INDEX="$2"; shift 2 ;;
        --warmup) WARMUP="$2"; shift 2 ;;
        --samples) SAMPLES="$2"; shift 2 ;;
        --blocks) BLOCKS="$2"; shift 2 ;;
        --pairs) PAIRS="$2"; shift 2 ;;
        --qualification-file) QUALIFICATION_FILE="$2"; shift 2 ;;
        *) printf 'unknown argument: %s\n' "$1" >&2; exit 2 ;;
    esac
done

if [[ ! "$DEVICE" =~ ^[0-9]+$ || ! "$LEASE_ID" =~ ^[A-Za-z0-9._-]+$ ||
      ("$OWNER" != "MAIN-1" && "$OWNER" != "MAIN-2") ]]; then
    printf 'device, explicit lease ID, and Main owner are required\n' >&2
    exit 2
fi
if [[ "$MODE" != "correctness" && "$MODE" != "noise-floor" && "$MODE" != "paired" ]]; then
    printf 'mode must be correctness, noise-floor, or paired\n' >&2
    exit 2
fi
if [[ "$MODE" == "correctness" &&
      ("$IMPLEMENTATION" != "parent" && "$IMPLEMENTATION" != "candidate") ]]; then
    printf 'correctness requires --implementation parent|candidate\n' >&2
    exit 2
fi
if [[ "$MODE" == "noise-floor" &&
      ("$IMPLEMENTATION" != "parent" && "$IMPLEMENTATION" != "candidate" ||
       ! "$SHAPE_INDEX" =~ ^([0-9]|1[0-9]|2[0-3])$ || WARMUP -lt 10 || SAMPLES -lt 21 || BLOCKS -ne 2) ]]; then
    printf 'noise-floor requires implementation, shape 0..23, warmup >=10, samples >=21, and two blocks\n' >&2
    exit 2
fi
if [[ "$MODE" == "paired" &&
      (! "$SHAPE_INDEX" =~ ^([0-9]|1[0-9]|2[0-3])$ || WARMUP -lt 10 || SAMPLES -lt 21 || PAIRS -lt 4 ||
       -z "$QUALIFICATION_FILE" || ! -f "$QUALIFICATION_FILE") ]]; then
    printf 'paired requires a local parent noise-floor record, shape 0..23, warmup >=10, samples >=21, and pairs >=4\n' >&2
    exit 2
fi
if [[ ! -f "$LEASE_LEDGER" ]]; then
    printf 'lease ledger unavailable: %s\n' "$LEASE_LEDGER" >&2
    exit 2
fi

read_active_lease() {
    awk -F '\t' -v device="$DEVICE" -v owner="$OWNER" -v lease="$LEASE_ID" \
        '$1 == device && $2 == owner && $3 == "DTYPE-SPECIAL-X" && $4 == lease {
            status=$5; start=$6; end=$7; line=$0
        }
        END {
            if (status == "LEASED" && end == "-") print line;
            else exit 1
        }' "$LEASE_LEDGER"
}

LEASE_ROW="$(read_active_lease)" || {
    printf 'no current DTYPE-SPECIAL-X lease for device=%s owner=%s lease=%s\n' \
        "$DEVICE" "$OWNER" "$LEASE_ID" >&2
    exit 2
}
LEASE_START="$(printf '%s\n' "$LEASE_ROW" | awk -F '\t' '{print $6}')"
NOW_UTC="$(date -u +%Y-%m-%dT%H:%M:%SZ)"
if [[ "$LEASE_START" > "$NOW_UTC" ]]; then
    printf 'lease start is in the future: %s\n' "$LEASE_START" >&2
    exit 2
fi

if [[ "$MODE" == "paired" ]]; then
    grep -q '^qualification=PASS$' "$QUALIFICATION_FILE" || {
        printf 'qualification file is not PASS\n' >&2
        exit 2
    }
    grep -q '^route=DTYPE-SPECIAL-X$' "$QUALIFICATION_FILE" || exit 2
    grep -q '^revision=V001$' "$QUALIFICATION_FILE" || exit 2
    grep -q '^mode=noise-floor$' "$QUALIFICATION_FILE" || exit 2
    grep -q '^implementation=parent$' "$QUALIFICATION_FILE" || exit 2
    grep -q "^shape_index=${SHAPE_INDEX}$" "$QUALIFICATION_FILE" || exit 2
fi

RUN_ID="dtype-${MODE}-$(date -u +%Y%m%dT%H%M%SZ)-$$"
mkdir -p "$LOCAL_LOG"
PREFLIGHT_PATH="${LOCAL_LOG}/unified-${RUN_ID}-preflight.txt"
RUN_LOG="${LOCAL_LOG}/unified-${RUN_ID}.log"
REMOTE_PREFIX="${REMOTE_ROOT}/logs/${RUN_ID}"
REMOTE_BINARY="${REMOTE_ROOT}/build/dtype_special_x_v001_unified_runner"
RUNNER_SHA="$(ssh "$REMOTE_HOST" "sha256sum '${REMOTE_BINARY}'" | awk '{print $1}')"
if [[ ! "$RUNNER_SHA" =~ ^[0-9a-f]{64}$ ]]; then
    printf 'unified runner binary is missing or has no SHA-256 identity\n' >&2
    exit 2
fi

REMOTE_QUALIFICATION=""
if [[ "$MODE" == "paired" ]]; then
    REMOTE_QUALIFICATION="${REMOTE_ROOT}/logs/${RUN_ID}-qualification.txt"
    scp "$QUALIFICATION_FILE" "${REMOTE_HOST}:${REMOTE_QUALIFICATION}"
fi

{
    printf 'route=DTYPE-SPECIAL-X\nrevision=V001\nmode=%s\n' "$MODE"
    printf 'lease_row=%s\n' "$LEASE_ROW"
    printf 'runner_binary=%s\nrunner_sha256=%s\n' "$REMOTE_BINARY" "$RUNNER_SHA"
    printf 'remote_preflight_begin=\n'
    ssh "$REMOTE_HOST" "printf 'preflight_utc='; date -u +%Y-%m-%dT%H:%M:%SZ; npu-smi info -t usages -i '${DEVICE}'; printf '%s\\n' '--- matching processes ---'; pgrep -af 'VLLMEngineCor|dtype_special_x_v001_unified_runner' || true"
} > "$PREFLIGHT_PATH"

LEASE_ROW="$(read_active_lease)" || {
    printf 'lease was released or changed during preflight\n' >&2
    exit 2
}
PREFLIGHT_UTC="$(awk -F= '/^preflight_utc=/{print $2; exit}' "$PREFLIGHT_PATH")"
if [[ ! "$PREFLIGHT_UTC" =~ ^[0-9]{4}-[0-9]{2}-[0-9]{2}T[0-9]{2}:[0-9]{2}:[0-9]{2}Z$ ]]; then
    printf 'could not obtain remote preflight time\n' >&2
    exit 2
fi

ARGS=(--mode "$MODE" --device "$DEVICE" --lease-id "$LEASE_ID" --owner "$OWNER"
      --preflight-utc "$PREFLIGHT_UTC" --runner-sha256 "$RUNNER_SHA"
      --out-prefix "$REMOTE_PREFIX" --warmup "$WARMUP" --samples "$SAMPLES")
if [[ "$MODE" == "correctness" ]]; then
    ARGS+=(--implementation "$IMPLEMENTATION")
elif [[ "$MODE" == "noise-floor" ]]; then
    ARGS+=(--implementation "$IMPLEMENTATION" --shape-index "$SHAPE_INDEX" --blocks "$BLOCKS")
else
    ARGS+=(--shape-index "$SHAPE_INDEX" --pairs "$PAIRS" --qualification-file "$REMOTE_QUALIFICATION")
fi

COMMAND_FILE="${LOCAL_LOG}/unified-${RUN_ID}-command.txt"
printf '%q ' "${ARGS[@]}" > "$COMMAND_FILE"
printf '\n' >> "$COMMAND_FILE"
printf 'remote_binary=%s\nrunner_sha256=%s\n' "$REMOTE_BINARY" "$RUNNER_SHA" >> "$COMMAND_FILE"

printf -v REMOTE_COMMAND '%q ' "$REMOTE_BINARY" "${ARGS[@]}"
set +e
ssh "$REMOTE_HOST" "source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh && export ASCEND_OPP_PATH=\$ASCEND_HOME_PATH/opp && export LD_LIBRARY_PATH=\$ASCEND_HOME_PATH/aarch64-linux/lib64:\${LD_LIBRARY_PATH:-} && exec ${REMOTE_COMMAND}" 2>&1 | tee "$RUN_LOG"
RUN_STATUS=${PIPESTATUS[0]}
set -e

for suffix in raw.tsv jitter.txt pairs.tsv correctness.tsv; do
    REMOTE_FILE="${REMOTE_PREFIX}.${suffix}"
    if ssh "$REMOTE_HOST" "test -f '${REMOTE_FILE}'"; then
        scp "${REMOTE_HOST}:${REMOTE_FILE}" "${LOCAL_LOG}/unified-${RUN_ID}.${suffix}"
    fi
done
printf 'runner_exit_status=%s\n' "$RUN_STATUS" >> "$RUN_LOG"
exit "$RUN_STATUS"
