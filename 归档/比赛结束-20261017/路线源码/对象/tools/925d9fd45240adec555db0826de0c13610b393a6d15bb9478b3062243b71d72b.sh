#!/usr/bin/env bash
# server3 设备资源准入公共 helper。
#
# 唯一准入常量：目标 NPU FREE_HBM >= 100 MB。
#
# 本文件不判断 AICore utilization、Vector/Core busy、VLLM、其他用户进程、
# device idle、system load、lease 或 exclusive authorization。
# 这些事实只作为测量上下文记录。
# lease 只是 coordination / bookkeeping metadata，不是执行权限。

MIN_FREE_HBM_MB=100

# 读取单卡空闲 HBM（MB）。
# server3 的 `npu-smi info -t usages` 只给出 HBM Capacity 与 Usage Rate，
# 空闲量按 capacity * (100 - usage_rate) 推算。
# 取不到时返回非 0。
free_hbm_mb() {
    local device_id="$1"
    npu-smi info -t usages -i "${device_id}" 2>/dev/null | awk '
        /HBM Capacity\(MB\)/ { capacity = $NF }
        /^[[:space:]]*HBM Usage Rate\(%\)/ { rate = $NF }
        END {
            if (capacity == "" || rate == "") exit 1
            printf "%d\n", capacity * (100 - rate) / 100
        }
    '
}

# 选择一张满足 FREE_HBM >= MIN_FREE_HBM_MB 的设备。
# 用法：DEVICE="$(choose_eligible_device 0 1 2 3 4 5 6 7)"
# 找不到合格设备时返回非 0，由调用方按真实资源阻塞处理。
choose_eligible_device() {
    local device_id free
    for device_id in "$@"; do
        if free="$(free_hbm_mb "${device_id}")"; then
            if [ "${free}" -ge "${MIN_FREE_HBM_MB}" ]; then
                printf '%s\n' "${device_id}"
                return 0
            fi
        fi
    done
    return 1
}

# 记录测量上下文。负载信息只记录，不阻塞。
# 用法：record_load_context 3
record_load_context() {
    local device_id="$1"
    local free=""
    free="$(free_hbm_mb "${device_id}" 2>/dev/null || true)"
    printf 'FREE_HBM_MB=%s\n' "${free:-UNKNOWN}"
    printf 'MIN_FREE_HBM_MB=%s\n' "${MIN_FREE_HBM_MB}"
    printf 'DEVICE_LOAD:\n'
    npu-smi info -t usages -i "${device_id}" 2>/dev/null || true
    printf 'OTHER_PROCESS_PRESENT=%s\n' \
        "$(ps -eo args= 2>/dev/null | grep -E '[Vv][Ll][Ll][Mm]|EngineCore' | grep -v grep >/dev/null && echo YES || echo NO)"
    printf 'LOAD_NOTE=measurement context only\n'
}

# 作为脚本 source 时不自动执行。
if [ "${BASH_SOURCE[0]}" = "${0}" ]; then
    printf 'MIN_FREE_HBM_MB=%s\n' "${MIN_FREE_HBM_MB}"
    printf 'usage: choose_eligible_device <device-id> [<device-id> ...]\n'
    printf '       record_load_context <device-id>\n'
fi
