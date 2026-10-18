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
CLI="${SKILL_DIR}/scripts/ut_generator_cli.py"
FIXTURE_DIR="${TEST_DIR}/fixtures"
GOLDEN_DIR="${TEST_DIR}/golden"
ARCH_FACTS="${REPO_ROOT}/.agent/skills/asc-npu-arch/references/npu-arch-facts.json"
GENERATION_CONSTRAINTS="${SKILL_DIR}/references/foundations/generation-constraints.json"
WORK_DIR="$(mktemp -d)"
seen_api_types=""
seen_chips=""

cleanup() {
    rm -rf "$WORK_DIR"
}
trap cleanup EXIT

fail() {
    echo "[FAIL] $*" >&2
    exit 1
}

json_field() {
    local file="$1"
    local field="$2"

    grep -m1 "\"${field}\"" "$file" | sed -E "s/.*\"${field}\"[[:space:]]*:[[:space:]]*\"([^\"]+)\".*/\\1/"
}

mark_seen() {
    local current="$1"
    local value="$2"

    if [[ " ${current} " == *" ${value} "* ]]; then
        printf '%s' "$current"
    else
        printf '%s %s' "$current" "$value"
    fi
}

assert_covered() {
    local label="$1"
    local seen="$2"
    shift 2

    for expected in "$@"; do
        [[ " ${seen} " == *" ${expected} "* ]] || fail "missing ${label} fixture coverage: ${expected}"
    done
}

read_json_object_keys() {
    local -n target="$1"
    local file="$2"
    local key="$3"
    local label="$4"
    local output

    if ! output="$(python3 - "$file" "$key" "$label" <<'PY'
import json
import sys

path, key, label = sys.argv[1:4]
try:
    with open(path, encoding="utf-8") as f:
        data = json.load(f)
except Exception as exc:
    raise SystemExit(f"{label}: failed to read {path}: {exc}")

if key not in data:
    raise SystemExit(f"{label}: missing top-level object {key!r} in {path}")

value = data[key]
if not isinstance(value, dict):
    raise SystemExit(
        f"{label}: expected top-level {key!r} to be an object in {path}, "
        f"got {type(value).__name__}"
    )
if not value:
    raise SystemExit(f"{label}: top-level object {key!r} is empty in {path}")

print("\n".join(value.keys()))
PY
    )"; then
        fail "failed to read ${label} from ${file}:${key}"
    fi

    mapfile -t target <<< "$output"
}

join_words() {
    local IFS=" "
    printf '%s' "$*"
}

self_test_reference_key_reader() {
    local generation_ref="${WORK_DIR}/self-generation-constraints.json"
    local arch_ref="${WORK_DIR}/self-npu-arch-facts.json"
    local -a self_api_types
    local -a self_chips

    cat > "$generation_ref" <<'JSON'
{"api_types":{"aiv":{},"c_api":{}},"chips":"wrong-source"}
JSON
    cat > "$arch_ref" <<'JSON'
{"chips":{"ascend910":{},"ascend310b1":{}},"api_types":"wrong-source"}
JSON

    read_json_object_keys self_api_types "$generation_ref" "api_types" "self-test API types"
    read_json_object_keys self_chips "$arch_ref" "chips" "self-test chips"

    [[ "$(join_words "${self_api_types[@]}")" == "aiv c_api" ]] || \
        fail "self-test must read API types from generation-constraints api_types"
    [[ "$(join_words "${self_chips[@]}")" == "ascend910 ascend310b1" ]] || \
        fail "self-test must read chips from npu-arch-facts chips"
}

self_test_reference_key_reader
read_json_object_keys expected_api_types "$GENERATION_CONSTRAINTS" "api_types" "generation constraints API types"
read_json_object_keys expected_chips "$ARCH_FACTS" "chips" "npu arch chip facts"

self_test_generator_helpers() {
    python3 - "$SKILL_DIR" "$GENERATION_CONSTRAINTS" <<'PY'
import json
import sys

skill_dir, constraints_path = sys.argv[1:3]
sys.path.insert(0, f"{skill_dir}/scripts")

from ut_generator import (  # noqa: E402
    AIC_GENERIC_MMAD_LIKE_APIS,
    AIV_GENERIC_BINARY_APIS,
    AIV_GENERIC_SCALAR_TENSOR_DISPATCH_APIS,
    API_TYPE_CHOICES,
    ApiType,
    ChipArch,
    TestCase,
    TEMPLATES,
    UTConfig,
    UTGenerator,
    create_generator,
    get_adv_profile_output_path,
)
from ut_generator_cli import default_kernel_params  # noqa: E402

with open(constraints_path, encoding="utf-8") as f:
    expected_api_types = list(json.load(f)["api_types"].keys())

assert API_TYPE_CHOICES == expected_api_types
assert AIV_GENERIC_BINARY_APIS == frozenset({"add", "sub"})
assert AIV_GENERIC_SCALAR_TENSOR_DISPATCH_APIS == frozenset({"duplicate"})
assert AIC_GENERIC_MMAD_LIKE_APIS == frozenset({"mmad"})
assert default_kernel_params(ApiType.AIV.value) == {}
assert default_kernel_params(ApiType.AIC.value) == {"m": 16, "k": 64, "n": 16}

config = UTConfig(api_type=ApiType.AIV, api_name="Add", chip=ChipArch.ASCEND910B1)
generator = UTGenerator(config)
TEMPLATES["_self_template"] = "${A} ${B}"
try:
    rendered = generator.render("_self_template", {"A": "${B}", "B": "ok"})
finally:
    del TEMPLATES["_self_template"]

assert rendered == "${B} ok"

try:
    create_generator(
        UTConfig(
            api_type=ApiType.AIV,
            api_name="Duplicate",
            chip=ChipArch.ASCEND950PR_9599,
            test_cases=[TestCase(name="Duplicate_bad_binary_shape", input_count=2)],
        )
    ).generate()
except ValueError as exc:
    assert "scalar_tensor_dispatch AIV template requires input_count=1" in str(exc)
else:
    raise AssertionError("Duplicate without scalar/tensor test-case shape must fail validation")

try:
    create_generator(
        UTConfig(
            api_type=ApiType.AIV,
            api_name="Select",
            chip=ChipArch.ASCEND950PR_9599,
        )
    ).generate()
except ValueError as exc:
    assert "AIVUTGenerator cannot infer an executable AIV UT signature" in str(exc)
else:
    raise AssertionError("selector-family AIV API must fail closed without a verified template")

duplicate_code = create_generator(
    UTConfig(
        api_type=ApiType.AIV,
        api_name="Duplicate",
        chip=ChipArch.ASCEND950PR_9599,
        test_cases=[
            TestCase(name="Duplicate_float_256", data_size=256, dtype="float", input_count=1),
            TestCase(
                name="Duplicate_tensor_trait_float_256",
                data_size=256,
                dtype="float",
                input_count=1,
                additional_params={"tensor_trait": True},
            ),
        ],
    )
).generate()
assert "Duplicate(dstLocal, scalarValue, dataSize);" in duplicate_code
assert "Duplicate(dstLocal, srcLocal, dataSize);" in duplicate_code
assert "main_Duplicate<TensorTrait<float>>" in duplicate_code

try:
    create_generator(
        UTConfig(
            api_type=ApiType.AIC,
            api_name="LoadData",
            chip=ChipArch.ASCEND950PR_9599,
        )
    ).generate()
except ValueError as exc:
    assert "AIC automatic generation requires a verified template family" in str(exc)
else:
    raise AssertionError("non-MMAD-like AIC API must fail closed instead of reusing MMAD body")

mul_code = create_generator(
    UTConfig(
        api_type=ApiType.AIV,
        api_name="Mul",
        chip=ChipArch.ASCEND910B1,
        test_cases=[TestCase(name="Mul_half_128", data_size=128, dtype="half", input_count=2)],
        kernel_params={"aiv_template": "binary"},
    )
).generate()
assert "Mul(dstLocal, src0Local, src1Local, dataSize);" in mul_code

try:
    create_generator(
        UTConfig(
            api_type=ApiType.AIV,
            api_name="Add",
            chip=ChipArch.ASCEND910B1,
            test_cases=[TestCase(name="Add_half_scalar", data_size=128, dtype="half", input_count=1)],
        )
    ).generate()
except ValueError as exc:
    assert "binary AIV template requires input_count=2" in str(exc)
else:
    raise AssertionError("binary AIV template must reject non-binary test cases")

try:
    create_generator(
        UTConfig(
            api_type=ApiType.REG_COMPUTE,
            api_name="Scatter",
            chip=ChipArch.ASCEND950PR_9599,
        )
    ).generate()
except ValueError as exc:
    assert "regbase automatic generation is disabled" in str(exc)
else:
    raise AssertionError("regbase placeholder generation must fail closed")

try:
    create_generator(
        UTConfig(
            api_type=ApiType.SIMT,
            api_name="CastSat",
            chip=ChipArch.ASCEND950PR_9599,
        )
    ).generate()
except ValueError as exc:
    assert "SIMT automatic generation is disabled" in str(exc)
else:
    raise AssertionError("SIMT placeholder generation must fail closed")

try:
    create_generator(
        UTConfig(
            api_type=ApiType.C_API,
            api_name="asc_reduce_sum",
            chip=ChipArch.ASCEND950PR_9599,
        )
    ).generate()
except ValueError as exc:
    assert "C API automatic generation is disabled" in str(exc)
else:
    raise AssertionError("C API generic mock generation must fail closed")

try:
    create_generator(
        UTConfig(
            api_type=ApiType.UTILS,
            api_name="RemovePointer",
            chip=ChipArch.ASCEND950PR_9599,
        )
    ).generate()
except ValueError as exc:
    assert "utils automatic generation is disabled" in str(exc)
else:
    raise AssertionError("utils generic generation must fail closed without a verified template")

adv_profile = {
    "source": "tests/api/adv_api/quantization/quantize/test_quantize_per_tensor.cpp",
    "output": "tests/api/adv_api/quantization/quantize/test_quantize_per_tensor.cpp",
}
assert get_adv_profile_output_path("Quantize", {"adv_profile": adv_profile}).endswith(
    adv_profile["output"]
)

adv_config = UTConfig(
    api_type=ApiType.ADV,
    api_name="Quantize",
    chip=ChipArch.ASCEND910B1,
    kernel_params={"adv_profile": adv_profile},
)
adv_code = create_generator(adv_config).generate()
assert "QuantizeParams" in adv_code
assert "Quantize<config>" in adv_code
assert "main_Quantize<" not in adv_code

try:
    create_generator(UTConfig(api_type=ApiType.ADV, api_name="UnknownAdv", chip=ChipArch.ASCEND910B1)).generate()
except ValueError as exc:
    assert "cannot infer an executable high-level API UT signature" in str(exc)
else:
    raise AssertionError("unknown ADV API must fail closed instead of generating a non-executable skeleton")

try:
    create_generator(
        UTConfig(
            api_type=ApiType.ADV,
            api_name="Softmax",
            chip=ChipArch.ASCEND610,
            kernel_params={
                "adv_profile": {
                    "source": "tests/api/adv_api/activation/softmax/__missing_softmax_source__.cpp",
                    "output": "tests/api/adv_api/activation/softmax/test_operator_softmax_v220.cpp",
                }
            },
        )
    ).generate()
except FileNotFoundError as exc:
    assert "points to a missing reference UT" in str(exc)
else:
    raise AssertionError("ADV profile must fail when its exact source UT is missing")
PY
}

self_test_generator_helpers

if grep -Eq '"?(Softmax|Quantize|TopK|DropOut|dropout|quantize|softmax|topk)"?' "${SKILL_DIR}/scripts/ut_generator.py"; then
    fail "ut_generator.py must not hard-code API-specific ADV profiles"
fi

run_case() {
    local name="$1"
    local fixture="${FIXTURE_DIR}/${name}.json"
    local golden="${GOLDEN_DIR}/${name}.cpp"
    local actual="${WORK_DIR}/${name}.cpp"
    local api_type
    local api_name
    local chip
    local expected_error

    [[ -f "$fixture" ]] || fail "missing fixture: $fixture"

    api_type="$(json_field "$fixture" "api_type")"
    api_name="$(json_field "$fixture" "api_name")"
    chip="$(json_field "$fixture" "chip")"
    expected_error="$(python3 - "$fixture" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as f:
    print(json.load(f).get("expected_error", ""))
PY
    )"
    seen_api_types="$(mark_seen "$seen_api_types" "$api_type")"
    seen_chips="$(mark_seen "$seen_chips" "$chip")"

    if [[ -n "$expected_error" ]]; then
        if python3 "$CLI" --config "$fixture" --output "$actual" >"${actual}.log" 2>&1; then
            fail "expected generation failure for fixture: $name"
        fi
        grep -Fq "$expected_error" "${actual}.log" || \
            fail "generation failure for ${name} must mention: ${expected_error}"
        return
    fi

    [[ -f "$golden" ]] || fail "missing golden: $golden"
    python3 "$CLI" --config "$fixture" --output "$actual" >/dev/null
    if grep -Eq 'template <[^>]*,[[:space:]]*Src1T([,>])' "$actual"; then
        fail "generated UT contains naked Src1T template parameter: $name"
    fi
    if [[ "$api_type" == "aiv" ]] && grep -Eq 'uint8_t (dstGm|src0Gm|src1Gm)\[[^]]*param\.data_size' "$actual"; then
        fail "generated AIV UT must not allocate runtime-sized GM arrays on stack: $name"
    fi
    if [[ "$api_type" == "aiv" ]] && grep -Fq "src1Gm" "$actual" && \
        ! grep -Fq "param.init_func(src0Gm.data(), src1Gm.data(), param.data_size);" "$actual"; then
        fail "generated AIV UT must initialize inputs through dtype-aware init function: $name"
    fi
    if [[ "$api_type" == "adv" ]] && grep -Fq "main_${api_name}<" "$actual"; then
        fail "generated adv UT must not reference an undefined main_${api_name} template: $name"
    fi
    if [[ "$api_type" == "adv" ]] && grep -Fq "TODO: 添加实际验证逻辑" "$actual"; then
        fail "generated adv UT must not keep generic validation TODO: $name"
    fi
    diff -u "$golden" "$actual" || fail "generated UT differs from golden: $name"
}

assert_contains() {
    local file="$1"
    local needle="$2"
    local label="$3"

    grep -Fq "$needle" "$file" || fail "missing ${label}: ${needle}"
}

assert_not_contains() {
    local file="$1"
    local needle="$2"
    local label="$3"

    if grep -Fq "$needle" "$file"; then
        fail "unexpected ${label}: ${needle}"
    fi
}

assert_adv_profile_output() {
    local api="$1"
    local source="$2"
    local output="${3:-$2}"
    shift 3
    local actual="${WORK_DIR}/adv_${api}.cpp"
    local config="${WORK_DIR}/adv_${api}.json"

    python3 - "$config" "$api" "$source" "$output" <<'PY'
import json
import sys

config_path, api_name, source, output = sys.argv[1:5]
with open(config_path, "w", encoding="utf-8") as f:
    json.dump(
        {
            "api_type": "adv",
            "api_name": api_name,
            "chip": "ascend910b1",
            "test_cases": [],
            "kernel_params": {
                "adv_profile": {
                    "source": source,
                    "output": output,
                }
            },
            "custom_includes": [],
            "output_dir": None,
        },
        f,
    )
PY

    python3 "$CLI" --config "$config" --output "$actual" >/dev/null
    assert_not_contains "$actual" "TODO: 添加实际验证逻辑" "${api} generic validation TODO"
    assert_not_contains "$actual" "${api}<T1, T2>(dstLocal, srcLocal, {height, width})" "${api} old generic call"
    assert_not_contains "$actual" "main_${api}<" "${api} undefined old main wrapper"

    while (($# > 0)); do
        assert_contains "$actual" "$1" "${api} executable profile marker"
        shift
    done
}

supported_output="${WORK_DIR}/list-supported.txt"
python3 "$CLI" --list-supported > "$supported_output"
grep -Fq "npu-arch-facts.json" "$supported_output" || fail "CLI must report npu-arch structured reference"
grep -Fq "generation-constraints.json" "$supported_output" || fail "CLI must report generation constraints reference"
grep -Fq "fp8_e8m0_t" "$supported_output" || fail "CLI must list documented builtin dtypes from npu-arch facts"
grep -Fq "api-specific-init" "$supported_output" || fail "CLI must distinguish dtype initialization constraints"
grep -Fq "通用 UT 生成可直接初始化的数据类型" "$supported_output" || fail "CLI must list generic generator dtype subset"

invalid_dtype_output="${WORK_DIR}/invalid-dtype.txt"
if python3 "$CLI" --type aiv --api Add --chip ascend910b1 --dtype fp8_e5m2_t --output - >"$invalid_dtype_output" 2>&1; then
    fail "generic generation must reject dtypes that require API-specific initialization"
fi
grep -Fq "invalid choice" "$invalid_dtype_output" || \
    fail "unsupported dtype rejection must come from structured generator dtype constraints"

invalid_output="${WORK_DIR}/invalid-simt.txt"
if python3 "$CLI" --type simt --api vector_add --chip ascend910b1 --output - >"$invalid_output" 2>&1; then
    fail "SIMT generation must reject chips disallowed by generation-constraints.json"
fi
grep -Fq "SIMT API only supports ascend950pr_9599" "$invalid_output" || \
    fail "SIMT rejection must use generation-constraints.json message"

invalid_adv_output="${WORK_DIR}/invalid-adv.txt"
if python3 "$CLI" --type adv --api UnknownAdv --chip ascend910b1 --output - >"$invalid_adv_output" 2>&1; then
    fail "unknown ADV generation must fail closed instead of generating a non-executable skeleton"
fi
grep -Fq "cannot infer an executable high-level API UT signature" "$invalid_adv_output" || \
    fail "unknown ADV rejection must explain executable profile requirement"

assert_adv_profile_output \
    Softmax \
    "tests/api/adv_api/activation/softmax/test_operator_softmax_v220.cpp" \
    "tests/api/adv_api/activation/softmax/test_operator_softmax_v220.cpp" \
    "MainSoftmax" "SoftMaxShapeInfo"
assert_adv_profile_output \
    Quantize \
    "tests/api/adv_api/quantization/quantize/test_quantize_per_tensor.cpp" \
    "tests/api/adv_api/quantization/quantize/test_quantize_per_tensor.cpp" \
    "QuantizeParams" "QuantizePolicy::PER_TENSOR" "sharedTmpBuffer"
assert_adv_profile_output \
    TopK \
    "tests/api/adv_api/sort/topk/test_operator_topk.cpp" \
    "tests/api/adv_api/sort/topk/test_operator_topk.cpp" \
    "TopKInfo" "TopkTiling" "outputGmIndex"
assert_adv_profile_output \
    DropOut \
    "tests/api/adv_api/filter/dropout/test_operator_dropout.cpp" \
    "tests/api/adv_api/filter/dropout/test_operator_dropout.cpp" \
    "DropOutShapeInfo" "maskLastAxis" "EXPECT_EQ"

coverage_report="${WORK_DIR}/coverage.html"
cat > "$coverage_report" <<'EOF'
<html><body>
<div>Lines: 1.0%</div>
<table>
<tr><td class="headerItem">Lines:</td><td class="headerCovTableEntryHi">93.6 %</td></tr>
<tr><td class="headerItem">Functions:</td><td class="headerCovTableEntryHi">91.2 %</td></tr>
</table>
</body></html>
EOF

report_json="${WORK_DIR}/generation-report.json"
actual_report_case="${WORK_DIR}/report_case.cpp"
ASC_API_UT_PROMPT_TOKENS=11 \
ASC_API_UT_COMPLETION_TOKENS=22 \
python3 "$CLI" \
    --config "${FIXTURE_DIR}/add_aiv_ascend910b1.json" \
    --output "$actual_report_case" \
    --coverage-report "$coverage_report" \
    --report-json "$report_json" >/dev/null

python3 - "$report_json" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as f:
    report = json.load(f)

assert report["token_usage"]["prompt_tokens"] == 11
assert report["token_usage"]["completion_tokens"] == 22
assert report["token_usage"]["total_tokens"] == 33
assert report["elapsed_seconds"] >= 0
assert report["current_coverage"]["status"] == "available"
assert report["current_coverage"]["lines"] == 93.6
assert report["current_coverage"]["functions"] == 91.2
PY

invalid_token_report_json="${WORK_DIR}/invalid-token-report.json"
invalid_token_case="${WORK_DIR}/invalid_token_case.cpp"
ASC_API_UT_PROMPT_TOKENS=abc \
ASC_API_UT_COMPLETION_TOKENS=" 7 " \
ASC_API_UT_TOTAL_TOKENS=bad \
python3 "$CLI" \
    --config "${FIXTURE_DIR}/add_aiv_ascend910b1.json" \
    --output "$invalid_token_case" \
    --report-json "$invalid_token_report_json" >/dev/null

python3 - "$invalid_token_report_json" <<'PY'
import json
import sys

with open(sys.argv[1], encoding="utf-8") as f:
    report = json.load(f)

token_usage = report["token_usage"]
assert token_usage["source"] == "env"
assert token_usage["prompt_tokens"] is None
assert token_usage["completion_tokens"] == 7
assert token_usage["total_tokens"] is None
assert "ASC_API_UT_PROMPT_TOKENS" in token_usage["reason"]
assert "ASC_API_UT_TOTAL_TOKENS" in token_usage["reason"]
PY

fixture_count=0
for fixture in "${FIXTURE_DIR}"/*.json; do
    [[ -f "$fixture" ]] || fail "no fixture found under ${FIXTURE_DIR}"
    run_case "$(basename "$fixture" .json)"
    fixture_count=$((fixture_count + 1))
done

assert_covered "API type" "$seen_api_types" "${expected_api_types[@]}"
assert_covered "chip" "$seen_chips" "${expected_chips[@]}"

echo "[PASS] asc-api-ut-gen generation golden validation passed (${fixture_count} fixtures)"
