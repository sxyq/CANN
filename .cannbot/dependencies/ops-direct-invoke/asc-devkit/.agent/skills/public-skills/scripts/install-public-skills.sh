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

set -e

# 脚本功能：安装或更新项目公共 skills
PUBLIC_SKILLS=("gitcode-pr" "gitcode-issue" "api-doc-generator" "gitcode-pipeline")

CONNECT_TIMEOUT=5
CLONE_TIMEOUT=20
REPO_URL="https://gitcode.com/cann-agent/skills.git"

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SKILLS_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
REMOTE_DIR="$SKILLS_DIR/_remote"
GITIGNORE="$(cd "$SCRIPT_DIR/../../.." && pwd)/.gitignore"

mkdir -p "$REMOTE_DIR"

if command -v mktemp &> /dev/null; then
    TEMP_DIR=$(mktemp -d)
else
    TEMP_DIR="/tmp/skills_install_$$"
    mkdir -p "$TEMP_DIR"
fi
trap 'rm -rf "$TEMP_DIR"' EXIT

echo "Checking repository accessibility..."
check_connectivity() {
    if command -v curl &> /dev/null; then
        http_code=$(curl --connect-timeout $CONNECT_TIMEOUT --max-time $CONNECT_TIMEOUT -s -o /dev/null \
            -w "%{http_code}" "https://gitcode.com" 2>/dev/null)
        if [ "$http_code" = "200" ] || [ "$http_code" = "301" ] || [ "$http_code" = "302" ]; then
            return 0
        fi
        return 1
    elif command -v wget &> /dev/null; then
        wget --timeout=$CONNECT_TIMEOUT -q --spider "https://gitcode.com" 2>/dev/null
        return $?
    else
        GIT_HTTP_LOW_SPEED_LIMIT=1 GIT_HTTP_LOW_SPEED_TIME=$CONNECT_TIMEOUT \
            git ls-remote "$REPO_URL" HEAD &>/dev/null
        return $?
    fi
}

if ! check_connectivity; then
    echo "Error: Cannot access gitcode.com, please check network connectivity"
    exit 1
fi

echo "Cloning skills repository..."
if command -v timeout &> /dev/null; then
    timeout $CLONE_TIMEOUT git clone --depth 1 "$REPO_URL" "$TEMP_DIR/skills" 2>&1
    if [ $? -ne 0 ]; then
        echo "Error: Failed to clone skills repository"
        exit 1
    fi
else
    GIT_HTTP_LOW_SPEED_LIMIT=1000
    GIT_HTTP_LOW_SPEED_TIME=$CLONE_TIMEOUT
    export GIT_HTTP_LOW_SPEED_LIMIT GIT_HTTP_LOW_SPEED_TIME
    git clone --depth 1 "$REPO_URL" "$TEMP_DIR/skills" 2>&1
    if [ $? -ne 0 ]; then
        echo "Error: Failed to clone skills repository"
        exit 1
    fi
fi

if [ ! -d "$TEMP_DIR/skills/skills" ]; then
    echo "Error: skills directory not found in repository"
    exit 1
fi

echo "Installing skills..."
for skill in "${PUBLIC_SKILLS[@]}"; do
    if [ -d "$TEMP_DIR/skills/skills/$skill" ]; then
        rm -rf "$REMOTE_DIR/$skill"
        cp -r "$TEMP_DIR/skills/skills/$skill" "$REMOTE_DIR/"
        rm -rf "$SKILLS_DIR/$skill"
        ln -sfn "_remote/$skill" "$SKILLS_DIR/$skill"
        echo "Installed skill: $skill"
    else
        echo "Warning: Skill '$skill' not found in repository"
    fi
done

if [ -f "$GITIGNORE" ]; then
    if ! grep -qxF ".agent/skills/_remote/" "$GITIGNORE"; then
        echo ".agent/skills/_remote/" >> "$GITIGNORE"
    fi

    for skill in "${PUBLIC_SKILLS[@]}"; do
        ENTRY=".agent/skills/$skill"
        if ! grep -qxF "$ENTRY" "$GITIGNORE"; then
            echo "$ENTRY" >> "$GITIGNORE"
        fi
    done
fi

echo "All skills installed successfully."
