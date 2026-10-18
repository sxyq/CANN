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

# 功能：监控两个目录一级结构变化

# 定义监控目录和快照文件（可根据实际情况修改）
DIR_LIST=(
    "/usr/local/Ascend/cann/|./monitor_snapshot_1.txt"
    "/usr/local/Ascend/cann/asc/|./monitor_snapshot_2.txt"
)
RESULT_CHECK="./result_check.txt"
rm -f "$RESULT_CHECK"

# 遍历监控每个目录
for item in "${DIR_LIST[@]}"; do
    # 分割目录和快照路径
    TARGET_DIR=$(echo "$item" | cut -d'|' -f1)
    SNAPSHOT_FILE=$(echo "$item" | cut -d'|' -f2)

    # 校验目录是否存在
    if [ ! -d "$TARGET_DIR" ]; then
        echo -e "\n【错误】目录不存在：$TARGET_DIR" 2>&1 | tee -a "$RESULT_CHECK"
        continue
    fi

    # 创建临时文件
    CURRENT_SNAP=$(mktemp)
    # 生成当前目录一级结构快照
    find "$TARGET_DIR" -maxdepth 1 -mindepth 1 -printf '%y %f\n' 2>/dev/null | sort > "$CURRENT_SNAP"
    echo -e "\n========== 监控目录：$TARGET_DIR =========="

    # 首次运行：生成初始快照（仅第一次生成）
    if [ ! -f "$SNAPSHOT_FILE" ]; then
        cp "$CURRENT_SNAP" "$SNAPSHOT_FILE"
        echo "✔ 首次运行，已生成初始快照：$SNAPSHOT_FILE"
        rm "$CURRENT_SNAP"
        continue
    fi

    # 对比快照
    DIFF=$(diff "$SNAPSHOT_FILE" "$CURRENT_SNAP" 2>/dev/null)
    if [ -z "$DIFF" ]; then
        echo "✔ 目录结构未变动" 2>&1 | tee -a "$RESULT_CHECK"
    else
        echo "✘ 目录结构已变动，差异如下：" 2>&1 | tee -a "$RESULT_CHECK"
        echo "$DIFF"
    fi

    # 清理临时文件
    rm "$CURRENT_SNAP"
done

# 结构判断
if [ ! -f $RESULT_CHECK ]; then
    echo -e "\n========== 初始快照已生成 =========="
    exit 0
else
    echo -e "\n========== 目录对比完成 =========="
fi

COUNT=$(grep -c -F "目录结构未变动" "$RESULT_CHECK")
if [ "$COUNT" -eq 2 ]; then
    echo "check pass"
else
    echo "check fail"
fi
