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

# Fixpipe（L0C 搬出）性能测试脚本
# 提取 PipeUtilization.csv 的 aic_fixpipe_time(us)（Fixpipe 搬出耗时），计算搬出带宽 GB/s

set -euo pipefail

# 显示帮助信息
show_help() {
    echo "Fixpipe 性能测试脚本"
    echo ""
    echo "使用方法:"
    echo "  $0 <SCENARIO_NUM> [PLATFORM]"
    echo ""
    echo "参数:"
    echo "  SCENARIO_NUM  测试场景编号（必选）"
    echo "  PLATFORM      平台架构（可选，默认按场景号推导）"
    echo ""
    echo "Atlas A3/A2 训练/推理平台场景 (dav-2201, 主频1800MHz):"
    echo "  1: L0C->L1 DataCopy float -> half"
    echo "  2: L0C->L1 DataCopy float -> int8_t"
    echo ""
    echo "Ascend 950PR/950DT 平台场景 (dav-3510, 主频1650MHz):"
    echo "  11: L0C->L1 DataCopy float -> half"
    echo "  12: L0C->L1 DataCopy float -> int8_t"
    echo "  13: L0C->UB Fixpipe  float -> float 非双目标模式"
    echo "  14: L0C->UB Fixpipe  float -> float 双目标模式（按 M 拆分）"
    echo ""
    echo "示例:"
    echo "  $0 1              # 测试场景1，默认平台dav-2201"
    echo "  $0 13 dav-3510    # 测试场景13，指定平台dav-3510"
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
if [ "$SCENARIO" -ge 1 ] && [ "$SCENARIO" -le 2 ]; then
    PLATFORM=${2:-dav-2201}
    if [ "$PLATFORM" != "dav-2201" ]; then
        echo "错误: 场景 ${SCENARIO} 仅支持 dav-2201，当前平台为 ${PLATFORM}"
        exit 1
    fi
    FREQUENCY=1800  # Atlas A3/A2 训练/推理平台主频 1800 MHz
    echo "场景 $SCENARIO 属于 Atlas A3/A2 训练/推理平台 (dav-2201)"
    echo "主频: ${FREQUENCY} MHz"
elif [ "$SCENARIO" -ge 11 ] && [ "$SCENARIO" -le 14 ]; then
    PLATFORM=${2:-dav-3510}
    if [ "$PLATFORM" != "dav-3510" ]; then
        echo "错误: 场景 ${SCENARIO} 仅支持 dav-3510，当前平台为 ${PLATFORM}"
        exit 1
    fi
    FREQUENCY=1650  # Ascend 950PR/950DT 平台主频 1650 MHz
    echo "场景 $SCENARIO 属于 Ascend 950PR/950DT 平台 (dav-3510)"
    echo "主频: ${FREQUENCY} MHz"
else
    echo "错误: 无效的场景编号 $SCENARIO"
    echo "有效范围: 1-2 (Atlas A3/A2 训练/推理平台) 或 11-14 (Ascend 950PR/950DT 平台)"
    exit 1
fi

# 根据场景确定目的数据类型大小（用于带宽计算的搬出量 = M * N * sizeof(目的类型)）
# 场景 1/11：half（2 字节）；场景 2/12：int8_t（1 字节）；场景 13/14：float（4 字节）
case "$SCENARIO" in
    1|11) DST_TYPE_SIZE=2; DST_TYPE_DESC="half" ;;
    2|12) DST_TYPE_SIZE=1; DST_TYPE_DESC="int8_t" ;;
    13|14) DST_TYPE_SIZE=4; DST_TYPE_DESC="float" ;;
esac

# Shape 配置数组（见方案设计 §4.2）
# 搬出性能只与 M、N 相关，K 固定取 64（仅用于让 Mmad 前置产出 L0C 数据）
# 从很小的 M*N 起步、逐步增大至满载（L0C 存 float，M*N*4 <= L0C 容量）：
#   dav-2201 L0C 128KB -> M*N <= 32768，末组 128x256；
#   dav-3510 L0C 256KB -> M*N <= 65536，末组 256x256。
if [ "$PLATFORM" = "dav-3510" ]; then
    SHAPES=(
        "16 64 16"      # M*N=256,   1 KB
        "32 64 32"      # M*N=1024,  4 KB
        "64 64 64"      # M*N=4096,  16 KB
        "128 64 128"    # M*N=16384, 64 KB
        "256 64 256"    # M*N=65536, 256 KB (dav-3510 满载)
    )
else
    SHAPES=(
        "16 64 16"      # M*N=256,   1 KB
        "32 64 32"      # M*N=1024,  4 KB
        "64 64 64"      # M*N=4096,  16 KB
        "128 64 128"    # M*N=16384, 64 KB
        "128 64 256"    # M*N=32768, 128 KB (dav-2201 满载)
    )
fi

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
echo -e "${GREEN}Fixpipe 性能测试 - 场景${SCENARIO}${NC}"
echo -e "${GREEN}平台: ${PLATFORM}，目的类型: ${DST_TYPE_DESC}${NC}"
echo -e "${GREEN}========================================${NC}"

# 创建性能数据输出目录
echo -e "${GREEN}创建性能数据目录: ${PERF_DATA_DIR}${NC}"
mkdir -p "${PERF_DATA_DIR}"

# 检查编译产物，不存在则按当前平台编译
if [ ! -f "build/demo" ]; then
    echo -e "${YELLOW}未找到可执行文件，开始编译...${NC}"
    mkdir -p build && cd build
    cmake -DCMAKE_ASC_ARCHITECTURES="${PLATFORM}" \
          -DASC_DIR="${ASC_DIR:-${ASCEND_HOME_PATH}/lib64/cmake}" ..
    make -j
    cd ..
fi

# 初始化 CSV 文件
# 列说明：
# - Test_ID: 测试编号
# - M, K, N: 矩阵维度
# - Shape: 矩阵规格字符串
# - AIC_FixPipe_Time(us): Fixpipe 搬出耗时（aic_fixpipe_time）
# - Cycle: 折算 Cycle 数（Time * Frequency）
# - Bandwidth(GB/s): 搬出带宽（M*N*sizeof(目的类型) / Time / 1e3）
echo "Test_ID,M,K,N,Shape,AIC_FixPipe_Time(us),Cycle,Bandwidth(GB/s)" > "${RESULT_CSV}"

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

    # 使用 msopprof 采集性能数据。msopprof 可能返回非 0，不能被 set -e 提前中断。
    echo -e "${GREEN}开始 msopprof 性能采集...${NC}"
    set +e
    msopprof build/demo "${SCENARIO}" "${M}" "${K}" "${N}" > /dev/null 2>&1
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
        echo -e "${RED}未找到 msopprof 输出目录 OPPROF_*${NC}"
        echo "${test_id},${M},${K},${N},${shape_str},N/A,N/A,N/A" >> "${RESULT_CSV}"
        test_id=$((test_id + 1))
        continue
    fi

    echo -e "${GREEN}msprof 输出目录: ${msprof_dir}${NC}"

    # 从 PipeUtilization.csv 提取 aic_fixpipe_time(us)（Fixpipe 搬出耗时）
    fixpipe_time="N/A"

    if [ -f "${msprof_dir}/PipeUtilization.csv" ]; then
        echo -e "${GREEN}找到 PipeUtilization.csv${NC}"
        echo -e "${YELLOW}PipeUtilization.csv 内容:${NC}"
        head -n 2 "${msprof_dir}/PipeUtilization.csv"

        # 确定 cube 行类型（可能为 cube0 / cube）
        cpu_type="cube"
        if grep -q "cube0" "${msprof_dir}/PipeUtilization.csv"; then
            cpu_type="cube0"
        elif grep -q "cube" "${msprof_dir}/PipeUtilization.csv"; then
            cpu_type="cube"
        fi

        # 方法1：动态查找 aic_fixpipe_time 列索引（兼容带/不带单位）
        metric_col_index=$(awk -F ',' 'NR==1 {for(i=1;i<=NF;i++) if($i=="aic_fixpipe_time(us)" || $i=="aic_fixpipe_time") print i}' "${msprof_dir}/PipeUtilization.csv" | head -n 1)

        if [ ! -z "$metric_col_index" ]; then
            fixpipe_time=$(awk -F ',' -v col="$metric_col_index" -v type="$cpu_type" '$2==type {print $col}' "${msprof_dir}/PipeUtilization.csv" | sed 's/[[:space:]]//g')
            echo -e "${GREEN}动态搜索列名: aic_fixpipe_time 列索引=${metric_col_index}${NC}"
        fi

        # 方法2备用：动态搜索失败时，从数据行提取首个合法数值兜底
        if [ -z "$fixpipe_time" ] || [ "$fixpipe_time" = "NA" ]; then
            echo -e "${YELLOW}动态搜索失败，尝试备用方法...${NC}"
            fixpipe_time=$(awk -F ',' 'NR==2 {for(i=1;i<=NF;i++) if($i ~ /^[0-9.]+$/) print $i}' "${msprof_dir}/PipeUtilization.csv" | grep -E '^[0-9.]+$' | head -n 1 | sed 's/[[:space:]]//g' || true)
            if [ ! -z "$fixpipe_time" ] && [ "$fixpipe_time" != "NA" ]; then
                echo -e "${GREEN}备用方法提取成功: ${fixpipe_time} us${NC}"
            else
                echo -e "${RED}所有方法均失败${NC}"
            fi
        fi

        echo -e "${GREEN}从 PipeUtilization.csv 提取: aic_fixpipe_time = ${fixpipe_time} us${NC}"
    else
        echo -e "${YELLOW}警告: 未找到 PipeUtilization.csv${NC}"
        ls -la "${msprof_dir}/" || true
    fi

    # 验证提取的数据是否为有效数值
    if [ "$fixpipe_time" = "N/A" ] || [ -z "$fixpipe_time" ] || [ "$fixpipe_time" = "NA" ]; then
        echo -e "${RED}未能提取有效的性能数据${NC}"
        fixpipe_time="N/A"
    elif ! [[ "$fixpipe_time" =~ ^[0-9.]+$ ]]; then
        echo -e "${YELLOW}警告: 提取的数据格式异常: ${fixpipe_time}${NC}"
        fixpipe_time=$(echo "$fixpipe_time" | grep -oE '[0-9.]+' | head -n 1 || true)
        if [ -z "$fixpipe_time" ]; then
            fixpipe_time="N/A"
        fi
    fi

    # 计算派生指标
    cycle_count="N/A"
    bandwidth="N/A"

    if [ "$fixpipe_time" != "N/A" ] && [[ "$fixpipe_time" =~ ^[0-9.]+$ ]]; then
        # Cycle = Time(us) * Frequency(MHz)
        cycle_count=$(awk "BEGIN {printf \"%.2f\", ${fixpipe_time} * ${FREQUENCY}}")

        # 搬出量 = M * N * sizeof(目的类型)；带宽 = 搬出量 / Time(us) / 1e3 (GB/s)
        data_size_bytes=$((M * N * DST_TYPE_SIZE))
        bandwidth=$(awk "BEGIN {printf \"%.3f\", ${data_size_bytes} / ${fixpipe_time} / 1e3}")

        echo -e "${GREEN}性能指标计算:${NC}"
        echo -e "${GREEN}  Fixpipe 搬出耗时: ${fixpipe_time} us${NC}"
        echo -e "${GREEN}  搬出量: ${data_size_bytes} bytes (M*N*sizeof(${DST_TYPE_DESC}))${NC}"
        echo -e "${GREEN}  Cycle数: ${cycle_count} cycles${NC}"
        echo -e "${GREEN}  带宽: ${bandwidth} GB/s${NC}"
    fi

    # 记录结果到 CSV
    echo "${test_id},${M},${K},${N},${shape_str},${fixpipe_time},${cycle_count},${bandwidth}" >> "${RESULT_CSV}"

    echo -e "${GREEN}测试 ${test_id} 完成${NC}"
    echo -e "${GREEN}  Shape: [${M}, ${K}, ${N}]${NC}"
    echo -e "${GREEN}  AIC_FixPipe_Time: ${fixpipe_time} us${NC}"
    echo -e "${GREEN}  Cycle: ${cycle_count} cycles${NC}"
    echo -e "${GREEN}  Bandwidth: ${bandwidth} GB/s${NC}"

    # 归档本轮 msopprof 输出目录
    if [ "$fixpipe_time" != "N/A" ]; then
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
