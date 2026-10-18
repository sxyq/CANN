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
SKILL_DIR="${SKILL_DIR:-${REPO_ROOT}/.agent/skills/asc-api-ut-gen}"
SKILL_FILE="${SKILL_DIR}/SKILL.md"
REFERENCES_DIR="${SKILL_DIR}/references"
SCRIPTS_DIR="${SKILL_DIR}/scripts"

required_references=(
    README.md
    api-guides/adv-api-ut-guide.md
    api-guides/c-api-ut-guide.md
    api-guides/membase-api-aic-ut-guide.md
    api-guides/membase-api-aiv-ut-guide.md
    api-guides/regbase-api-ut-guide.md
    api-guides/simt-api-ut-guide.md
    api-guides/utils-api-ut-guide.md
    foundations/branch-coverage-guide.md
    foundations/generation-constraints.json
    foundations/local-tensor-memory.md
    foundations/test-templates.md
    troubleshooting/faq.md
    workflows/automation-guide.md
    workflows/coverage-report-backfill-guide.md
    workflows/coverage-scan-guide.md
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
expect_dir "${REFERENCES_DIR}/api-guides"
expect_dir "${REFERENCES_DIR}/foundations"
expect_dir "${REFERENCES_DIR}/troubleshooting"
expect_dir "${REFERENCES_DIR}/workflows"
expect_dir "$SCRIPTS_DIR"

[[ "$(basename "$SKILL_DIR")" == "asc-api-ut-gen" ]] || fail "skill directory must be asc-api-ut-gen"
[[ ! -e "${SKILL_DIR}/README.md" ]] || fail "skill directory must not contain README.md"

[[ "$(head -n 1 "$SKILL_FILE")" == "---" ]] || fail "SKILL.md must start with YAML frontmatter"
frontmatter_close_count="$(grep -c '^---$' "$SKILL_FILE")"
[[ "$frontmatter_close_count" -ge 2 ]] || fail "SKILL.md frontmatter must be closed by ---"
grep -Eq '^name:[[:space:]]*asc-api-ut-gen[[:space:]]*$' "$SKILL_FILE" || fail "frontmatter name must be asc-api-ut-gen"
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
done

python3 -m json.tool "${REFERENCES_DIR}/foundations/generation-constraints.json" >/dev/null || \
    fail "invalid JSON: ${REFERENCES_DIR}/foundations/generation-constraints.json"

root_reference_count="$(find "$REFERENCES_DIR" -maxdepth 1 -type f -name '*.md' | wc -l)"
[[ "$root_reference_count" -eq 1 ]] || fail "references root must contain only README.md"

expect_file "${SCRIPTS_DIR}/ut_generator.py"
expect_file "${SCRIPTS_DIR}/ut_generator_cli.py"

links="$(grep -oE '\(references/[^)#]+' "$SKILL_FILE" | sed 's/^[(]//' || true)"
[[ -n "$links" ]] || fail "SKILL.md must link to references/"
while IFS= read -r link; do
    [[ -z "$link" ]] && continue
    expect_file "${SKILL_DIR}/${link}"
done <<< "$links"

for reference in "${required_references[@]}"; do
    grep -Fq "references/${reference}" "$SKILL_FILE" || fail "SKILL.md does not link ${reference}"
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

echo "[PASS] asc-api-ut-gen skill structure validation passed"
