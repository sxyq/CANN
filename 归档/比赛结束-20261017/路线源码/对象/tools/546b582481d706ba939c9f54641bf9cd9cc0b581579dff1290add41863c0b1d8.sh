#!/usr/bin/env bash
set -eo pipefail

R02_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
R02_CAPTURE="${R02_ROOT}/timing-scope-20261008"
R02_STAGE="${1:-}"
if [[ "${R02_STAGE}" != compile && "${R02_STAGE}" != pp && "${R02_STAGE}" != pc ]]; then
    printf 'usage: %s compile|pp|pc\n' "$0" >&2
    exit 2
fi
source /usr/local/Ascend/ascend-toolkit/set_env.sh >/dev/null 2>&1
cd "${R02_ROOT}"
mkdir -p "${R02_CAPTURE}"

r02_context() {
    local label="$1"
    date -u > "${R02_CAPTURE}/${label}.time.txt"
    cat /proc/loadavg > "${R02_CAPTURE}/${label}.load.txt"
    npu-smi info > "${R02_CAPTURE}/${label}.npu.txt"
    npu-smi info -t usages -i 1 > "${R02_CAPTURE}/${label}.usages.txt"
    awk '
        /HBM Capacity\(MB\)/ { capacity = $NF }
        /^[[:space:]]*HBM Usage Rate\(%\)/ { rate = $NF }
        END {
            if (capacity == "" || rate == "") exit 2
            free = int(capacity * (100 - rate) / 100)
            printf "DEVICE=1 FREE_HBM_MB=%d\n", free
            if (free < 100) exit 75
        }
    ' "${R02_CAPTURE}/${label}.usages.txt"
}

if [[ "${R02_STAGE}" == compile ]]; then
    test ! -e "${R02_CAPTURE}/compile.log"
    r02_context compile-pre
    {
        date -u
        hostname
        /usr/bin/c++ --version
        stat -c '%n %s %y' build/r02_runner build/libr02_parent.so build/libr02_candidate.so
        cat build/CMakeFiles/r02_runner.dir/flags.make
        cat build/CMakeFiles/r02_runner.dir/link.txt
        make -C build -f CMakeFiles/r02_runner.dir/build.make CMakeFiles/r02_runner.dir/build VERBOSE=1
        stat -c '%n %s %y' build/r02_runner build/libr02_parent.so build/libr02_candidate.so
        build/r02_runner --help
        date -u
    } > "${R02_CAPTURE}/compile.log" 2>&1
    cat "${R02_CAPTURE}/compile.log"
    printf 'HOST_COMPILE=PASS KERNEL_REBUILD=NO\n'
    exit 0
fi

R02_MODE=paired-parent
if [[ "${R02_STAGE}" == pc ]]; then R02_MODE=paired; fi
test ! -e "${R02_CAPTURE}/${R02_STAGE}-event.tsv"
test ! -e "${R02_CAPTURE}/${R02_STAGE}-profile"
test ! -e "${R02_CAPTURE}/${R02_STAGE}-profile.log"
r02_context "${R02_STAGE}-pre"
set +e
(
    date -u
    set -x
    msprof --output="${R02_CAPTURE}/${R02_STAGE}-profile" \
        --ai-core=off --task-time=on --ascendcl=on --runtime-api=on --aicpu=off \
        --application="${R02_ROOT}/build/r02_runner 1 20480 fp16 ${R02_MODE} - 45 21 4 ${R02_CAPTURE}/${R02_STAGE}-event.tsv"
    R02_PROFILE_RC=$?
    set +x
    date -u
    exit "${R02_PROFILE_RC}"
) > "${R02_CAPTURE}/${R02_STAGE}-profile.log" 2>&1
R02_RC=$?
set -e
cat "${R02_CAPTURE}/${R02_STAGE}-profile.log"
r02_context "${R02_STAGE}-post"
printf 'PROFILE_RC=%s MODE=%s\n' "${R02_RC}" "${R02_MODE}"
exit "${R02_RC}"
