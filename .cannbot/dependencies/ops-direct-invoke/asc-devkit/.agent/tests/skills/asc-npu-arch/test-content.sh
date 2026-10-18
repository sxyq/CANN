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
SKILL_FILE="${REPO_ROOT}/.agent/skills/asc-npu-arch/SKILL.md"
REFERENCE_FILE="${REPO_ROOT}/.agent/skills/asc-npu-arch/references/npu-arch-guide.md"
FACTS_FILE="${REPO_ROOT}/.agent/skills/asc-npu-arch/references/npu-arch-facts.json"
API_UT_FILE="${REPO_ROOT}/.agent/skills/asc-api-ut-gen/SKILL.md"

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

for file in "$SKILL_FILE" "$REFERENCE_FILE"; do
    expect_fixed "$file" "统一维护原则" "unified maintenance principle"
    expect_fixed "$file" "统一芯片调用名" "unified chip names section"
    expect_fixed "$file" "统一数据类型视图" "unified dtype section"
    expect_fixed "$file" "ascend910" "ascend910 chip token"
    expect_fixed "$file" "ascend910b1" "ascend910b1 chip token"
    expect_fixed "$file" "ascend310p" "ascend310p chip token"
    expect_fixed "$file" "ascend610" "ascend610 chip token"
    expect_fixed "$file" "ascend310b1" "ascend310b1 chip token"
    expect_fixed "$file" "ascend950pr_9599" "ascend950pr_9599 chip token"
    expect_fixed "$file" "half" "half dtype"
    expect_fixed "$file" "float" "float dtype"
    expect_fixed "$file" "bfloat16_t" "bfloat16_t dtype"
    expect_fixed "$file" "int8_t" "int8_t dtype"
    expect_fixed "$file" "uint32_t" "uint32_t dtype"
    expect_fixed "$file" "docs/zh/api/context/内置数据类型.md" "builtin dtype source"
    expect_fixed "$file" "立即数赋值和初始化" "immediate assignment rule"
    expect_fixed "$file" "fp8_e8m0_t" "fp8_e8m0_t dtype"
    expect_fixed "$file" "complex64" "complex64 dtype"
done

expect_fixed "$SKILL_FILE" "其他 AscendC skill 只保留场景特有的限制或约束" "single source of truth rule"
expect_fixed "$REFERENCE_FILE" "其他 skill 不应再维护并行的完整芯片调用名表" "guide centralization rule"
expect_fixed "$SKILL_FILE" "dtype 大小" "dtype size source statement"
expect_fixed "$REFERENCE_FILE" "基础 dtype 大小与 32B 对齐" "dtype size alignment section"
expect_fixed "$REFERENCE_FILE" "dtype 大小表" "dtype size table centralization rule"
expect_fixed "$SKILL_FILE" "npu-arch-facts.json" "structured facts source"
expect_fixed "$REFERENCE_FILE" "npu-arch-facts.json" "guide structured facts source"

python3 - "$FACTS_FILE" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as f:
    data = json.load(f)

for chip in [
    "ascend910", "ascend910b1", "ascend310p", "ascend610",
    "ascend310b1", "ascend950pr_9599"
]:
    assert chip in data["chips"], chip
    assert data["chips"][chip]["npu_arch"], chip
    assert data["chips"][chip]["arch_dir"], chip

for dtype, size in {
    "bool": 1,
    "half": 2,
    "float": 4,
    "bfloat16_t": 2,
    "int8_t": 1,
    "uint32_t": 4,
    "double": 8,
    "complex32": 4,
    "complex64": 8,
    "fp4x2_e2m1_t": 1,
    "fp4x2_e1m2_t": 1,
    "hifloat8_t": 1,
    "fp8_e5m2_t": 1,
    "fp8_e4m3fn_t": 1,
    "fp8_e8m0_t": 1,
    "int4x2_t": 1,
}.items():
    assert data["dtypes"][dtype]["size"] == size, dtype

for dtype in [
    "bool", "int8_t", "uint8_t", "int16_t", "uint16_t",
    "half", "int32_t", "uint32_t", "float", "int64_t", "uint64_t"
]:
    assert data["dtypes"][dtype]["immediate_assignable"] is True, dtype
    assert data["dtypes"][dtype]["generic_ut_generation"] is True, dtype

for dtype in [
    "bfloat16_t", "double", "complex32", "complex64",
    "fp4x2_e2m1_t", "fp4x2_e1m2_t", "hifloat8_t",
    "fp8_e5m2_t", "fp8_e4m3fn_t", "fp8_e8m0_t", "int4x2_t"
]:
    assert data["dtypes"][dtype]["generic_ut_generation"] is False, dtype

assert data["dtype_source"] == "docs/zh/api/context/内置数据类型.md"
assert data["dtype_shorthands"]["S4"]["storage_type"] == "int4x2_t"
assert "fp8_e8m0_t" in data["dtype_product_support"]["ascend950"]["dtypes"]
assert "double" in data["dtype_product_support"]["atlas_a2_a3"]["dtypes"]
PY

expect_fixed "$API_UT_FILE" "../asc-npu-arch/SKILL.md" "asc-api-ut-gen back reference"

echo "[PASS] asc-npu-arch skill content validation passed"
