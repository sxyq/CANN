#!/usr/bin/env bash
set -euo pipefail

if [ "$#" -ne 1 ]; then
    echo "usage: paired-probe-harness.sh --same-binary|--paired" >&2
    exit 64
fi
case "$1" in
    --same-binary)
        RUNNER_MODE=--qualify-v011
        LOG_PREFIX=paired-probe-samebinary
        ;;
    --paired)
        RUNNER_MODE=--paired
        LOG_PREFIX=paired-probe-unified
        ;;
    *)
        echo "usage: paired-probe-harness.sh --same-binary|--paired" >&2
        exit 64
        ;;
esac

if [ "${MAIN_DEVICE_LEASE:-}" != "R31B" ]; then
    echo "Main must grant the R31B device lease before timing" >&2
    exit 64
fi
if [ "${R31B_LOAD_WINDOW:-}" != "CLEAR" ]; then
    echo "Main must confirm a clear comparable R31B load window before timing" >&2
    exit 64
fi

LEASE_FILE="${R31B_LEASE_FILE:-/Users/sunyiyang/Desktop/Project/cann/phase4/control/server3-device-leases.tsv}"
if [ ! -r "$LEASE_FILE" ] || ! awk -F '\t' '
    NR == 1 { next }
    $1 == "4" && $4 != "" {
        owner[$4] = $2
        route[$4] = $3
        status[$4] = $5
    }
    END {
        active = 0
        selected = 0
        for (id in status) {
            if (status[id] == "LEASED") {
                active++
                if (owner[id] == "MAIN-1" && route[id] == "R31B") {
                    selected++
                }
            }
        }
        exit !(active == 1 && selected == 1)
    }
' "$LEASE_FILE"; then
    echo "Canonical lease ledger does not show one active MAIN-1 R31B lease on device 4" >&2
    exit 64
fi

EXPECTED_PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
EXPECTED_CANDIDATE_SOURCE_SHA=9f5c353e65a13a740fe97dc7e6415df032d27560831a3ad142c77592b8208eb5
EXPECTED_WRAPPER_SOURCE_SHA=b827960aa2d60e6efd953b30f3ecfcfab17af5111f4d03e5309c4c94c68d1a55
EXPECTED_RUNNER_CPP_SHA=d8a574735bda3e07cbfbc1de3c890a72fb7b437983eae38ea22d0bf8b0bd03bc
EXPECTED_RUNNER_ABI_SHA=096c0229f5d9e6fa37460808387517280cb0f2e4296e07b8c37f75cd73c89617
EXPECTED_RUNNER_CMAKE_SHA=29eb6e937ca94fbf2877ae22d2ee9a864066f8e5844ea0ccdc356c1ea40a3f3a
EXPECTED_RUNNER_SHA=6e9337a2c1bf4e9c74db56f55d90fd5eb233fdb7f6f4d189265b0a8f80cdd7fc
for expected in "$EXPECTED_WRAPPER_SOURCE_SHA" "$EXPECTED_RUNNER_CPP_SHA" \
                "$EXPECTED_RUNNER_ABI_SHA" "$EXPECTED_RUNNER_CMAKE_SHA" \
                "$EXPECTED_RUNNER_SHA"; do
    if [[ ! "$expected" =~ ^[0-9a-f]{64}$ ]]; then
        echo "Paired runner sources or executable SHA are not recorded; refusing remote execution" >&2
        exit 67
    fi
done

if [ "$RUNNER_MODE" = "--paired" ]; then
    QUALIFICATION_LOG="${R31B_QUALIFICATION_LOG:-}"
    if [ -z "$QUALIFICATION_LOG" ] || [ ! -r "$QUALIFICATION_LOG" ]; then
        echo "A readable same-binary qualification log is required before P/C timing" >&2
        exit 65
    fi
    if ! awk -v parent="$EXPECTED_PARENT_SOURCE_SHA" \
            -v candidate="$EXPECTED_CANDIDATE_SOURCE_SHA" \
            -v runner="$EXPECTED_RUNNER_SHA" '
        BEGIN {
            required["fp16-tail-d12288"] = 1
            required["fp16-wide-d32768"] = 1
            required["bf16-tail-d12288"] = 1
            required["bf16-wide-d32768"] = 1
        }
        /^PARENT_SOURCE_SHA=/ { split($0, part, "="); seen_parent = part[2] }
        /^CANDIDATE_SOURCE_SHA=/ { split($0, part, "="); seen_candidate = part[2] }
        /^RUNNER_SHA=/ { split($0, part, "="); seen_runner = part[2] }
        $1 == "SAME_BINARY_QUAL" {
            case_name = ""
            result = ""
            for (i = 2; i <= NF; ++i) {
                split($i, part, "=")
                if (part[1] == "case") case_name = part[2]
                if (part[1] == "result") result = part[2]
            }
            if (case_name in required) {
                count[case_name]++
                verdict[case_name] = result
            }
        }
        /^RUNNER_COMPLETE mode=QUALIFY_V011 correctness=PASS / { completed = 1 }
        END {
            if (seen_parent != parent || seen_candidate != candidate ||
                seen_runner != runner || !completed) exit 1
            for (case_name in required) {
                if (count[case_name] != 1 || verdict[case_name] != "PASS") exit 1
            }
        }
    ' "$QUALIFICATION_LOG"; then
        echo "Same-binary log does not qualify all four exact shapes and runner identities" >&2
        exit 65
    fi
fi

EVIDENCE_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
STAMP="$(date '+%Y%m%dT%H%M%S%z')"
LOG="${EVIDENCE_DIR}/${LOG_PREFIX}-${STAMP}.txt"
if [ -e "$LOG" ]; then
    echo "Refusing to overwrite existing evidence: $LOG" >&2
    exit 66
fi

printf 'ROUTE=R31B\nREVISION=V016\nDIRECT_PARENT=R31B-V011\n' | tee "$LOG"
printf 'RUNNER_MODE=%s\nMAIN_DEVICE_LEASE=%s\nR31B_LOAD_WINDOW=%s\n' \
    "$RUNNER_MODE" "$MAIN_DEVICE_LEASE" "$R31B_LOAD_WINDOW" | tee -a "$LOG"
printf 'PROCESS_MODEL=ONE_PROCESS_ALL_CASES\nDEVICE=4\nWARMUP=10\n' | tee -a "$LOG"
if [ "$RUNNER_MODE" = "--qualify-v011" ]; then
    printf 'QUALIFICATION_BINARY=V011_ONLY\nBLOCKS=2\nSAMPLES_PER_BLOCK=31\n' | tee -a "$LOG"
else
    printf 'PAIRS_PER_CASE=21\nQUALIFICATION_LOG=%s\n' "$QUALIFICATION_LOG" | tee -a "$LOG"
fi
printf 'TIMING_PRIMARY=DEVICE_EVENT_SINGLE_LAUNCH\nTIMING_DIAGNOSTIC=STEADY_CLOCK_LAUNCH_AND_EVENT_WAIT\n' | tee -a "$LOG"

if ssh cann-server3 "RUNNER_MODE='$RUNNER_MODE' bash -s" <<'REMOTE' | tee -a "$LOG"
set -euo pipefail

ROUTE_DIR=/home/data4t2/lelinfeng/phase4-review-repro/R31B-V016
RUNNER_SOURCE_DIR=${ROUTE_DIR}/paired-runner-src
PARENT_SOURCE=/home/data4t2/lelinfeng/phase4-workspaces/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc
CANDIDATE_SOURCE=${ROUTE_DIR}/probe-src/R31B-V016-WIDE-TILE-SEED_kernel.asc
RUNNER=${RUNNER_SOURCE_DIR}/build/paired_runner_v016
EXPECTED_PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
EXPECTED_CANDIDATE_SOURCE_SHA=9f5c353e65a13a740fe97dc7e6415df032d27560831a3ad142c77592b8208eb5
EXPECTED_WRAPPER_SOURCE_SHA=b827960aa2d60e6efd953b30f3ecfcfab17af5111f4d03e5309c4c94c68d1a55
EXPECTED_RUNNER_CPP_SHA=d8a574735bda3e07cbfbc1de3c890a72fb7b437983eae38ea22d0bf8b0bd03bc
EXPECTED_RUNNER_ABI_SHA=096c0229f5d9e6fa37460808387517280cb0f2e4296e07b8c37f75cd73c89617
EXPECTED_RUNNER_CMAKE_SHA=29eb6e937ca94fbf2877ae22d2ee9a864066f8e5844ea0ccdc356c1ea40a3f3a
EXPECTED_RUNNER_SHA=6e9337a2c1bf4e9c74db56f55d90fd5eb233fdb7f6f4d189265b0a8f80cdd7fc

verify_sha() {
    local label="$1"
    local path="$2"
    local expected="$3"
    test -f "$path"
    local actual
    actual=$(sha256sum "$path" | awk '{print $1}')
    printf '%s_SHA=%s\n' "$label" "$actual"
    if [ "$actual" != "$expected" ]; then
        printf 'IDENTITY_MISMATCH=%s\n' "$label" >&2
        exit 68
    fi
}

test -x "$RUNNER"
verify_sha PARENT_SOURCE "$PARENT_SOURCE" "$EXPECTED_PARENT_SOURCE_SHA"
verify_sha CANDIDATE_SOURCE "$CANDIDATE_SOURCE" "$EXPECTED_CANDIDATE_SOURCE_SHA"
verify_sha PAIRED_WRAPPER_SOURCE "${RUNNER_SOURCE_DIR}/paired_runner_v016.asc" "$EXPECTED_WRAPPER_SOURCE_SHA"
verify_sha PAIRED_RUNNER_CPP "${RUNNER_SOURCE_DIR}/paired_runner.cpp" "$EXPECTED_RUNNER_CPP_SHA"
verify_sha PAIRED_RUNNER_ABI "${RUNNER_SOURCE_DIR}/paired_runner_abi.h" "$EXPECTED_RUNNER_ABI_SHA"
verify_sha PAIRED_RUNNER_CMAKE "${RUNNER_SOURCE_DIR}/CMakeLists.txt" "$EXPECTED_RUNNER_CMAKE_SHA"
verify_sha RUNNER "$RUNNER" "$EXPECTED_RUNNER_SHA"

capture_load() {
    printf 'LOAD_SNAPSHOT=%s\n' "$1"
    TZ=Asia/Shanghai date '+%Y-%m-%dT%H:%M:%S%z'
    npu-smi info -t usages -i 4
    printf 'PROCESSES_BEGIN\n'
    ps -eo pid=,comm=,args= | grep -E '[Vv][Ll][Ll][Mm]|EngineCore|Worker_TP' | grep -v grep | sort -n || true
    printf 'PROCESSES_END\n'
}

printf 'REMOTE_HOST=%s\n' "$(hostname)"
source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh
export LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu:${LD_LIBRARY_PATH:-}
capture_load BEFORE_RUN
printf 'RUNNER_COMMAND=%s %s\n' "$RUNNER" "$RUNNER_MODE"
"$RUNNER" "$RUNNER_MODE"
capture_load AFTER_RUN
printf 'REMOTE_RUN=PASS\n'
REMOTE
then
    :
else
    printf 'REMOTE_RUN=FAIL\n' | tee -a "$LOG"
    exit 69
fi

printf 'EVIDENCE_LOG=%s\n' "$LOG" | tee -a "$LOG"
