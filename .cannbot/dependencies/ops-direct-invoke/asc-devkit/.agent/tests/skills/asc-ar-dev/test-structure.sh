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
SKILL_DIR="${SKILL_DIR:-${REPO_ROOT}/.agent/skills/asc-ar-dev}"
SKILL_FILE="${SKILL_DIR}/SKILL.md"
REFERENCES_DIR="${SKILL_DIR}/references"

required_references=(
    README.md
    00_format_template.md
    01_call_rules.md
    02_local_environment.md
    03_devkit_snippets.md
    04_debug_dump.md
    05_build_meta.md
    06_task_goal_example.md
    07_requirement_type_routing.md
    08_api_lookup.md
)

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
expect_dir "$REFERENCES_DIR"

[[ "$(basename "$SKILL_DIR")" == "asc-ar-dev" ]] || fail "skill directory must be asc-ar-dev"
[[ ! -e "${SKILL_DIR}/README.md" ]] || fail "skill directory must not contain README.md"
[[ ! -e "${REFERENCES_DIR}/06_task_goal_debug_bus_example.md" ]] || fail "old debug-bus task goal example must not be present"

[[ "$(head -n 1 "$SKILL_FILE")" == "---" ]] || fail "SKILL.md must start with YAML frontmatter"
frontmatter_close_count="$(grep -c '^---$' "$SKILL_FILE")"
[[ "$frontmatter_close_count" -ge 2 ]] || fail "SKILL.md frontmatter must be closed by ---"
grep -Eq '^name:[[:space:]]*asc-ar-dev[[:space:]]*$' "$SKILL_FILE" || fail "frontmatter name must be asc-ar-dev"
grep -Eq '^description:[[:space:]]*.+' "$SKILL_FILE" || fail "frontmatter description is required"

frontmatter_text="$(sed -n '2,/^---$/p' "$SKILL_FILE" | sed '$d')"
if grep -Eq '<[^>]+>' <<< "$frontmatter_text"; then
    fail "frontmatter must not contain XML-like angle bracket tags"
fi

awk 'BEGIN { body = 0; found = 0 }
     NR > 1 && $0 == "---" { body = 1; next }
     body && NF { found = 1 }
     END { exit found ? 0 : 1 }' "$SKILL_FILE" || fail "SKILL.md body must not be empty"

for reference in "${required_references[@]}"; do
    expect_file "${REFERENCES_DIR}/${reference}"
    grep -Fq "references/${reference}" "$SKILL_FILE" || fail "SKILL.md does not link references/${reference}"
    grep -Fq "$reference" "${REFERENCES_DIR}/README.md" || fail "references/README.md does not index ${reference}"
done

while IFS= read -r markdown_file; do
    while IFS= read -r markdown_link; do
        [[ -z "$markdown_link" ]] && continue
        target="${markdown_link#*]}"
        target="${target#(}"
        target="${target%)}"
        target="${target%%#*}"
        [[ "$target" =~ ^https?:// ]] && continue
        [[ "$target" == *.md ]] || continue
        if [[ "$target" = /* ]]; then
            target_path="$target"
        else
            target_path="$(dirname "$markdown_file")/$target"
        fi
        [[ -f "$target_path" ]] || fail "broken markdown link in ${markdown_file}: ${target}"
    done < <(grep -oE '\[[^]]+\]\([^)]+\.md(#[^)]+)?\)' "$markdown_file" || true)
done < <(find "$SKILL_FILE" "$REFERENCES_DIR" -type f -name '*.md' -print)

if grep -RFn "references/09_aicore_printf_ringbuf.md" "$SKILL_DIR"; then
    fail "skill must not link missing references/09_aicore_printf_ringbuf.md"
fi

if find "$SKILL_DIR" -type f \( -name '*.md' -o -name '*.json' -o -name '*.py' -o -name '*.sh' \) -print | grep -q '__pycache__'; then
    fail "skill directory must not contain generated cache files"
fi

echo "[PASS] asc-ar-dev skill structure validation passed"
