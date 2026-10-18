#!/bin/bash
# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------

# Mmad（Cube 矩阵乘）性能测试脚本（仿真环境）
# 支持通过命令行参数指定场景编号
# 使用 msopprof simulator 采集，从 simulator/core0.cubecore0/ 提取 MMAD 指令的
# dur（持续时间，us）与 cycles，写入 CSV
# 性能（MAC/cycle）与算力利用率由 generate_roofline.py 基于硬件并行度从 CSV 推导

set -euo pipefail

# 显示帮助信息
show_help() {
    echo "Mmad 性能测试脚本"
    echo ""
    echo "使用方法:"
    echo "  $0 <SCENARIO_NUM> [PLATFORM]"
    echo ""
    echo "参数:"
    echo "  SCENARIO_NUM  测试场景编号（必选）"
    echo "  PLATFORM      平台架构（可选，默认按场景号推导）"
    echo ""
    echo "Atlas A3/A2 训练/推理平台场景 (dav-2201, 主频1800MHz):"
    echo "  1: Mmad b8  (int8 * int8 -> int32)"
    echo "  2: Mmad b16 (half * half -> float)"
    echo "  3: Mmad b32 (float * float -> float)"
    echo "  4: MmadWithSparse b8 (int8 * int8 -> int32, 4:2 结构化稀疏)"
    echo ""
    echo "Ascend 950PR/950DT 平台场景 (dav-3510, 主频1650MHz):"
    echo "  11: Mmad b8  (int8 * int8 -> int32)"
    echo "  12: Mmad b16 (half * half -> float)"
    echo "  13: Mmad b32 (float * float -> float)"
    echo "  14: MmadMx mxfp8 (fp8_e4m3fn * fp8_e4m3fn -> float, scale fp8_e8m0)"
    echo "  15: MmadMx mxfp4 (fp4x2_e2m1 * fp4x2_e2m1 -> float, scale fp8_e8m0)"
    echo ""
    echo "示例:"
    echo "  $0 1              # 测试场景1，默认平台dav-2201"
    echo "  $0 11 dav-3510    # 测试场景11，指定平台dav-3510"
    echo ""
}

# 解析命令行参数
if [ $# -lt 1 ] || [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    show_help
    exit 0
fi

SCENARIO=$1

# 验证场景编号为数字
if ! [[ "$SCENARIO" =~ ^[0-9]+$ ]]; then
    echo "错误: SCENARIO 必须是数字"
    show_help
    exit 1
fi

# 根据场景确定平台和主频
if [ "$SCENARIO" -ge 1 ] && [ "$SCENARIO" -le 4 ]; then
    PLATFORM=${2:-dav-2201}
    if [ "$PLATFORM" != "dav-2201" ]; then
        echo "错误: 场景 ${SCENARIO} 仅支持 dav-2201，当前平台为 ${PLATFORM}"
        exit 1
    fi
    FREQUENCY=1800  # Atlas A3/A2 训练/推理平台主频 1800 MHz
    SIM_MODEL=Ascend910B3  # dav-2201 对应的仿真器型号
    echo "场景 $SCENARIO 属于 Atlas A3/A2 训练/推理平台 (dav-2201)"
    echo "主频: ${FREQUENCY} MHz"
elif [ "$SCENARIO" -ge 11 ] && [ "$SCENARIO" -le 15 ]; then
    PLATFORM=${2:-dav-3510}
    if [ "$PLATFORM" != "dav-3510" ]; then
        echo "错误: 场景 ${SCENARIO} 仅支持 dav-3510，当前平台为 ${PLATFORM}"
        exit 1
    fi
    FREQUENCY=1650  # Ascend 950PR/950DT 平台主频 1650 MHz
    SIM_MODEL=Ascend950PR_9589  # dav-3510 对应的仿真器型号
    echo "场景 $SCENARIO 属于 Ascend 950PR/950DT 平台 (dav-3510)"
    echo "主频: ${FREQUENCY} MHz"
else
    echo "错误: 无效的场景编号 $SCENARIO"
    echo "有效范围: 1-4 (Atlas A3/A2 训练/推理平台) 或 11-15 (Ascend 950PR/950DT 平台)"
    exit 1
fi

# Shape 配置数组（按场景精度选择，见方案设计 §4.2）
# 统一固定 M=N，逐步增大 K 直至 L0A/L0B 满载（不同精度满载点不同）
case "$SCENARIO" in
    1)
        # b8 dav-2201：L0A/L0B 在 K=512 时满载（M=N=128）
        SHAPES=("32 32 32" "64 64 64" "128 128 128" "128 256 128" "128 512 128")
        ;;
    11)
        # b8 dav-3510：末组 256 256 256 同时满载 L0A(64KB)/L0C(256KB)
        SHAPES=("32 32 32" "64 64 64" "128 128 128" "128 256 128" "256 256 256")
        ;;
    2)
        # b16 dav-2201：L0A/L0B 在 K=256 时满载（M=N=128）
        SHAPES=("32 32 32" "64 64 64" "64 128 64" "128 128 128" "128 256 128")
        ;;
    12)
        # b16 dav-3510：末组 256 128 256 同时满载 L0A/L0B/L0C
        SHAPES=("32 32 32" "64 64 64" "64 128 64" "128 128 128" "256 128 256")
        ;;
    3|13)
        # b32：L0A 在 M·K=16384 时满载（128 128 128），MN 撑不到 256，两架构均保 4 组
        SHAPES=("32 32 32" "64 64 64" "64 128 64" "128 128 128")
        ;;
    4)
        # sparse b8 dav-2201：K 取 64 倍数，L0A/L0B 在 K=512 时满载
        SHAPES=("64 64 64" "64 128 64" "128 128 128" "128 256 128" "128 512 128")
        ;;
    14)
        # mxfp8 dav-3510：K 取 64 倍数，末组 256 256 256 满载 L0A/L0C
        SHAPES=("64 64 64" "64 128 64" "128 128 128" "128 256 128" "256 256 256")
        ;;
    15)
        # mxfp4 dav-3510：fp4 打包，K 取 64 倍数，末组 256 512 256 同时满载 L0A/L0B/L0C
        SHAPES=("64 64 64" "128 128 128" "128 256 128" "128 512 128" "256 512 256")
        ;;
esac

# 生成时间戳，创建唯一输出目录
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
PERF_DATA_DIR="perf_data_${TIMESTAMP}_scenario${SCENARIO}"
RESULT_CSV="${PERF_DATA_DIR}/perf_result_scenario${SCENARIO}.csv"

# 颜色输出
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}Mmad 性能测试 - 场景${SCENARIO}${NC}"
echo -e "${GREEN}平台: ${PLATFORM}${NC}"
echo -e "${GREEN}========================================${NC}"

# 创建性能数据输出目录
echo -e "${GREEN}创建性能数据目录: ${PERF_DATA_DIR}${NC}"
mkdir -p "${PERF_DATA_DIR}"

# 配置仿真器动态库路径（按平台选择对应型号的仿真器）
export LD_LIBRARY_PATH="${ASCEND_HOME_PATH}/tools/simulator/${SIM_MODEL}/lib:${LD_LIBRARY_PATH:-}"

# 检查编译产物，不存在则按当前平台仿真编译
if [ ! -f "build/demo" ]; then
    echo -e "${YELLOW}未找到可执行文件，开始仿真编译...${NC}"
    mkdir -p build && cd build
    cmake -DCMAKE_ASC_ARCHITECTURES="${PLATFORM}" \
          -DCMAKE_ASC_RUN_MODE=sim \
          -DASC_DIR="${ASC_DIR:-${ASCEND_HOME_PATH}/lib64/cmake}" ..
    make -j
    cd ..
fi

# 初始化 CSV 文件
# 列说明：
# - Test_ID: 测试编号
# - M, K, N: 矩阵维度
# - Shape: 矩阵规格字符串
# - MMAD_Dur(us): MMAD 指令持续时间（取自 trace.json 的 dur，高精度）
# - Cycles: MMAD 指令周期数（取自 instr_exe.csv 的 cycles 列）
# 性能与算力利用率由 generate_roofline.py 基于硬件并行度从本 CSV 推导，此处不重复计算
echo "Test_ID,M,K,N,Shape,MMAD_Dur(us),Cycles" > "${RESULT_CSV}"

# 循环测试不同的 shape
test_id=1
for shape in "${SHAPES[@]}"; do
    read M K N <<< "$shape"
    shape_str="${M}_${K}_${N}"

    echo -e "${YELLOW}----------------------------------------${NC}"
    echo -e "${YELLOW}测试 ${test_id}: Shape [${M}, ${K}, ${N}]${NC}"
    echo -e "${YELLOW}----------------------------------------${NC}"

    # 清理之前的 msopprof 输出目录
    rm -rf OPPROF_* 2>/dev/null || true

    # 使用 msopprof simulator 采集性能数据。msopprof 可能返回非 0，不能被 set -e 提前中断。
    echo -e "${GREEN}开始 msopprof simulator 性能采集...${NC}"
    set +e
    msopprof simulator build/demo "${SCENARIO}" "${M}" "${K}" "${N}" > /dev/null 2>&1
    msprof_exit_code=$?
    set -e
    if [ "${msprof_exit_code}" -ne 0 ]; then
        echo -e "${RED}msprof 执行失败（退出码: ${msprof_exit_code}）${NC}"
        echo "${test_id},${M},${K},${N},${shape_str},ERROR,ERROR,ERROR" >> "${RESULT_CSV}"
        test_id=$((test_id + 1))
        continue
    fi

    # 查找 msopprof 生成的性能数据目录
    msprof_dir=$(ls -dt OPPROF_* 2>/dev/null | head -n 1 || true)

    if [ -z "$msprof_dir" ] || [ ! -d "$msprof_dir" ]; then
        echo -e "${RED}未找到 msprof 输出目录 OPPROF_*${NC}"
        echo "${test_id},${M},${K},${N},${shape_str},N/A,N/A,N/A" >> "${RESULT_CSV}"
        test_id=$((test_id + 1))
        continue
    fi

    echo -e "${GREEN}msprof 输出目录: ${msprof_dir}${NC}"

    # 从 simulator/core0.cubecore0/ 提取 MMAD 指令的 dur（us）与 cycles
    mmad_dur="N/A"
    mmad_cycles="N/A"
    instr_csv="${msprof_dir}/simulator/core0.cubecore0/core0.cubecore0_instr_exe.csv"
    trace_json="${msprof_dir}/simulator/core0.cubecore0/trace.json"

    # cycles：取自 instr_exe.csv 中 MMAD 系列指令（MMAD/MMAD_SP/MMAD_MX 等）CUBE 行的 cycles 列（第 5 列）
    if [ -f "${instr_csv}" ]; then
        echo -e "${GREEN}找到 ${instr_csv}${NC}"
        mmad_cycles=$(awk -F ',' '$1 ~ /^MMAD/ && $3=="CUBE" {print $5; exit}' "${instr_csv}" | sed 's/[[:space:]]//g')
        mmad_instr=$(awk -F ',' '$1 ~ /^MMAD/ && $3=="CUBE" {print $1; exit}' "${instr_csv}" | sed 's/[[:space:]]//g')
    else
        echo -e "${YELLOW}警告: 未找到 core0.cubecore0_instr_exe.csv${NC}"
    fi

    # dur：优先取 trace.json 中 MMAD 系列事件的高精度 dur（us），无则回退 instr_exe.csv 的 running_time 列（第 6 列）
    if [ -f "${trace_json}" ] && [ -n "${mmad_instr:-}" ]; then
        mmad_dur=$(grep -o "\"name\"[: ]*\"${mmad_instr}\"[^}]*\"dur\"[: ]*[0-9.]*\|\"dur\"[: ]*[0-9.]*[^}]*\"name\"[: ]*\"${mmad_instr}\"" "${trace_json}" 2>/dev/null \
                   | grep -o '"dur"[: ]*[0-9.]*' | grep -o '[0-9.]\+' | head -n 1)
    fi
    if [ -z "${mmad_dur}" ] || [ "${mmad_dur}" = "N/A" ]; then
        if [ -f "${instr_csv}" ]; then
            mmad_dur=$(awk -F ',' '$1 ~ /^MMAD/ && $3=="CUBE" {print $6; exit}' "${instr_csv}" | sed 's/[[:space:]]//g')
        fi
    fi

    [ -z "${mmad_dur}" ] && mmad_dur="N/A"
    [ -z "${mmad_cycles}" ] && mmad_cycles="N/A"
    echo -e "${GREEN}提取 MMAD: dur=${mmad_dur} us, cycles=${mmad_cycles}${NC}"

    # 记录结果到 CSV
    echo "${test_id},${M},${K},${N},${shape_str},${mmad_dur},${mmad_cycles}" >> "${RESULT_CSV}"

    echo -e "${GREEN}测试 ${test_id} 完成${NC}"
    echo -e "${GREEN}  Shape: [${M}, ${K}, ${N}]${NC}"
    echo -e "${GREEN}  MMAD_Dur: ${mmad_dur} us${NC}"
    echo -e "${GREEN}  Cycles: ${mmad_cycles}${NC}"

    # 归档本轮 msopprof 输出目录
    if [ "${mmad_dur}" != "N/A" ] || [ "${mmad_cycles}" != "N/A" ]; then
        mv "${msprof_dir}" "${PERF_DATA_DIR}/test_${test_id}_${shape_str}" 2>/dev/null || true
    fi

    test_id=$((test_id + 1))
done

# 输出汇总结果
echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}性能测试汇总结果${NC}"
echo -e "${GREEN}========================================${NC}"
cat "${RESULT_CSV}"

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}测试完成！${NC}"
echo -e "${GREEN}场景: ${SCENARIO}${NC}"
echo -e "${GREEN}平台: ${PLATFORM}${NC}"
echo -e "${GREEN}结果文件: ${RESULT_CSV}${NC}"
echo -e "${GREEN}性能数据目录: ${PERF_DATA_DIR}${NC}"
echo -e "${GREEN}========================================${NC}"

# 表格格式展示
echo -e "\n${YELLOW}性能数据表格:${NC}"
if command -v column >/dev/null 2>&1; then
    column -t -s ',' "${RESULT_CSV}"
else
    cat "${RESULT_CSV}"
fi
