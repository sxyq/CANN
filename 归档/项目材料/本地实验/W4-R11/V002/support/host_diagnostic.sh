#!/usr/bin/env bash
set -eo pipefail

R11_ROOT="$(cd "$(dirname "$0")/.." && pwd)"
R11_TOOLKIT=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
R11_CAPTURE="${R11_ROOT}/results/local-protocol-20261008"
R11_BUILD="${R11_ROOT}/build-server3-v002"
R11_RUNNER="${R11_BUILD}/adaptive_probe_local_pp"
R11_STAGE="${1:-}"

if [[ "${R11_STAGE}" != compile && "${R11_STAGE}" != pp && "${R11_STAGE}" != pc ]]; then
    printf 'usage: %s compile|pp|pc\n' "$0" >&2
    exit 2
fi
cd "${R11_ROOT}"
source /usr/local/Ascend/ascend-toolkit/set_env.sh >/dev/null 2>&1
mkdir -p "${R11_CAPTURE}"

r11_context() {
    local label="$1"
    date -u > "${R11_CAPTURE}/${label}.time.txt"
    cat /proc/loadavg > "${R11_CAPTURE}/${label}.load.txt"
    npu-smi info > "${R11_CAPTURE}/${label}.npu.txt"
    npu-smi info -t usages -i 1 > "${R11_CAPTURE}/${label}.usages.txt"
    awk '
        /HBM Capacity\(MB\)/ { capacity = $NF }
        /^[[:space:]]*HBM Usage Rate\(%\)/ { rate = $NF }
        END {
            if (capacity == "" || rate == "") exit 2
            free = int(capacity * (100 - rate) / 100)
            printf "DEVICE=1 FREE_HBM_MB=%d\n", free
            if (free < 100) exit 75
        }
    ' "${R11_CAPTURE}/${label}.usages.txt"
}

if [[ "${R11_STAGE}" == compile ]]; then
    test ! -e "${R11_RUNNER}"
    test ! -e "${R11_CAPTURE}/compile.log"
    r11_context compile-pre
    {
        date -u
        hostname
        /usr/bin/c++ --version
        stat -c '%n %s %y' "${R11_BUILD}/adaptive_probe" \
            "${R11_BUILD}/libadaptive_parent.so" "${R11_BUILD}/libadaptive_candidate.so"
        set -x
        /usr/bin/c++ -std=gnu++17 \
            -I"${R11_ROOT}/support" \
            -I"${R11_TOOLKIT}/aarch64-linux/include" \
            -I"${R11_TOOLKIT}/include" \
            -I"${R11_TOOLKIT}/compiler/tikcpp/tikcfw" \
            "${R11_ROOT}/support/probe.cpp" -o "${R11_RUNNER}" \
            -L"${R11_TOOLKIT}/aarch64-linux/lib64" -L"${R11_TOOLKIT}/lib64" \
            -Wl,-rpath,"${R11_TOOLKIT}/aarch64-linux/lib64:${R11_TOOLKIT}/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common:\$ORIGIN:${R11_BUILD}" \
            "${R11_BUILD}/libadaptive_parent.so" "${R11_BUILD}/libadaptive_candidate.so" \
            -lascendcl -ldl -lm -lascendc_runtime -lascendcl -lruntime -lregister \
            -lerror_manager -lprofapi -lge_common_base -lascendalog -lmmpa -ldl \
            -lascend_dump -lc_sec
        set +x
        date -u
    } > "${R11_CAPTURE}/compile.log" 2>&1
    cat "${R11_CAPTURE}/compile.log"
    printf 'HOST_COMPILE=PASS KERNEL_REBUILD=NO\n'
    exit 0
fi

R11_MODE=local-pp
if [[ "${R11_STAGE}" == pc ]]; then R11_MODE=local; fi
test -x "${R11_RUNNER}"
test ! -e "${R11_CAPTURE}/${R11_STAGE}-event.raw.tsv"
test ! -e "${R11_CAPTURE}/${R11_STAGE}-profile"
test ! -e "${R11_CAPTURE}/${R11_STAGE}-profile.log"
r11_context "${R11_STAGE}-pre"
set +e
{
    set -x
    msprof --output="${R11_CAPTURE}/${R11_STAGE}-profile" \
        --ai-core=off --task-time=on --ascendcl=on --runtime-api=on --aicpu=off \
        --application="${R11_RUNNER} --mode ${R11_MODE} --device 1 --output ${R11_CAPTURE}/${R11_STAGE}-event.raw.tsv"
} > "${R11_CAPTURE}/${R11_STAGE}-profile.log" 2>&1
R11_RC=$?
set -e
cat "${R11_CAPTURE}/${R11_STAGE}-profile.log"
r11_context "${R11_STAGE}-post"
printf 'PROFILE_RC=%s MODE=%s\n' "${R11_RC}" "${R11_MODE}"
exit "${R11_RC}"
