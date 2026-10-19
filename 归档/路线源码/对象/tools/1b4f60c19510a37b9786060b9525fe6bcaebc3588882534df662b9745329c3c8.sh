#!/usr/bin/env bash
set -eo pipefail

SOURCE_ROOT=/home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924/identity_audit_r015c_r2_vs_r3_20260924
RUN_ROOT=/home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924/paired_device_event_r015c_20260924
SUPPORT_ROOT="$RUN_ROOT/support/device_event"
R2_SOURCE_ROOT="$SOURCE_ROOT/r2_parent"
R3_SOURCE_ROOT="$SOURCE_ROOT/r3_candidate"
R2_ROOT="$RUN_ROOT/parent"
R3_ROOT="$RUN_ROOT/candidate"

R2_REVISION=R015C-r2
R2_SOURCE_SHA=9471d7c2faf12c8d5d31e3cc3865fe2a703362b2d8cdb635beda2e64ebe259be
R2_OBJECT_SHA=d6e4483899ab37f80ecafebad29eebca652c6c34ba45c785be66ac7c6a3b981e
R2_EXECUTABLE_SHA=dad7a4c6b83fa84774f074d366c4f20301b932c5eeb572042bc6ec7ffc9b83e7

R3_REVISION=R015C-r3
R3_SOURCE_SHA=e1786bec2673519f41887fdffa0e11751f515a81037d854c88ae7edc0e4af903
R3_OBJECT_SHA=392c575073960181af49b8fa1036aadb4c61506e5d4146d58b57d5119aeeb390
R3_EXECUTABLE_SHA=faea8be543277b77af2bc07d6b08ca43134fceaed17ab807fe8cfd5ecdcd0aee

EVENT_HOST_SOURCE_SHA=5d0abe48ebe1a21b8c8af84341f5667992fd1bc23eac09b553e2180cbbbb74c3
EVENT_CMAKE_SHA=bf3bbde2eca10b9de9339dad83ab01672a00cfa889921463c86bff0c322bb0d9
EVENT_OBJECT_TARGET=row_copy_event_probe
EVENT_EXECUTABLE=row_copy_event_probe
TILING_SOURCE_SHA=5e4ad750f5c63da357324c0a829674aff81b7c0b655e8ff08072b1c289c66c41
CASES=("2 256" "5 4096" "3 8192")
WARMUP_LAUNCHES=32
EVENT_REPEATS=7
LAUNCHES_PER_SAMPLE=512

fail() {
    printf 'ERROR %s\n' "$*" >&2
    exit 2
}

sha_of() {
    sha256sum "$1" | awk '{print $1}'
}

assert_sha() {
    local path="$1" expected="$2" label="$3" actual
    [[ -f "$path" ]] || fail "missing $label: $path"
    actual="$(sha_of "$path")"
    [[ "$actual" == "$expected" ]] || fail "$label SHA256 mismatch: path=$path expected=$expected actual=$actual"
}

verify_revision() {
    local label="$1" source_root="$2" root="$3" revision="$4" source_sha="$5" object_sha="$6" executable_sha="$7"
    local executable="$root/build/$EVENT_EXECUTABLE"
    local build_root kernel_dir dependency object

    [[ "$executable" = /* ]] || fail "non-absolute executable path: $executable"
    [[ "$(readlink -f "$executable")" == "$executable" ]] || fail "executable path is not canonical: $executable"
    assert_sha "$source_root/op_kernel/row_copy_kernel.asc" "$source_sha" "$label kernel source"
    assert_sha "$source_root/op_kernel/row_copy_tiling.h" "$TILING_SOURCE_SHA" "$label tiling source"
    assert_sha "$SUPPORT_ROOT/op_host/row_copy_event_host.asc" "$EVENT_HOST_SOURCE_SHA" "shared event host source"
    assert_sha "$SUPPORT_ROOT/CMakeLists.txt" "$EVENT_CMAKE_SHA" "event CMake source"
    object="$root/build/CMakeFiles/$EVENT_OBJECT_TARGET.dir/op_host/row_copy_event_host.asc.o"
    assert_sha "$object" "$object_sha" "$label object"
    assert_sha "$executable" "$executable_sha" "$label executable"

    build_root="$(grep '^CMAKE_HOME_DIRECTORY:INTERNAL=' "$root/build/CMakeCache.txt" | cut -d= -f2-)"
    [[ "$build_root" == "$SUPPORT_ROOT" ]] || fail "$label CMake source root mismatch: $build_root"
    kernel_dir="$(grep '^ROW_COPY_KERNEL_DIR:PATH=' "$root/build/CMakeCache.txt" | cut -d= -f2-)"
    [[ "$kernel_dir" == "$source_root/op_kernel" ]] || fail "$label configured kernel directory mismatch: $kernel_dir"
    dependency="$root/build/CMakeFiles/$EVENT_OBJECT_TARGET.dir/op_host/row_copy_event_host.asc.o.d"
    grep -Fq "$source_root/op_kernel/row_copy_kernel.asc" "$dependency" || \
        fail "$label compiler dependency does not name its declared kernel source"
    grep -Fq "$SUPPORT_ROOT/op_host/row_copy_event_host.asc" "$dependency" || \
        fail "$label compiler dependency does not name the shared timing host source"

    printf 'IDENTITY role=%s revision=%s source_sha256=%s executable_path=%s executable_sha256=%s object_sha256=%s\n' \
        "$label" "$revision" "$source_sha" "$executable" "$executable_sha" "$object_sha"
}

verify_identity() {
    verify_revision parent "$R2_SOURCE_ROOT" "$R2_ROOT" "$R2_REVISION" "$R2_SOURCE_SHA" "$R2_OBJECT_SHA" "$R2_EXECUTABLE_SHA"
    verify_revision candidate "$R3_SOURCE_ROOT" "$R3_ROOT" "$R3_REVISION" "$R3_SOURCE_SHA" "$R3_OBJECT_SHA" "$R3_EXECUTABLE_SHA"
    [[ "$R2_EXECUTABLE_SHA" != "$R3_EXECUTABLE_SHA" ]] || fail "parent and candidate executable SHA256 are identical"
    printf 'ARTIFACTS_DISTINCT=YES\n'
}

make_command() {
    local executable="$1" device="$2" rows="$3" cols="$4" warmups="$5" repeats="$6" launches="$7"
    COMMAND=(timeout 90s env "ASCEND_DEVICE_ID=$device" ASCEND_SLOG_PRINT_TO_STDOUT=0 ASCEND_GLOBAL_LOG_LEVEL=3 \
        "$executable" --device-event-bench "$rows" "$cols" "$device" "$warmups" "$repeats" "$launches")
}

print_plan_entry() {
    local role="$1" revision="$2" source_sha="$3" executable="$4" executable_sha="$5"
    local device="$6" rows="$7" cols="$8" pair="$9"
    make_command "$executable" "$device" "$rows" "$cols" "$WARMUP_LAUNCHES" "$EVENT_REPEATS" "$LAUNCHES_PER_SAMPLE"
    printf 'PLAN pair=%s route=MODE-X-R015C role=%s revision=%s source_sha256=%s executable_path=%s executable_sha256=%s rows=%s D=%s timing=ACL_DEVICE_EVENT warmup_launches=%s event_repeats=%s launches_per_sample=%s\n' \
        "$pair" "$role" "$revision" "$source_sha" "$executable" "$executable_sha" "$rows" "$cols" "$WARMUP_LAUNCHES" "$EVENT_REPEATS" "$LAUNCHES_PER_SAMPLE"
    printf 'COMMAND='
    printf '%q ' "${COMMAND[@]}"
    printf '\n'
}

snapshot() {
    local label="$1" device="$2" rc output
    printf '\nSNAPSHOT_BEGIN label=%s device=%s\n' "$label" "$device"
    for query in "npu-smi info" "npu-smi info -t usages -i $device" "npu-smi info -t process -i $device"; do
        printf 'SNAPSHOT_COMMAND=%s\n' "$query"
        set +e
        output="$(bash -c "$query" 2>&1)"
        rc=$?
        set -e
        printf '%s\n' "$output"
        printf 'SNAPSHOT_EXIT=%s\n' "$rc"
    done
    printf 'HOST_PROCESSES_BEGIN\n'
    ps -eo pid,comm,rss,args | awk 'NR == 1 || $0 ~ /(python|vllm|row_copy_probe)/'
    printf 'SNAPSHOT_END label=%s device=%s\n\n' "$label" "$device"
}

LAST_EVENT_TOTAL_NS=0
LAST_EVENT_NS_PER_LAUNCH=0
LAST_EVENT_JITTER_NS_PER_LAUNCH=0
LAST_STATUS=FAIL
PARENT_FAILED=0
CANDIDATE_FAILED=0

run_one() {
    local role="$1" revision="$2" source_sha="$3" executable="$4" executable_sha="$5"
    local device="$6" rows="$7" cols="$8" pair="$9" phase="${10}"
    local current_sha output rc summary median_total median_per_launch jitter_per_launch sample_launches

    verify_identity >/dev/null
    current_sha="$(sha_of "$executable")"
    [[ "$current_sha" == "$executable_sha" ]] || fail "executable changed immediately before run: $executable"
    make_command "$executable" "$device" "$rows" "$cols" "$WARMUP_LAUNCHES" "$EVENT_REPEATS" "$LAUNCHES_PER_SAMPLE"
    printf '\nRUN_BEGIN route=MODE-X-R015C phase=%s pair=%s role=%s revision=%s source_sha256=%s executable_path=%s executable_sha256=%s timing=ACL_DEVICE_EVENT timed_launches_per_sample=%s event_repeats=%s\n' \
        "$phase" "$pair" "$role" "$revision" "$source_sha" "$executable" "$current_sha" "$LAUNCHES_PER_SAMPLE" "$EVENT_REPEATS"
    printf 'COMMAND='
    printf '%q ' "${COMMAND[@]}"
    printf '\n'

    set +e
    output="$("${COMMAND[@]}" 2>&1)"
    rc=$?
    set -e
    printf '%s\n' "$output"
    summary="$(grep '^DEVICE_EVENT_SUMMARY ' <<< "$output" | tail -n 1)"
    [[ -n "$summary" ]] || fail "missing device event summary for $role, rows=$rows, D=$cols"
    median_total="$(sed -n 's/.*median_total_ns=\([0-9][0-9]*\).*/\1/p' <<< "$summary")"
    median_per_launch="$(sed -n 's/.*median_ns_per_launch=\([0-9.][0-9.]*\).*/\1/p' <<< "$summary")"
    jitter_per_launch="$(sed -n 's/.*jitter_range_ns_per_launch=\([0-9.][0-9.]*\).*/\1/p' <<< "$summary")"
    sample_launches="$(sed -n 's/.*launches_per_sample=\([0-9][0-9]*\).*/\1/p' <<< "$summary")"
    [[ "$median_total" =~ ^[0-9]+$ && "$median_per_launch" =~ ^[0-9]+\.[0-9]+$ && \
       "$jitter_per_launch" =~ ^[0-9]+\.[0-9]+$ && "$sample_launches" == "$LAUNCHES_PER_SAMPLE" ]] || \
        fail "malformed device event summary for $role"
    LAST_EVENT_TOTAL_NS="$median_total"
    LAST_EVENT_NS_PER_LAUNCH="$median_per_launch"
    LAST_EVENT_JITTER_NS_PER_LAUNCH="$jitter_per_launch"
    if [[ "$rc" == 0 ]] && grep -Fq "NPU_EVENT_PROBE PASS device=$device rows=$rows D=$cols exact=1" <<< "$output"; then
        LAST_STATUS=PASS
    else
        LAST_STATUS=FAIL
        if [[ "$role" == parent ]]; then PARENT_FAILED=1; else CANDIDATE_FAILED=1; fi
    fi
    printf 'RUN_END phase=%s pair=%s role=%s revision=%s exit=%s correctness=%s device_event_median_total_ns=%s device_event_median_ns_per_launch=%s device_event_jitter_ns_per_launch=%s process_wall_time=NOT_MEASURED\n' \
        "$phase" "$pair" "$role" "$revision" "$rc" "$LAST_STATUS" "$LAST_EVENT_TOTAL_NS" "$LAST_EVENT_NS_PER_LAUNCH" "$LAST_EVENT_JITTER_NS_PER_LAUNCH"
    current_sha="$(sha_of "$executable")"
    [[ "$current_sha" == "$executable_sha" ]] || fail "executable SHA256 changed during run: $executable"
}

median() {
    printf '%s\n' "$@" | sort -n | awk '{ values[NR] = $1 } END { if (NR % 2) printf "%.3f", values[(NR + 1) / 2]; else printf "%.3f", (values[NR / 2] + values[NR / 2 + 1]) / 2 }'
}

spread() {
    printf '%s\n' "$@" | sort -n | awk 'NR == 1 { low = $1 } { high = $1 } END { printf "%.3f", high - low }'
}

audit_plan() {
    local device="$1" pairs="$2" pair shape rows cols
    [[ "$device" =~ ^[0-7]$ ]] || fail "device must be 0..7"
    [[ "$pairs" =~ ^[1-9][0-9]*$ ]] || fail "pairs must be a positive integer"
    verify_identity
    printf 'MODE=HOST_ONLY_PLAN; NPU_EXECUTION=NOT_RUN; TIMING=NOT_RUN\n'
    for shape in "${CASES[@]}"; do
        read -r rows cols <<< "$shape"
        for ((pair = 1; pair <= pairs; ++pair)); do
            if (( pair % 2 )); then
                print_plan_entry parent "$R2_REVISION" "$R2_SOURCE_SHA" "$R2_ROOT/build/$EVENT_EXECUTABLE" "$R2_EXECUTABLE_SHA" "$device" "$rows" "$cols" "$pair"
                print_plan_entry candidate "$R3_REVISION" "$R3_SOURCE_SHA" "$R3_ROOT/build/$EVENT_EXECUTABLE" "$R3_EXECUTABLE_SHA" "$device" "$rows" "$cols" "$pair"
            else
                print_plan_entry candidate "$R3_REVISION" "$R3_SOURCE_SHA" "$R3_ROOT/build/$EVENT_EXECUTABLE" "$R3_EXECUTABLE_SHA" "$device" "$rows" "$cols" "$pair"
                print_plan_entry parent "$R2_REVISION" "$R2_SOURCE_SHA" "$R2_ROOT/build/$EVENT_EXECUTABLE" "$R2_EXECUTABLE_SHA" "$device" "$rows" "$cols" "$pair"
            fi
        done
    done
}

run_pairs() {
    local lease="$1" device="$2" warmups="$3" pairs="$4"
    local shape rows cols pair
    local -a parent_times candidate_times parent_sample_jitters candidate_sample_jitters deltas
    local parent_median candidate_median parent_jitter candidate_jitter parent_inner_jitter candidate_inner_jitter
    local delta_median faster slower tied direction parent_executable candidate_executable

    [[ -n "$lease" ]] || fail "Main-1 lease ID is required"
    [[ "$lease" =~ ^[A-Za-z0-9][A-Za-z0-9._-]{0,80}$ ]] || fail "invalid Main-1 lease ID"
    [[ "$device" =~ ^[0-7]$ ]] || fail "device must be 0..7"
    [[ "$warmups" =~ ^[0-9]+$ ]] || fail "warmups must be a nonnegative integer"
    [[ "$pairs" =~ ^[1-9][0-9]*$ ]] || fail "pairs must be a positive integer"
    verify_identity
    source /usr/local/Ascend/ascend-toolkit/set_env.sh
    local pair_log="$RUN_ROOT/paired_event_${lease}.log"
    [[ ! -e "$pair_log" ]] || fail "refusing to overwrite existing paired log: $pair_log"
    exec > >(tee "$pair_log") 2>&1
    parent_executable="$R2_ROOT/build/$EVENT_EXECUTABLE"
    candidate_executable="$R3_ROOT/build/$EVENT_EXECUTABLE"
    printf 'MODE=LEASED_PAIRED_DEVICE_EVENT_RUN route=MODE-X-R015C Main1_lease=%s priority=5 device=%s log=%s order=AB_BA_ALTERNATING warmup_launches=%s event_repeats=%s launches_per_sample=%s\n' \
        "$lease" "$device" "$pair_log" "$WARMUP_LAUNCHES" "$EVENT_REPEATS" "$LAUNCHES_PER_SAMPLE"
    printf 'COMPARISON_ELIGIBILITY=DIAGNOSTIC_ONLY_PARENT_R2_HAS_RECORDED_CORRECTNESS_FAILURES\n'
    printf 'TIMING_SCOPE=ACL_DEVICE_EVENT_INTERVAL_AROUND_KERNEL_BATCH; INIT_ALLOC_H2D_D2H_CORRECTNESS_TEARDOWN_EXCLUDED\n'

    for shape in "${CASES[@]}"; do
        read -r rows cols <<< "$shape"
        printf '\nCASE_BEGIN rows=%s D=%s warmups=%s measured_pairs=%s\n' "$rows" "$cols" "$warmups" "$pairs"
        for ((pair = 1; pair <= warmups; ++pair)); do
            if (( pair % 2 )); then
                run_one parent "$R2_REVISION" "$R2_SOURCE_SHA" "$parent_executable" "$R2_EXECUTABLE_SHA" "$device" "$rows" "$cols" "$pair" WARMUP
                run_one candidate "$R3_REVISION" "$R3_SOURCE_SHA" "$candidate_executable" "$R3_EXECUTABLE_SHA" "$device" "$rows" "$cols" "$pair" WARMUP
            else
                run_one candidate "$R3_REVISION" "$R3_SOURCE_SHA" "$candidate_executable" "$R3_EXECUTABLE_SHA" "$device" "$rows" "$cols" "$pair" WARMUP
                run_one parent "$R2_REVISION" "$R2_SOURCE_SHA" "$parent_executable" "$R2_EXECUTABLE_SHA" "$device" "$rows" "$cols" "$pair" WARMUP
            fi
        done

        parent_times=()
        candidate_times=()
        parent_sample_jitters=()
        candidate_sample_jitters=()
        deltas=()
        for ((pair = 1; pair <= pairs; ++pair)); do
            snapshot "before_pair_${pair}_R${rows}_D${cols}" "$device"
            if (( pair % 2 )); then
                run_one parent "$R2_REVISION" "$R2_SOURCE_SHA" "$parent_executable" "$R2_EXECUTABLE_SHA" "$device" "$rows" "$cols" "$pair" MEASURED
                parent_times[$pair]="$LAST_EVENT_NS_PER_LAUNCH"
                parent_sample_jitters[$pair]="$LAST_EVENT_JITTER_NS_PER_LAUNCH"
                run_one candidate "$R3_REVISION" "$R3_SOURCE_SHA" "$candidate_executable" "$R3_EXECUTABLE_SHA" "$device" "$rows" "$cols" "$pair" MEASURED
                candidate_times[$pair]="$LAST_EVENT_NS_PER_LAUNCH"
                candidate_sample_jitters[$pair]="$LAST_EVENT_JITTER_NS_PER_LAUNCH"
            else
                run_one candidate "$R3_REVISION" "$R3_SOURCE_SHA" "$candidate_executable" "$R3_EXECUTABLE_SHA" "$device" "$rows" "$cols" "$pair" MEASURED
                candidate_times[$pair]="$LAST_EVENT_NS_PER_LAUNCH"
                candidate_sample_jitters[$pair]="$LAST_EVENT_JITTER_NS_PER_LAUNCH"
                run_one parent "$R2_REVISION" "$R2_SOURCE_SHA" "$parent_executable" "$R2_EXECUTABLE_SHA" "$device" "$rows" "$cols" "$pair" MEASURED
                parent_times[$pair]="$LAST_EVENT_NS_PER_LAUNCH"
                parent_sample_jitters[$pair]="$LAST_EVENT_JITTER_NS_PER_LAUNCH"
            fi
            deltas[$pair]="$(awk -v c="${candidate_times[$pair]}" -v p="${parent_times[$pair]}" 'BEGIN { printf "%.3f", c - p }')"
            snapshot "after_pair_${pair}_R${rows}_D${cols}" "$device"
            printf 'PAIR_RESULT rows=%s D=%s pair=%s parent_event_ns_per_launch=%s candidate_event_ns_per_launch=%s delta_candidate_minus_parent_ns_per_launch=%s\n' \
                "$rows" "$cols" "$pair" "${parent_times[$pair]}" "${candidate_times[$pair]}" "${deltas[$pair]}"
        done

        parent_median="$(median "${parent_times[@]}")"
        candidate_median="$(median "${candidate_times[@]}")"
        parent_jitter="$(spread "${parent_times[@]}")"
        candidate_jitter="$(spread "${candidate_times[@]}")"
        parent_inner_jitter="$(median "${parent_sample_jitters[@]}")"
        candidate_inner_jitter="$(median "${candidate_sample_jitters[@]}")"
        delta_median="$(median "${deltas[@]}")"
        faster=0
        slower=0
        tied=0
        for pair in "${deltas[@]}"; do
            relation="$(awk -v delta="$pair" 'BEGIN { if (delta < 0) print "faster"; else if (delta > 0) print "slower"; else print "tied" }')"
            if [[ "$relation" == faster ]]; then faster=$((faster + 1)); elif [[ "$relation" == slower ]]; then slower=$((slower + 1)); else tied=$((tied + 1)); fi
        done
        if (( faster == pairs )); then direction=CANDIDATE_FASTER_ALL_PAIRS
        elif (( slower == pairs )); then direction=CANDIDATE_SLOWER_ALL_PAIRS
        else direction=MIXED_OR_TIED
        fi
        printf 'CASE_SUMMARY rows=%s D=%s parent_median_event_ns_per_launch=%s candidate_median_event_ns_per_launch=%s median_pair_delta_ns_per_launch=%s parent_pair_jitter_range_ns=%s candidate_pair_jitter_range_ns=%s parent_inner_event_jitter_median_ns_per_launch=%s candidate_inner_event_jitter_median_ns_per_launch=%s direction=%s faster_pairs=%s slower_pairs=%s tied_pairs=%s load_quality=LOAD_CONTAMINATED_UNLESS_MAIN_RECLASSIFIES\n' \
            "$rows" "$cols" "$parent_median" "$candidate_median" "$delta_median" "$parent_jitter" "$candidate_jitter" "$parent_inner_jitter" "$candidate_inner_jitter" "$direction" "$faster" "$slower" "$tied"
    done
    printf '\nFINAL parent_any_correctness_failure=%s candidate_any_correctness_failure=%s performance_claim=NONE decision=NEEDS_ONE_MORE_LOCAL\n' \
        "$PARENT_FAILED" "$CANDIDATE_FAILED"
    [[ "$PARENT_FAILED" == 0 && "$CANDIDATE_FAILED" == 0 ]] || exit 3
}

case "${1:-}" in
    --audit-plan)
        [[ "$#" -ge 2 && "$#" -le 3 ]] || fail "usage: $0 --audit-plan DEVICE_ID [PAIRS]"
        audit_plan "$2" "${3:-5}"
        ;;
    --run-after-main1-lease)
        [[ "$#" -ge 3 && "$#" -le 5 ]] || fail "usage: $0 --run-after-main1-lease LEASE_ID DEVICE_ID [WARMUPS] [PAIRS]"
        run_pairs "$2" "$3" "${4:-3}" "${5:-5}"
        ;;
    *)
        fail "usage: $0 --audit-plan DEVICE_ID [PAIRS] | --run-after-main1-lease LEASE_ID DEVICE_ID [WARMUPS] [PAIRS]"
        ;;
esac
