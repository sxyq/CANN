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

# LoadData 性能测试脚本
# 支持通过命令行参数指定场景编号
# 参考 README.md 中的 msopprof 使用说明

set -euo pipefail

# 显示帮助信息
show_help() {
    echo "LoadData 性能测试脚本"
    echo ""
    echo "使用方法:"
    echo "  $0 <SCENARIO_NUM> [PLATFORM]"
    echo ""
    echo "参数:"
    echo "  SCENARIO_NUM  测试场景编号（必选）"
    echo "  PLATFORM      平台架构（可选，默认: dav-2201）"
    echo ""
    echo "Atlas A3/A2 训练/推理平台场景 (dav-2201, 主频1800MHz):"
    echo "  1: LoadData（2D矩阵搬运）A (A矩阵 L1->L0A)"
    echo "  2: LoadData（2D矩阵搬运）B (B矩阵 L1->L0B)"
    echo "  3: LoadDataWithTranspose A (A矩阵转置)"
    echo "  4: LoadDataWithTranspose B (B矩阵转置)"
    echo "  5: LoadData（卷积数据搬运）v2 A (A矩阵 3D)"
    echo "  6: LoadData（卷积数据搬运）v2 B (B矩阵 3D)"
    echo "  7: LoadSparse (稀疏加载)"
    echo "  8: LoadBias (Bias加载)"
    echo "  9: LoadFixBuffer (FixPipe加载)"
    echo ""
    echo "Ascend 950PR/950DT 平台场景 (dav-3510, 主频1650MHz):"
    echo "  11: LoadData（2D矩阵搬运V2）A"
    echo "  12: LoadData（2D矩阵搬运V2）B"
    echo "  13: LoadData（MX矩阵搬运）A (带Scale)"
    echo "  14: LoadData（MX矩阵搬运）B (带Scale)"
    echo "  15: LoadData（卷积数据搬运）v2 A"
    echo "  16: LoadData（卷积数据搬运）v2 B"
    echo "  17: LoadDataWithTranspose B"
    echo "  18: LoadBias"
    echo "  19: LoadFixBuffer"
    echo ""
    echo "示例:"
    echo "  $0 1              # 测试场景1，默认平台dav-2201"
    echo "  $0 1 dav-2201     # 测试场景1，指定平台dav-2201"
    echo "  $0 11 dav-3510    # 测试场景11，指定平台dav-3510"
    echo ""
}

# 解析命令行参数
if [ $# -lt 1 ] || [ "$1" = "-h" ] || [ "$1" = "--help" ]; then
    show_help
    exit 0
fi

# 配置参数
SCENARIO=$1
PLATFORM=${2:-dav-2201}  # 默认平台 dav-2201

# 验证场景编号
if ! [[ "$SCENARIO" =~ ^[0-9]+$ ]]; then
    echo "错误: SCENARIO 必须是数字"
    show_help
    exit 1
fi

# 根据场景确定平台和主频
if [ "$SCENARIO" -ge 1 ] && [ "$SCENARIO" -le 9 ]; then
    PLATFORM=${2:-dav-2201}
    if [ "$PLATFORM" != "dav-2201" ]; then
        echo "错误: 场景 ${SCENARIO} 仅支持 dav-2201，当前平台为 ${PLATFORM}"
        exit 1
    fi
    FREQUENCY=1800  # Atlas A3/A2 训练/推理平台主频 1800 MHz
    echo "场景 $SCENARIO 属于 Atlas A3/A2 训练/推理平台 (dav-2201)"
    echo "主频: ${FREQUENCY} MHz"
elif [ "$SCENARIO" -ge 11 ] && [ "$SCENARIO" -le 19 ]; then
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
    echo "有效范围: 1-9 (Atlas A3/A2 训练/推理平台) 或 11-19 (Ascend 950PR/950DT 平台)"
    exit 1
fi

# Shape 配置数组（根据场景选择）
# 默认配置：测试不同数据量
SHAPES=(
    "32 32 32"      # 2KB
    "64 64 64"      # 8KB
    "64 128 64"     # 16KB
    "128 128 128"   # 32KB
    "256 128 256"   # 64KB (L0满载)
)

# 主频已在平台识别部分设置（FREQUENCY 变量）

# 根据场景确定要采集的性能指标
# 场景9/19: LoadFixBuffer，采集 aic_fixpipe_time
# 其他场景: 默认采集 aic_mte1_time
PERF_METRIC="aic_mte1_time"
PERF_METRIC_DESC="MTE1 (L1->L0)"

if [ "$SCENARIO" -eq 9 ] || [ "$SCENARIO" -eq 19 ]; then
    PERF_METRIC="aic_fixpipe_time"
    PERF_METRIC_DESC="FixPipe (L0C->GM or L1->FixPipe)"
fi

echo "性能指标: ${PERF_METRIC} (${PERF_METRIC_DESC})"

# 生成时间戳，用于创建唯一的输出目录
TIMESTAMP=$(date +"%Y%m%d_%H%M%S")
PERF_DATA_DIR="perf_data_${TIMESTAMP}_scenario${SCENARIO}"
RESULT_CSV="${PERF_DATA_DIR}/perf_result_scenario${SCENARIO}.csv"

# 颜色输出
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${GREEN}========================================${NC}"
echo -e "${GREEN}LoadData 性能测试 - 场景${SCENARIO}${NC}"
echo -e "${GREEN}平台: ${PLATFORM}${NC}"
echo -e "${GREEN}========================================${NC}"

# 创建统一的性能数据输出目录（带时间戳）
echo -e "${GREEN}创建性能数据目录: ${PERF_DATA_DIR}${NC}"
mkdir -p "${PERF_DATA_DIR}"

# 检查编译产物
if [ ! -f "build/demo" ]; then
    echo -e "${YELLOW}未找到可执行文件，开始编译...${NC}"
    mkdir -p build && cd build
    cmake -DCMAKE_ASC_ARCHITECTURES="${PLATFORM}" ..
    make -j
    cd ..
fi

# 初始化 CSV 文件
# 列说明：
# - Test_ID: 测试编号
# - M, K, N: 矩阵维度
# - Shape: 矩阵规格字符串
# - Perf_Time(us): 性能指标时间（aic_mte1_time 或 aic_fixpipe_time）
# - Cycle: 折算的 Cycle 数（Time * Frequency）
# - Bandwidth(GB/s): 数据搬运带宽
if [ "$PERF_METRIC" = "aic_fixpipe_time" ]; then
    echo "Test_ID,M,K,N,Shape,AIC_FixPipe_Time(us),Cycle,Bandwidth(GB/s)" > "${RESULT_CSV}"
else
    echo "Test_ID,M,K,N,Shape,AIC_MTE1_Time(us),Cycle,Bandwidth(GB/s)" > "${RESULT_CSV}"
fi

# 循环测试不同的 shape
test_id=1
for shape in "${SHAPES[@]}"; do
    # 解析 shape 参数
    read M K N <<< "$shape"
    shape_str="${M}_${K}_${N}"

    echo -e "${YELLOW}----------------------------------------${NC}"
    echo -e "${YELLOW}测试 ${test_id}: Shape [${M}, ${K}, ${N}]${NC}"
    echo -e "${YELLOW}----------------------------------------${NC}"

    # 清理之前的 msopprof 输出目录
    rm -rf OPPROF_* 2>/dev/null || true

    # 使用 msopprof 采集性能数据
    echo -e "${GREEN}开始 msopprof 性能采集...${NC}"
    # 检查 msopprof 执行结果。该命令可能返回非 0，不能被 set -e 提前中断。
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

    # 根据场景选择要提取的性能指标（动态搜索列名）
    perf_time="N/A"

    if [ -f "${msprof_dir}/PipeUtilization.csv" ]; then
        echo -e "${GREEN}找到 PipeUtilization.csv${NC}"

        # 显示 PipeUtilization.csv 内容（便于调试）
        echo -e "${YELLOW}PipeUtilization.csv 内容:${NC}"
        head -n 2 "${msprof_dir}/PipeUtilization.csv"

        # PipeUtilization.csv 格式：
        # 第1行：列名（如 aic_mte1_time(us), aic_fixpipe_time(us)）
        # 第2行：数据（对应列的数值）

        # 确定 CPU核类型（cube或vector）
        # PipeUtilization.csv可能有多行数据（多个block），需要找到cube行
        cpu_type="cube"
        if grep -q "cube0" "${msprof_dir}/PipeUtilization.csv"; then
            cpu_type="cube0"
        elif grep -q "cube" "${msprof_dir}/PipeUtilization.csv"; then
            cpu_type="cube"
        elif grep -q "vec" "${msprof_dir}/PipeUtilization.csv"; then
            cpu_type="vec"
        fi

        # 方法1：优先使用动态查找列索引（根据关键字搜索）
        # 搜索列名（带或不带单位）
        metric_col_index=""

        if [ "$PERF_METRIC" = "aic_fixpipe_time" ]; then
            # 场景9/19：查找 aic_fixpipe_time 列
            # 尝试多种列名格式
            metric_col_index=$(awk -F ',' 'NR==1 {for(i=1;i<=NF;i++) if($i=="aic_fixpipe_time(us)" || $i=="aic_fixpipe_time") print i}' "${msprof_dir}/PipeUtilization.csv" | head -n 1)

            if [ ! -z "$metric_col_index" ]; then
                # 提取对应行的数据（cube行）
                perf_time=$(awk -F ',' -v col="$metric_col_index" -v type="$cpu_type" '$2==type {print $col}' "${msprof_dir}/PipeUtilization.csv" | sed 's/[[:space:]]//g')
                echo -e "${GREEN}动态搜索列名: aic_fixpipe_time 列索引=${metric_col_index}${NC}"
            fi

            echo -e "${GREEN}从 PipeUtilization.csv 提取: aic_fixpipe_time = ${perf_time} us${NC}"
        else
            # 其他场景：查找 aic_mte1_time 列
            # 尝试多种列名格式
            metric_col_index=$(awk -F ',' 'NR==1 {for(i=1;i<=NF;i++) if($i=="aic_mte1_time(us)" || $i=="aic_mte1_time") print i}' "${msprof_dir}/PipeUtilization.csv" | head -n 1)

            if [ ! -z "$metric_col_index" ]; then
                # 提取对应行的数据（cube行）
                perf_time=$(awk -F ',' -v col="$metric_col_index" -v type="$cpu_type" '$2==type {print $col}' "${msprof_dir}/PipeUtilization.csv" | sed 's/[[:space:]]//g')
                echo -e "${GREEN}动态搜索列名: aic_mte1_time 列索引=${metric_col_index}${NC}"
            fi

            echo -e "${GREEN}从 PipeUtilization.csv 提取: aic_mte1_time = ${perf_time} us${NC}"
        fi

        # 方法2备用：如果动态搜索失败，尝试简单的第二行提取
        if [ -z "$perf_time" ] || [ "$perf_time" = "NA" ]; then
            echo -e "${YELLOW}动态搜索失败，尝试备用方法...${NC}"

            if [ "$PERF_METRIC" = "aic_fixpipe_time" ]; then
                # 备用：直接提取第二行，查找包含 fixpipe 的列
                perf_time=$(awk -F ',' 'NR==2 {for(i=1;i<=NF;i++) if($i ~ /^aic_fixpipe_time/ || $i ~ /^[0-9.]+$/) print $i}' "${msprof_dir}/PipeUtilization.csv" | grep -E '^[0-9.]+$' | head -n 1 | sed 's/[[:space:]]//g' || true)
            else
                # 备用：直接提取第二行，查找包含 mte1 的列
                perf_time=$(awk -F ',' 'NR==2 {for(i=1;i<=NF;i++) if($i ~ /^aic_mte1_time/ || $i ~ /^[0-9.]+$/) print $i}' "${msprof_dir}/PipeUtilization.csv" | grep -E '^[0-9.]+$' | head -n 1 | sed 's/[[:space:]]//g' || true)
            fi

            if [ ! -z "$perf_time" ] && [ "$perf_time" != "NA" ]; then
                echo -e "${GREEN}备用方法提取成功: ${perf_time} us${NC}"
            else
                echo -e "${RED}所有方法均失败${NC}"
            fi
        fi
    else
        echo -e "${YELLOW}警告: 未找到 PipeUtilization.csv${NC}"
        echo -e "${YELLOW}列出 msprof 输出目录内容:${NC}"
        ls -la "${msprof_dir}/" || true
    fi

    # 验证提取的数据是否为有效数值
    if [ "$perf_time" = "N/A" ] || [ -z "$perf_time" ] || [ "$perf_time" = "NA" ]; then
        echo -e "${RED}未能提取有效的性能数据${NC}"
        perf_time="N/A"
    elif ! [[ "$perf_time" =~ ^[0-9.]+$ ]]; then
        echo -e "${YELLOW}警告: 提取的数据格式异常: ${perf_time}${NC}"
        # 尝试清理数据（移除可能的单位或其他字符）
        perf_time=$(echo "$perf_time" | grep -oE '[0-9.]+' | head -n 1 || true)
        if [ -z "$perf_time" ]; then
            perf_time="N/A"
        fi
    fi

    # 计算性能指标
    cycle_count="N/A"
    bandwidth="N/A"

    if [ "$perf_time" != "N/A" ] && [[ "$perf_time" =~ ^[0-9.]+$ ]]; then
        # 计算 Cycle 数
        cycle_count=$(awk "BEGIN {printf \"%.2f\", ${perf_time} * ${FREQUENCY}}")

        # 根据场景计算数据搬运量
        # 默认：bfloat16 (2 bytes)
        data_type_size=2

        # 场景9/19: LoadFixBuffer
        # FixPipe搬运数据量：N * sizeof(data_type)
        # 场景9: uint64_t (8 bytes)
        # 场景19: uint64_t (8 bytes)
        if [ "$SCENARIO" -eq 9 ] || [ "$SCENARIO" -eq 19 ]; then
            data_type_size=8  # uint64_t
            data_size_bytes=$((N * data_type_size))
            data_desc="FixPipe N*sizeof(uint64_t)"
        elif [ "$SCENARIO" -eq 8 ] || [ "$SCENARIO" -eq 18 ]; then
            data_type_size=4  # float
            data_size_bytes=$((N * data_type_size))
            data_desc="Bias N*sizeof(float)"
        elif [ "$SCENARIO" -eq 7 ]; then
            data_type_size=1  # int8_t
            data_size_bytes=$((K * N * data_type_size))
            data_desc="Sparse B矩阵 K*N (int8)"
        elif [ "$SCENARIO" -eq 13 ] || [ "$SCENARIO" -eq 14 ]; then
            data_type_size=1  # fp8_e4m3fn
            # 计算数据量（根据是A还是B矩阵）
            if [ "$SCENARIO" -eq 13 ]; then
                data_size_bytes=$((M * K * data_type_size))
                data_desc="A矩阵 M*K (fp8)"
            else
                data_size_bytes=$((K * N * data_type_size))
                data_desc="B矩阵 K*N (fp8)"
            fi
        elif [ "$SCENARIO" -eq 1 ] || [ "$SCENARIO" -eq 3 ] || [ "$SCENARIO" -eq 5 ] || [ "$SCENARIO" -eq 11 ] || [ "$SCENARIO" -eq 15 ]; then
            # A矩阵：M*K
            data_size_bytes=$((M * K * data_type_size))
            data_desc="A矩阵 M*K"
        elif [ "$SCENARIO" -eq 2 ] || [ "$SCENARIO" -eq 4 ] || [ "$SCENARIO" -eq 6 ] || [ "$SCENARIO" -eq 12 ] || [ "$SCENARIO" -eq 16 ] || [ "$SCENARIO" -eq 17 ]; then
            # B矩阵：K*N
            data_size_bytes=$((K * N * data_type_size))
            data_desc="B矩阵 K*N"
        else
            # 默认：A矩阵
            data_size_bytes=$((M * K * data_type_size))
            data_desc="默认 A矩阵 M*K"
        fi

        # 计算带宽
        bandwidth=$(awk "BEGIN {printf \"%.3f\", ${data_size_bytes} / ${perf_time} / 1e3}")

        echo -e "${GREEN}性能指标计算:${NC}"
        echo -e "${GREEN}  性能指标: ${PERF_METRIC} (${PERF_METRIC_DESC})${NC}"
        echo -e "${GREEN}  数据类型大小: ${data_type_size} bytes${NC}"
        echo -e "${GREEN}  数据量: ${data_size_bytes} bytes (${data_desc})${NC}"
        echo -e "${GREEN}  Cycle数: ${cycle_count} cycles${NC}"
        echo -e "${GREEN}  带宽: ${bandwidth} GB/s${NC}"
    fi

    # 记录结果到 CSV
    echo "${test_id},${M},${K},${N},${shape_str},${perf_time},${cycle_count},${bandwidth}" >> "${RESULT_CSV}"

    # 显示性能指标名称
    perf_metric_name="AIC_MTE1_Time"
    if [ "$PERF_METRIC" = "aic_fixpipe_time" ]; then
        perf_metric_name="AIC_FixPipe_Time"
    fi

    echo -e "${GREEN}测试 ${test_id} 完成${NC}"
    echo -e "${GREEN}  Shape: [${M}, ${K}, ${N}]${NC}"
    echo -e "${GREEN}  ${perf_metric_name}: ${perf_time} us${NC}"
    echo -e "${GREEN}  Cycle: ${cycle_count} cycles${NC}"
    echo -e "${GREEN}  Bandwidth: ${bandwidth} GB/s${NC}"

    # 保存 msopprof 输出目录
    if [ "$perf_time" != "N/A" ]; then
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

# 显示 CSV 文件内容（表格格式）
echo -e "\n${YELLOW}性能数据表格:${NC}"
if command -v column >/dev/null 2>&1; then
    column -t -s ',' "${RESULT_CSV}"
else
    cat "${RESULT_CSV}"
fi

# 提示用户生成 Roofline 图
echo -e "\n${YELLOW}提示：${NC}"
echo -e "${YELLOW}本次测试所有数据已保存到: ${PERF_DATA_DIR}${NC}"
echo -e "${YELLOW}如需生成 Roofline 图，可使用:${NC}"
echo -e "${YELLOW}  python3 generate_roofline_with_latency.py --csv ${RESULT_CSV}${NC}"
