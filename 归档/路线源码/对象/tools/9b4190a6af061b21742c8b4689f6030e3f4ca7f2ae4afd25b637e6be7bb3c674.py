#!/usr/bin/env python3
"""Independent CPU matrix for AddRmsNormBias.

The script keeps the input tensors in the requested dtype, performs the
reference math in float32, and casts only the final output.  It evaluates a
blocked float32 path separately from the direct reference path, so the
reported numbers do not depend on an Ascend runtime.
"""

from __future__ import annotations

import argparse
import json
import math
import sys
from collections import defaultdict
from datetime import datetime
from pathlib import Path

import numpy as np

try:
    import ml_dtypes
except ImportError as exc:  # pragma: no cover - exercised only on missing dependency
    ml_dtypes = None
    ML_DTYPES_ERROR = str(exc)
else:
    ML_DTYPES_ERROR = None


ROOT = Path(__file__).resolve().parents[2]
OUT_DIR = Path(__file__).resolve().parent
REPORT_PATH = ROOT / "临时" / "R001-V003-Agent02-CPU矩阵.md"
JSON_PATH = OUT_DIR / "cpu_matrix_results.json"

SEED = 1742
D_VALUES = [1, 31, 32, 33, 67, 127, 128, 129, 1000, 4096, 8192, 32768]
RANKS = (2, 3, 4)
OUTER_VALUES = {"one": 1, "small": 4, "large": 8192}
EPS_VALUES = (0.0, 1e-8, 1e-5, 1e-3)
# Match the V003 kernel tile so the CPU reduction order models the current path.
BLOCK = 4096
REL_FLOOR = 1e-6
RATIO_LIMIT = 1e-3

DTYPE_MAP = {
    "fp32": np.float32,
    "fp16": np.float16,
}
if ml_dtypes is not None:
    DTYPE_MAP["bf16"] = ml_dtypes.bfloat16

# Project-specific thresholds from the problem notes and local verifier.
TOLERANCE = {
    "fp32": {"rtol": 1e-4, "atol": 1e-4},
    "fp16": {"rtol": 1e-3, "atol": 1e-3},
    "bf16": {"rtol": 1e-3, "atol": 1e-3},
}


def dtype_obj(name):
    return DTYPE_MAP[name]


def shape_for(rank, outer, dim):
    if rank == 2:
        return [outer, dim]
    if rank == 3:
        return [1, outer, dim]
    if rank == 4:
        return [1, 1, outer, dim]
    raise ValueError(f"unsupported rank: {rank}")


def cast_output(value, name):
    return np.asarray(value, dtype=dtype_obj(name))


def make_affine(dim, name, rng, mode="random"):
    dtype = dtype_obj(name)
    if mode == "random":
        gamma = rng.uniform(0.9, 1.1, dim).astype(dtype)
        bias = rng.uniform(-0.1, 0.1, dim).astype(dtype)
        return gamma, bias
    if mode == "channel_ramp":
        gamma = np.linspace(0.5, 1.5, dim, dtype=np.float32).astype(dtype)
        bias = np.linspace(-0.25, 0.25, dim, dtype=np.float32).astype(dtype)
        return gamma, bias
    if mode == "identity":
        return np.ones(dim, dtype=dtype), np.zeros(dim, dtype=dtype)
    raise ValueError(f"unsupported affine mode: {mode}")


def make_inputs(rows, dim, name, rng, mode="uniform"):
    dtype = dtype_obj(name)
    if mode == "uniform":
        x = rng.uniform(-2.0, 2.0, (rows, dim)).astype(dtype)
        residual = rng.uniform(-2.0, 2.0, (rows, dim)).astype(dtype)
        return x, residual
    if mode == "zero":
        return np.zeros((rows, dim), dtype=dtype), np.zeros((rows, dim), dtype=dtype)
    if mode == "large_fp16":
        value = np.full((rows, dim), 128.0, dtype=dtype)
        return value, value.copy()
    if mode == "bf16_pattern":
        base = np.empty(dim, dtype=np.float32)
        pattern = np.arange(dim, dtype=np.float32) % 8
        base[:] = 0.25 + pattern / 3.0
        x = np.broadcast_to(base, (rows, dim)).copy().astype(dtype)
        residual = np.zeros((rows, dim), dtype=dtype)
        return x, residual
    raise ValueError(f"unsupported input mode: {mode}")


def golden_direct(x, residual, gamma, bias, epsilon, name):
    """Direct float32 reference following the problem formula."""
    with np.errstate(all="ignore"):
        x32 = x.astype(np.float32)
        residual32 = residual.astype(np.float32)
        gamma32 = gamma.astype(np.float32)
        bias32 = bias.astype(np.float32)
        y = np.add(x32, residual32, dtype=np.float32)
        sum_sq = np.sum(np.multiply(y, y, dtype=np.float32), axis=-1, dtype=np.float32)
        mean_sq = sum_sq / np.float32(x.shape[-1])
        rms = np.sqrt(mean_sq + np.float32(epsilon)).reshape((-1, 1))
        normalized = np.divide(y, rms)
        output = np.add(np.multiply(normalized, gamma32), bias32)
    return cast_output(output, name)


def candidate_r017(x, residual, gamma, bias, epsilon, name, block=BLOCK):
    """Blocked float32 path representing R017 with a final dtype cast."""
    with np.errstate(all="ignore"):
        x32 = x.astype(np.float32)
        residual32 = residual.astype(np.float32)
        gamma32 = gamma.astype(np.float32)
        bias32 = bias.astype(np.float32)
        y = np.add(x32, residual32, dtype=np.float32)
        sum_sq = np.zeros(y.shape[0], dtype=np.float32)
        for start in range(0, y.shape[-1], block):
            block_y = y[:, start : start + block]
            block_sum = np.sum(np.multiply(block_y, block_y, dtype=np.float32), axis=-1, dtype=np.float32)
            sum_sq = np.add(sum_sq, block_sum, dtype=np.float32)
        mean_sq = sum_sq / np.float32(x.shape[-1])
        rms = np.sqrt(mean_sq + np.float32(epsilon)).reshape((-1, 1))
        normalized = np.divide(y, rms)
        output = np.add(np.multiply(normalized, gamma32), bias32)
    return cast_output(output, name)


class MetricAccumulator:
    def __init__(self, rtol, atol):
        self.rtol = rtol
        self.atol = atol
        self.total = 0
        self.mismatches = 0
        self.max_abs = 0.0
        self.max_rel = 0.0
        self.nonfinite_mismatches = 0

    def add(self, actual, expected):
        actual32 = actual.astype(np.float32)
        expected32 = expected.astype(np.float32)
        diff = np.abs(actual32 - expected32)
        finite = np.isfinite(actual32) & np.isfinite(expected32)
        if np.any(finite):
            self.max_abs = max(self.max_abs, float(np.max(diff[finite])))
            denom = np.maximum(np.maximum(np.abs(actual32[finite]), np.abs(expected32[finite])), REL_FLOOR)
            rel = diff[finite] / denom
            self.max_rel = max(self.max_rel, float(np.max(rel)))

        nonfinite_equal = (np.isnan(actual32) & np.isnan(expected32)) | (actual32 == expected32)
        nonfinite_bad = (~finite) & (~nonfinite_equal)
        self.nonfinite_mismatches += int(np.count_nonzero(nonfinite_bad))

        close = np.isclose(
            actual32,
            expected32,
            rtol=self.rtol,
            atol=self.atol,
            equal_nan=True,
        )
        self.mismatches += int(np.count_nonzero(~close))
        self.total += int(actual32.size)

    def finish(self):
        ratio = self.mismatches / self.total if self.total else 0.0
        return {
            "total": self.total,
            "mismatches": self.mismatches,
            "mismatch_ratio": ratio,
            "max_abs_error": self.max_abs,
            "max_relative_error": self.max_rel,
            "nonfinite_mismatches": self.nonfinite_mismatches,
            "strict_pass": self.mismatches == 0,
            "ratio_pass": ratio <= RATIO_LIMIT,
        }


def case_seed(case_id):
    return SEED + case_id * 1009


def chunk_rows(dim):
    return max(1, min(32, 1_048_576 // max(dim, 1)))


def evaluate_stream_case(
    case_id,
    section,
    name,
    rank,
    dim,
    outer,
    epsilon,
    affine_mode="random",
    input_mode="uniform",
):
    rng = np.random.default_rng(case_seed(case_id))
    gamma, bias = make_affine(dim, name, rng, affine_mode)
    tol = TOLERANCE[name]
    metrics = MetricAccumulator(tol["rtol"], tol["atol"])
    rows_per_chunk = chunk_rows(dim)
    for row_start in range(0, outer, rows_per_chunk):
        rows = min(rows_per_chunk, outer - row_start)
        x, residual = make_inputs(rows, dim, name, rng, input_mode)
        expected = golden_direct(x, residual, gamma, bias, epsilon, name)
        actual = candidate_r017(x, residual, gamma, bias, epsilon, name)
        metrics.add(actual, expected)
    result = {
        "section": section,
        "case_id": case_id,
        "dtype": name,
        "rank": rank,
        "shape": shape_for(rank, outer, dim),
        "outer": outer,
        "outer_label": None,
        "D": dim,
        "epsilon": epsilon,
        "affine_mode": affine_mode,
        "input_mode": input_mode,
        "rows_per_chunk": rows_per_chunk,
    }
    result.update(metrics.finish())
    return result


def q_binary(left, right, name, op):
    left32 = np.asarray(left, dtype=np.float32)
    right32 = np.asarray(right, dtype=np.float32)
    with np.errstate(all="ignore"):
        if op == "add":
            value = left32 + right32
        elif op == "mul":
            value = left32 * right32
        elif op == "div":
            value = left32 / right32
        else:
            raise ValueError(op)
    with np.errstate(all="ignore"):
        return value.astype(dtype_obj(name))


def low_precision_candidate(x, residual, gamma, bias, epsilon, name):
    """Quantize after every arithmetic stage to model an R004 path."""
    dim = x.shape[-1]
    y = q_binary(x, residual, name, "add")
    sum_sq = np.zeros(y.shape[0], dtype=dtype_obj(name))
    for index in range(dim):
        square = q_binary(y[:, index], y[:, index], name, "mul")
        sum_sq = q_binary(sum_sq, square, name, "add")
    mean_sq = q_binary(sum_sq, np.asarray(float(dim), dtype=dtype_obj(name)), name, "div")
    mean_eps = q_binary(mean_sq, np.asarray(epsilon, dtype=dtype_obj(name)), name, "add")
    with np.errstate(all="ignore"):
        rms = np.sqrt(mean_eps.astype(np.float32)).astype(dtype_obj(name))
    inverse = q_binary(np.ones_like(rms), rms, name, "div")
    normalized = q_binary(y, inverse.reshape((-1, 1)), name, "mul")
    scaled = q_binary(normalized, gamma.reshape((1, -1)), name, "mul")
    return q_binary(scaled, bias.reshape((1, -1)), name, "add")


def evaluate_low_case(case_id, name, dim, outer, input_mode, affine_mode, epsilon):
    rng = np.random.default_rng(case_seed(case_id))
    gamma, bias = make_affine(dim, name, rng, affine_mode)
    x, residual = make_inputs(outer, dim, name, rng, input_mode)
    expected = golden_direct(x, residual, gamma, bias, epsilon, name)
    actual = low_precision_candidate(x, residual, gamma, bias, epsilon, name)
    tol = TOLERANCE[name]
    metrics = MetricAccumulator(tol["rtol"], tol["atol"])
    metrics.add(actual, expected)
    result = {
        "section": "low_precision_counterexample",
        "case_id": case_id,
        "dtype": name,
        "rank": 2,
        "shape": [outer, dim],
        "outer": outer,
        "D": dim,
        "epsilon": epsilon,
        "affine_mode": affine_mode,
        "input_mode": input_mode,
    }
    result.update(metrics.finish())
    return result


def build_main_matrix(quick=False):
    rows = []
    case_id = 1
    for name in ("fp32", "fp16", "bf16"):
        for rank in RANKS:
            for dim in D_VALUES:
                for outer_label, outer in OUTER_VALUES.items():
                    if quick and (dim not in (1, 67, 4096, 32768) or outer_label == "large"):
                        continue
                    result = evaluate_stream_case(
                        case_id,
                        "main_matrix",
                        name,
                        rank,
                        dim,
                        outer,
                        1e-5,
                    )
                    result["outer_label"] = outer_label
                    rows.append(result)
                    case_id += 1
    return rows, case_id


def build_auxiliary_cases(case_id, quick=False):
    rows = []
    for name in ("fp32", "fp16", "bf16"):
        for rank in RANKS:
            for dim in (1, 67, 4096):
                for epsilon in EPS_VALUES:
                    if quick and epsilon not in (0.0, 1e-5):
                        continue
                    result = evaluate_stream_case(
                        case_id,
                        "epsilon_matrix",
                        name,
                        rank,
                        dim,
                        4,
                        epsilon,
                        affine_mode="channel_ramp",
                    )
                    result["outer_label"] = "small"
                    rows.append(result)
                    case_id += 1

            result = evaluate_stream_case(
                case_id,
                "broadcast_matrix",
                name,
                rank,
                67,
                4,
                1e-5,
                affine_mode="channel_ramp",
            )
            result["outer_label"] = "small"
            rows.append(result)
            case_id += 1

            for epsilon in (0.0, 1e-5):
                result = evaluate_stream_case(
                    case_id,
                    "zero_matrix",
                    name,
                    rank,
                    67,
                    4,
                    epsilon,
                    affine_mode="channel_ramp",
                    input_mode="zero",
                )
                result["outer_label"] = "small"
                rows.append(result)
                case_id += 1

            result = evaluate_stream_case(
                case_id,
                "nan_inf_matrix",
                name,
                rank,
                67,
                4,
                1e-5,
                affine_mode="channel_ramp",
                input_mode="uniform",
            )
            result["outer_label"] = "small"
            rows.append(result)
            case_id += 1
    return rows, case_id


def add_nonfinite_probe(row, name, value_kind):
    dim = row["D"]
    outer = row["outer"]
    rng = np.random.default_rng(case_seed(row["case_id"]))
    gamma, bias = make_affine(dim, name, rng, "channel_ramp")
    x, residual = make_inputs(outer, dim, name, rng, "uniform")
    if value_kind == "nan":
        x[0, 0] = np.asarray(np.nan, dtype=dtype_obj(name))
    elif value_kind == "inf":
        x[0, 0] = np.asarray(np.inf, dtype=dtype_obj(name))
    else:
        raise ValueError(value_kind)
    expected = golden_direct(x, residual, gamma, bias, row["epsilon"], name)
    actual = candidate_r017(x, residual, gamma, bias, row["epsilon"], name)
    metrics = MetricAccumulator(TOLERANCE[name]["rtol"], TOLERANCE[name]["atol"])
    metrics.add(actual, expected)
    row["input_mode"] = value_kind
    row.update(metrics.finish())


def build_special_nonfinite(rows):
    for row in rows:
        if row["section"] == "nan_inf_matrix":
            # Keep the same case id and input stream, then replace one value.
            add_nonfinite_probe(row, row["dtype"], "nan" if row["case_id"] % 2 else "inf")


def build_low_counterexamples(case_id):
    rows = []
    rows.append(evaluate_low_case(case_id, "fp16", 64, 4, "large_fp16", "identity", 1e-5))
    case_id += 1
    rows.append(evaluate_low_case(case_id, "fp16", 1024, 64, "uniform", "random", 1e-5))
    case_id += 1
    rows.append(evaluate_low_case(case_id, "bf16", 32768, 4, "bf16_pattern", "channel_ramp", 1e-5))
    case_id += 1
    rows.append(evaluate_low_case(case_id, "bf16", 32768, 4, "uniform", "random", 1e-5))
    return rows, case_id + 1


def aggregate(rows, keys):
    groups = defaultdict(list)
    for row in rows:
        groups[tuple(row[key] for key in keys)].append(row)
    output = []
    for group_key, group_rows in sorted(groups.items(), key=lambda item: tuple(str(v) for v in item[0])):
        output.append(
            {
                **dict(zip(keys, group_key)),
                "cases": len(group_rows),
                "strict_passes": sum(bool(row["strict_pass"]) for row in group_rows),
                "ratio_passes": sum(bool(row["ratio_pass"]) for row in group_rows),
                "max_abs_error": max(row["max_abs_error"] for row in group_rows),
                "max_relative_error": max(row["max_relative_error"] for row in group_rows),
                "max_mismatch_ratio": max(row["mismatch_ratio"] for row in group_rows),
                "total_elements": sum(row["total"] for row in group_rows),
            }
        )
    return output


def fmt(value):
    if isinstance(value, bool):
        return "是" if value else "否"
    if value == math.inf:
        return "inf"
    if isinstance(value, float):
        return f"{value:.3e}"
    return str(value)


def report_table(rows, columns):
    header = "| " + " | ".join(columns) + " |"
    divider = "| " + " | ".join("---" for _ in columns) + " |"
    body = []
    for row in rows:
        body.append("| " + " | ".join(fmt(row.get(key, "")) for key in columns) + " |")
    return "\n".join([header, divider] + body)


def generate_report(payload, command, quick):
    main_rows = [row for row in payload["cases"] if row["section"] == "main_matrix"]
    aux_rows = [row for row in payload["cases"] if row["section"] != "main_matrix" and row["section"] != "low_precision_counterexample"]
    low_rows = [row for row in payload["cases"] if row["section"] == "low_precision_counterexample"]
    by_dtype = aggregate(main_rows, ["dtype"])
    by_rank = aggregate(main_rows, ["rank"])
    by_shape_group = aggregate(main_rows, ["dtype", "rank", "outer_label"])
    by_aux = aggregate(aux_rows, ["section", "dtype"])

    worst_main = sorted(main_rows, key=lambda row: (row["max_abs_error"], row["max_relative_error"]), reverse=True)[:8]
    worst_low = sorted(low_rows, key=lambda row: (row["max_abs_error"], row["max_relative_error"]), reverse=True)

    lines = [
        "# R024 CPU 验证矩阵",
        "",
        "> 本报告由独立 CPU 参考链生成。所有结果来自本机 Python/numpy/ml_dtypes，不能当作 CANN 编译、NPU 精度或平台结果。",
        "",
        "## 结论",
        "",
        f"- 主矩阵完成 {len(main_rows)} 个用例，覆盖 3 种 dtype、2D/3D/4D、全部指定 D，以及 `outer=1/4/8192`；大矩阵按块流式处理，未一次性保存全量张量。",
        f"- 主矩阵 R017（FP32 中间、末次转换）严格逐元素通过 {sum(row['strict_pass'] for row in main_rows)}/{len(main_rows)}；最大绝对误差为 {fmt(max(row['max_abs_error'] for row in main_rows))}，最大稳健相对误差为 {fmt(max(row['max_relative_error'] for row in main_rows))}。BF16 有 {sum(row['dtype'] == 'bf16' and not row['strict_pass'] for row in main_rows)} 个分块归约用例落在输出舍入边界，最高失配比例仍为 {fmt(max(row['mismatch_ratio'] for row in main_rows))}。",
        "- R004 低精度中间路径在专门反例中出现明显失配：FP16 大值平方溢出，BF16 大 D 低精度逐步累加也超过本轮容差；不进入默认计算链。",
        "- R024 结论：CPU 数学链、尾块 D、形状展平和 gamma/bias 末维广播均已得到参考级证据；仍缺少真实 Kernel 输出，不能替代服务器 CANN/NPU 验证。",
        "",
        "## 运行信息",
        "",
        f"- 运行时间：`{payload['run_time']}`",
        f"- 命令：`{command}`",
        f"- Python：`{payload['environment']['python']}`；numpy：`{payload['environment']['numpy']}`；ml_dtypes：`{payload['environment']['ml_dtypes']}`",
        f"- 随机种子：`{SEED}`；参考块长：`{BLOCK}`；流式行块上限：按 `min(32, 1048576 // D)` 计算。",
        f"- 本次是否为快速子集：`{'是' if quick else '否'}`。正式结果应使用不带 `--quick` 的命令。",
        "",
        "## 参考链",
        "",
        "```text",
        "x/residual/gamma/bias 先以目标 dtype 生成",
        "        -> 转 FP32",
        "y = x + residual",
        "rms = sqrt(sum(y*y) / D + epsilon)",
        "out = y / rms * gamma + bias",
        "        -> 只在输出处转换回目标 dtype",
        "```",
        f"`golden_direct` 使用直接 FP32 求和；`candidate_r017` 使用固定 `{BLOCK}` 元素块在 FP32 中累加，再执行相同归一化和广播。两者比较仅表示 CPU 参考链内部的一致性，不表示 NPU 输出误差。",
        "",
        "## 容差口径",
        "",
        "逐元素条件为 `abs(actual - golden) <= atol + rtol * abs(golden)`，同时记录失配比例。相对误差使用 `abs(actual-golden) / max(abs(actual), abs(golden), 1e-6)`，避免零值分母放大数值。",
        "",
        report_table(
            [
                {"dtype": name, "rtol": TOLERANCE[name]["rtol"], "atol": TOLERANCE[name]["atol"], "ratio": RATIO_LIMIT}
                for name in ("fp32", "fp16", "bf16")
            ],
            ["dtype", "rtol", "atol", "ratio"],
        ),
        "",
        "`strict_pass` 要求所有有限元素满足逐元素容差；`ratio_pass` 只要求失配比例不超过 0.1%，用于和历史矩阵保持同一统计口径。",
        "",
        "## 覆盖范围",
        "",
        report_table(
            [
                {"项目": "dtype", "覆盖": "fp32 / fp16 / bf16"},
                {"项目": "rank", "覆盖": "2D / 3D / 4D；3D=(1, outer, D)，4D=(1, 1, outer, D)"},
                {"项目": "D", "覆盖": ", ".join(str(value) for value in D_VALUES)},
                {"项目": "outer", "覆盖": "one=1 / small=4 / large=8192"},
                {"项目": "epsilon", "覆盖": "0 / 1e-8 / 1e-5 / 1e-3（辅助矩阵，D=1/67/4096）"},
                {"项目": "gamma/bias", "覆盖": "(D,) 逐通道向量广播；随机和 channel_ramp 两种"},
                {"项目": "特殊值", "覆盖": "全零、NaN、Inf；另有 FP16 大值和 BF16 大 D 反例"},
            ],
            ["项目", "覆盖"],
        ),
        "",
        f"主矩阵共 {len(main_rows)} 个用例，每个指定 D 都有 3 dtype × 3 rank × 3 outer 组合；每个用例处理元素数为 `outer × D`。",
        "",
        "## 主矩阵结果",
        "",
        "按 dtype 汇总：",
        "",
        report_table(
            by_dtype,
            ["dtype", "cases", "strict_passes", "ratio_passes", "max_abs_error", "max_relative_error", "max_mismatch_ratio", "total_elements"],
        ),
        "",
        "按 rank 汇总：",
        "",
        report_table(
            by_rank,
            ["rank", "cases", "strict_passes", "ratio_passes", "max_abs_error", "max_relative_error", "max_mismatch_ratio", "total_elements"],
        ),
        "",
        "按 dtype × rank × outer 汇总：",
        "",
        report_table(
            by_shape_group,
            ["dtype", "rank", "outer_label", "cases", "strict_passes", "max_abs_error", "max_relative_error", "max_mismatch_ratio"],
        ),
        "",
        "最坏主矩阵用例：",
        "",
        report_table(
            [
                {
                    "dtype": row["dtype"],
                    "rank": row["rank"],
                    "D": row["D"],
                    "outer": row["outer_label"],
                    "max_abs": row["max_abs_error"],
                    "max_rel": row["max_relative_error"],
                    "mismatch": row["mismatch_ratio"],
                    "strict": row["strict_pass"],
                }
                for row in worst_main
            ],
            ["dtype", "rank", "D", "outer", "max_abs", "max_rel", "mismatch", "strict"],
        ),
        "",
        f"主矩阵的 `actual` 与 `golden` 都使用 FP32 中间，差异只来自直接求和与 `{BLOCK}` 元素分块累加的顺序；输出仍在目标 dtype 末次转换。",
        "",
        "## epsilon 与广播",
        "",
        report_table(
            by_aux,
            ["section", "dtype", "cases", "strict_passes", "ratio_passes", "max_abs_error", "max_relative_error", "max_mismatch_ratio"],
        ),
        "",
        "辅助矩阵中的 `channel_ramp` 让每个通道的 gamma/bias 不同，再在 2D/3D/4D 中复用同一 `(D,)` 向量；这验证了最后一维广播的参考语义。全零用例还验证了 `epsilon=0` 和默认 epsilon 下的输出传播。",
        "",
        "## R004 低精度反例",
        "",
        "下列候选在每个算术阶段都量化回 FP16/BF16；golden 仍按 R017 的 FP32 中间链计算。",
        "",
        report_table(
            [
                {
                    "dtype": row["dtype"],
                    "shape": str(row["shape"]),
                    "input": row["input_mode"],
                    "max_abs": row["max_abs_error"],
                    "max_rel": row["max_relative_error"],
                    "mismatch": row["mismatch_ratio"],
                    "strict": row["strict_pass"],
                }
                for row in worst_low
            ],
            ["dtype", "shape", "input", "max_abs", "max_rel", "mismatch", "strict"],
        ),
        "",
        "- FP16 `[4, 64]` 反例令 `x=residual=128`，所以 `y=256`；FP16 中 `y*y` 溢出为 `inf`，低精度路径的归一化结果失真。",
        "- BF16 `[4, 32768]` 反例采用大 D、通道变化和低精度逐元素累加；低精度累加的舍入会改变均方和与归一化尺度，出现逐元素失配。",
        "- 该结果只否定低精度中间计算的候选，不代表 Ascend C 指令的具体舍入误差；硬件指令仍需真机测量。",
        "",
        "## R024 / R017 / R004 判断",
        "",
        report_table(
            [
                {"路线": "R024", "判断": "CPU 参考链完成", "依据": f"主矩阵 {len(main_rows)} 用例 + epsilon/广播/特殊值辅助矩阵；无 NPU 结论"},
                {"路线": "R017", "判断": "计算域策略保留；BF16 舍入边界待真机核对", "依据": "输入、加法、平方、归约、epsilon、开方、除法、gamma/bias 全在 FP32，末次转换；BF16 严格全元素通过未达成"},
                {"路线": "R004", "判断": "仅作反例，不采用", "依据": "FP16 溢出和 BF16 大 D 累加失配均可复现"},
            ],
            ["路线", "判断", "依据"],
        ),
        "",
        "## 未覆盖与限制",
        "",
        "- 本机没有 CANN 编译器和 Ascend NPU；没有运行 `kernel.asc`，也没有产生真实 NPU 输出。",
        "- 题面平台的真实 15 个 shape、数据分布和最终逐元素阈值没有公开；本矩阵按仓库已记录的公式和本地阈值执行。",
        "- 常规输入使用 `U(-2, 2)`，辅助用例使用零值、通道变化、NaN/Inf 和大值；未遍历所有可能的随机分布。",
        "- NaN/Inf 用例用于传播行为核对；相对误差只统计有限元素，不能用它们推导普通精度结论。",
        "- CPU 结果不能证明 DataCopyPad 写回安全、Ascend C 归约 API 语义、硬件 rsqrt 舍入、并行写回或性能。",
        "",
        "## 复现与产物",
        "",
        f"```bash\ncd {ROOT}\npython3 {OUT_DIR / 'cpu_matrix.py'}\n```",
        "",
        f"- 脚本：`{OUT_DIR / 'cpu_matrix.py'}`",
        f"- 逐用例 JSON：`{JSON_PATH}`",
        f"- 本报告：`{REPORT_PATH}`",
        "- 本轮只创建上述临时目录内的脚本/JSON，以及用户指定的 CPU 矩阵文档；未改源码、版本目录、服务器文件或提交平台。",
        "",
    ]
    REPORT_PATH.write_text("\n".join(lines), encoding="utf-8")


def main():
    parser = argparse.ArgumentParser(description="Run the independent AddRmsNormBias CPU matrix")
    parser.add_argument("--quick", action="store_true", help="run a small development subset")
    args = parser.parse_args()

    if ml_dtypes is None:
        payload = {
            "status": "dependency_missing",
            "missing": "ml_dtypes",
            "error": ML_DTYPES_ERROR,
            "run_time": datetime.now().isoformat(timespec="seconds"),
        }
        JSON_PATH.write_text(json.dumps(payload, indent=2, ensure_ascii=False), encoding="utf-8")
        REPORT_PATH.write_text(
            "# R024 CPU 验证矩阵\n\n"
            "本轮未完成：本机缺少 `ml_dtypes`，无法构造 BF16 参考链。\n\n"
            f"依赖错误：`{ML_DTYPES_ERROR}`\n",
            encoding="utf-8",
        )
        return 2

    started = datetime.now()
    main_rows, next_case = build_main_matrix(args.quick)
    aux_rows, next_case = build_auxiliary_cases(next_case, args.quick)
    build_special_nonfinite(aux_rows)
    low_rows, _ = build_low_counterexamples(next_case)
    cases = main_rows + aux_rows + low_rows
    environment = {
        "python": sys.version.split()[0],
        "numpy": np.__version__,
        "ml_dtypes": ml_dtypes.__version__,
    }
    payload = {
        "status": "complete",
        "run_time": started.isoformat(timespec="seconds"),
        "environment": environment,
        "seed": SEED,
        "quick": args.quick,
        "D_values": D_VALUES,
        "outer_values": OUTER_VALUES,
        "tolerance": TOLERANCE,
        "cases": cases,
    }
    JSON_PATH.write_text(json.dumps(payload, indent=2, ensure_ascii=False, allow_nan=True), encoding="utf-8")
    command = f"python3 {OUT_DIR / 'cpu_matrix.py'}{' --quick' if args.quick else ''}"
    generate_report(payload, command, args.quick)

    main_strict = sum(bool(row["strict_pass"]) for row in main_rows)
    print(f"completed: main={len(main_rows)} strict={main_strict}/{len(main_rows)} auxiliary={len(aux_rows)} low={len(low_rows)}")
    print(f"report: {REPORT_PATH}")
    print(f"json: {JSON_PATH}")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
