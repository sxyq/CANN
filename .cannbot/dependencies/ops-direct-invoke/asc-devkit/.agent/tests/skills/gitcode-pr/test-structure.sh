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
SKILL_LINK="${SKILLS_DIR}/gitcode-pr"
REMOTE_SKILL="${SKILLS_DIR}/_remote/gitcode-pr"
PUBLIC_SKILL_DIR="${SKILLS_DIR}/public-skills"
PUBLIC_SKILL_FILE="${PUBLIC_SKILL_DIR}/SKILL.md"
INSTALL_SCRIPT="${PUBLIC_SKILL_DIR}/scripts/install-public-skills.sh"

fail() {
    echo "[FAIL] $*" >&2
    exit 1
}

expect_file() {
    local path="$1"
    [[ -f "$path" ]] || fail "missing file: $path"
    [[ -s "$path" ]] || fail "empty file: $path"
}

expect_dir() {
    local path="$1"
    [[ -d "$path" ]] || fail "missing directory: $path"
}

expect_symlink() {
    local path="$1"
    [[ -L "$path" ]] || fail "missing symlink: $path"
}

expect_dir "$PUBLIC_SKILL_DIR"
expect_file "$PUBLIC_SKILL_FILE"
expect_file "$INSTALL_SCRIPT"
[[ -x "$INSTALL_SCRIPT" ]] || fail "install script must be executable: $INSTALL_SCRIPT"

if [[ ! -d "$REMOTE_SKILL" || ! -L "$SKILL_LINK" ]]; then
    bash "$INSTALL_SCRIPT"
fi

expect_dir "${SKILLS_DIR}/_remote"
expect_dir "$REMOTE_SKILL"
expect_symlink "$SKILL_LINK"
[[ "$(readlink "$SKILL_LINK")" == "_remote/gitcode-pr" ]] || fail "gitcode-pr must link to _remote/gitcode-pr"

expect_file "${REMOTE_SKILL}/SKILL.md"
[[ "$(head -n 1 "${REMOTE_SKILL}/SKILL.md")" == "---" ]] || fail "remote SKILL.md must start with YAML frontmatter"
grep -Eq '^name:[[:space:]]*gitcode-pr[[:space:]]*$' "${REMOTE_SKILL}/SKILL.md" || \
    fail "remote frontmatter name must be gitcode-pr"
grep -Eq '^description:[[:space:]]*.+' "${REMOTE_SKILL}/SKILL.md" || fail "remote frontmatter description is required"

[[ ! -e "${SKILL_LINK}/scripts/create_pr.py" ]] || \
    fail "gitcode-pr must not use the previous asc-devkit-local create_pr.py implementation"
[[ ! -e "${SKILL_LINK}/scripts/pr_api.py" ]] || \
    fail "gitcode-pr must not use the previous asc-devkit-local pr_api.py implementation"

bash -n "$INSTALL_SCRIPT" || fail "install script syntax check failed"

echo "[PASS] gitcode-pr skill structure validation passed"
