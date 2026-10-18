#!/usr/bin/env bash
# ----------------------------------------------------------------------------------------------------------
# Copyright (c) 2026 Huawei Technologies Co., Ltd.
# This program is free software, you can redistribute it and/or modify it under the terms and conditions of
# CANN Open Software License Agreement Version 2.0 (the "License").
# Please refer to the License for details. You may not use this file except in compliance with the License.
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND, EITHER EXPRESS OR IMPLIED,
# INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT, MERCHANTABILITY, OR FITNESS FOR A PARTICULAR PURPOSE.
# See LICENSE in the root of the software repository for the full text of the License.
# ----------------------------------------------------------------------------------------------------------

set -euo pipefail

TEST_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "${TEST_DIR}/../../../.." && pwd)"
SKILLS_DIR="${REPO_ROOT}/.agent/skills"
PUBLIC_SKILL_FILE="${SKILLS_DIR}/public-skills/SKILL.md"
INSTALL_SCRIPT="${SKILLS_DIR}/public-skills/scripts/install-public-skills.sh"
REMOTE_SKILL_FILE="${SKILLS_DIR}/_remote/gitcode-pr/SKILL.md"
AGENT_README="${REPO_ROOT}/.agent/README.md"
GITIGNORE="${REPO_ROOT}/.gitignore"

fail() {
    echo "[FAIL] $*" >&2
    exit 1
}

expect_fixed() {
    local file="$1"
    local text="$2"
    local label="$3"
    grep -Fq -- "$text" "$file" || fail "missing content: ${label}"
}

if [[ ! -f "$REMOTE_SKILL_FILE" ]]; then
    bash "$INSTALL_SCRIPT"
fi

expect_fixed "$PUBLIC_SKILL_FILE" "安装公共skills步骤" "public skill install workflow"
expect_fixed "$PUBLIC_SKILL_FILE" "install-public-skills.sh" "public skill install script reference"
expect_fixed "$PUBLIC_SKILL_FILE" "PUBLIC_SKILLS" "public skill list reference"
expect_fixed "$PUBLIC_SKILL_FILE" "## 更新规则" "public skill update rules"
expect_fixed "$PUBLIC_SKILL_FILE" "每次执行安装脚本都会重新克隆远端仓库" "public skill reclone update rule"
expect_fixed "$PUBLIC_SKILL_FILE" "先删除本地缓存中的同名目录" "public skill replace-cache update rule"
expect_fixed "$PUBLIC_SKILL_FILE" "重新创建一级入口软链" "public skill symlink update rule"
expect_fixed "$PUBLIC_SKILL_FILE" "https://gitcode.com/cann-agent/skills.git" "remote skills repository"
expect_fixed "$PUBLIC_SKILL_FILE" ".agent/skills/_remote/" "asc-devkit remote skill directory"
expect_fixed "$PUBLIC_SKILL_FILE" "gitcode-pr" "gitcode-pr public skill"
expect_fixed "$PUBLIC_SKILL_FILE" "gitcode-issue" "gitcode-issue public skill"
expect_fixed "$PUBLIC_SKILL_FILE" "api-doc-generator" "api-doc-generator public skill"
expect_fixed "$PUBLIC_SKILL_FILE" "gitcode-pipeline" "gitcode-pipeline public skill"

expect_fixed "$INSTALL_SCRIPT" 'PUBLIC_SKILLS=("gitcode-pr" "gitcode-issue" "api-doc-generator" "gitcode-pipeline")' \
    "script public skill list"
expect_fixed "$INSTALL_SCRIPT" 'REPO_URL="https://gitcode.com/cann-agent/skills.git"' "script remote repository"
expect_fixed "$INSTALL_SCRIPT" 'git clone --depth 1 "$REPO_URL"' "script shallow clone"
expect_fixed "$INSTALL_SCRIPT" 'REMOTE_DIR="$SKILLS_DIR/_remote"' "script remote directory"
expect_fixed "$INSTALL_SCRIPT" 'ln -sfn "_remote/$skill" "$SKILLS_DIR/$skill"' "script symlink creation"
expect_fixed "$INSTALL_SCRIPT" 'rm -rf "$SKILLS_DIR/$skill"' "script replace local skill with symlink"
expect_fixed "$INSTALL_SCRIPT" 'ENTRY=".agent/skills/$skill"' "script gitignore symlink entry"

expect_fixed "$REMOTE_SKILL_FILE" "创建/提交 PR" "remote gitcode-pr create PR triggers"
expect_fixed "$REMOTE_SKILL_FILE" "获取PR改动" "remote gitcode-pr changes triggers"
expect_fixed "$REMOTE_SKILL_FILE" "查看 PR 讨论" "remote gitcode-pr discussion triggers"
expect_fixed "$REMOTE_SKILL_FILE" "删除 PR 评论" "remote gitcode-pr delete comment triggers"

expect_fixed "$AGENT_README" "\`public-skills\`" "README public-skills listing"
expect_fixed "$AGENT_README" "\`gitcode-pr\`" "README gitcode-pr listing"
expect_fixed "$AGENT_README" "_remote/gitcode-pr" "README remote skill implementation"
expect_fixed "$GITIGNORE" ".agent/skills/_remote/" "gitignore remote skill cache"
expect_fixed "$GITIGNORE" ".agent/skills/gitcode-pr" "gitignore gitcode-pr symlink"

echo "[PASS] gitcode-pr skill content validation passed"
