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
SKILL_DIR="${SKILL_DIR:-${REPO_ROOT}/.agent/skills/asc-npu-arch}"
SKILL_FILE="${SKILL_DIR}/SKILL.md"
REFERENCE_FILE="${SKILL_DIR}/references/npu-arch-guide.md"
FACTS_FILE="${SKILL_DIR}/references/npu-arch-facts.json"

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

expect_dir "$SKILL_DIR"
expect_file "$SKILL_FILE"
expect_dir "${SKILL_DIR}/references"
expect_file "$REFERENCE_FILE"
expect_file "$FACTS_FILE"

[[ "$(basename "$SKILL_DIR")" == "asc-npu-arch" ]] || fail "skill directory must be asc-npu-arch"
[[ ! -e "${SKILL_DIR}/README.md" ]] || fail "skill directory must not contain README.md"

[[ "$(head -n 1 "$SKILL_FILE")" == "---" ]] || fail "SKILL.md must start with YAML frontmatter"
frontmatter_close_count="$(grep -c '^---$' "$SKILL_FILE")"
[[ "$frontmatter_close_count" -ge 2 ]] || fail "SKILL.md frontmatter must be closed by ---"
grep -Eq '^name:[[:space:]]*asc-npu-arch[[:space:]]*$' "$SKILL_FILE" || fail "frontmatter name must be asc-npu-arch"
grep -Eq '^description:[[:space:]]*.+' "$SKILL_FILE" || fail "frontmatter description is required"

grep -Fq "references/npu-arch-guide.md" "$SKILL_FILE" || fail "SKILL.md must link npu-arch-guide.md"
grep -Fq "references/npu-arch-facts.json" "$SKILL_FILE" || fail "SKILL.md must link npu-arch-facts.json"

python3 -m json.tool "$FACTS_FILE" >/dev/null || fail "invalid JSON: $FACTS_FILE"

echo "[PASS] asc-npu-arch skill structure validation passed"
