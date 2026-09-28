#!/usr/bin/env bash
set -eo pipefail

source /usr/local/Ascend/ascend-toolkit/set_env.sh
export CPLUS_INCLUDE_PATH="${CPLUS_INCLUDE_PATH:-/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward:/usr/include/aarch64-linux-gnu}"

RUN_ROOT=/home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924/paired_device_event_r015c_20260924
SUPPORT_ROOT="$RUN_ROOT/support/device_event"
SOURCE_ROOT=/home/data4t2/lelinfeng/MODE-X-R015C_runs/dispatch_20260924/identity_audit_r015c_r2_vs_r3_20260924
LOG="$RUN_ROOT/build_and_identity_event_timing_20260924.log"

[[ -f "$SUPPORT_ROOT/CMakeLists.txt" ]] || { echo "missing Route-owned event support source: $SUPPORT_ROOT" >&2; exit 2; }
[[ ! -e "$LOG" ]] || { echo "refusing to overwrite existing log: $LOG" >&2; exit 2; }

mkdir -p "$RUN_ROOT/parent" "$RUN_ROOT/candidate"
{
    date -Is
    printf 'MODE=HOST_BUILD_AND_IDENTITY_ONLY\n'
    printf 'CANN_HOME=%s\n' "$ASCEND_HOME_PATH"
    sha256sum "$SUPPORT_ROOT/CMakeLists.txt" "$SUPPORT_ROOT/op_host/row_copy_event_host.asc"

    for entry in "parent R015C-r2 r2_parent 9471d7c2faf12c8d5d31e3cc3865fe2a703362b2d8cdb635beda2e64ebe259be" \
                 "candidate R015C-r3 r3_candidate e1786bec2673519f41887fdffa0e11751f515a81037d854c88ae7edc0e4af903"; do
        read -r role revision source_dir_name source_sha <<< "$entry"
        source_dir="$SOURCE_ROOT/$source_dir_name"
        build_dir="$RUN_ROOT/$role/build"
        printf '\nBUILD role=%s revision=%s kernel_source_sha256_expected=%s kernel_source_sha256_actual=' "$role" "$revision" "$source_sha"
        actual_source_sha="$(sha256sum "$source_dir/op_kernel/row_copy_kernel.asc" | awk '{print $1}')"
        printf '%s\n' "$actual_source_sha"
        [[ "$actual_source_sha" == "$source_sha" ]] || { echo "kernel source SHA mismatch for $revision" >&2; exit 3; }
        printf 'CONFIGURE_COMMAND='; printf '%q ' cmake -S "$SUPPORT_ROOT" -B "$build_dir" -DCMAKE_BUILD_TYPE=Release "-DROW_COPY_KERNEL_DIR:PATH=$source_dir/op_kernel"; printf '\n'
        cmake -S "$SUPPORT_ROOT" -B "$build_dir" \
            -DCMAKE_BUILD_TYPE=Release \
            -DROW_COPY_KERNEL_DIR:PATH="$source_dir/op_kernel"
        printf 'BUILD_COMMAND='; printf '%q ' cmake --build "$build_dir" --parallel 4; printf '\n'
        cmake --build "$build_dir" --parallel 4
        printf 'BUILD_EXIT_%s=0\n' "$role"
        sha256sum \
            "$source_dir/op_kernel/row_copy_kernel.asc" \
            "$source_dir/op_kernel/row_copy_tiling.h" \
            "$SUPPORT_ROOT/op_host/row_copy_event_host.asc" \
            "$build_dir/CMakeFiles/row_copy_event_probe.dir/op_host/row_copy_event_host.asc.o" \
            "$build_dir/row_copy_event_probe"
        grep '^CMAKE_HOME_DIRECTORY:INTERNAL=' "$build_dir/CMakeCache.txt"
        grep '^ROW_COPY_KERNEL_DIR:PATH=' "$build_dir/CMakeCache.txt"
        grep -F "$source_dir/op_kernel/row_copy_kernel.asc" \
            "$build_dir/CMakeFiles/row_copy_event_probe.dir/op_host/row_copy_event_host.asc.o.d"
    done

    parent_sha="$(sha256sum "$RUN_ROOT/parent/build/row_copy_event_probe" | awk '{print $1}')"
    candidate_sha="$(sha256sum "$RUN_ROOT/candidate/build/row_copy_event_probe" | awk '{print $1}')"
    [[ "$parent_sha" != "$candidate_sha" ]] || { echo "parent and candidate executables are identical" >&2; exit 4; }
    printf '\nPARENT_EXECUTABLE_SHA256=%s\n' "$parent_sha"
    printf 'CANDIDATE_EXECUTABLE_SHA256=%s\n' "$candidate_sha"
    printf 'EXECUTABLES_DISTINCT=YES\nNPU_EXECUTION=NOT_RUN\nTIMING=NOT_RUN\n'
} 2>&1 | tee -a "$LOG"
