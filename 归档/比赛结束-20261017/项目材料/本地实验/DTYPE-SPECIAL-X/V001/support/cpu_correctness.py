#!/usr/bin/env python3
"""CPU-only FP32 reference and conversion-helper OFAT checks for V001."""

import json
from pathlib import Path

import numpy as np


ATOL = 2.0 ** -16
RTOL = 2.0 ** -10
MAX_ABS_LIMIT = 1e-2
REQUIRED_RATIO = 0.99


def aligned_f32(count: int, misaligned: bool = False) -> np.ndarray:
    raw = np.empty(count + 16, dtype=np.float32)
    base = raw.ctypes.data
    first = ((32 - base % 32) % 32) // raw.itemsize
    if misaligned:
        first += 1
    return raw[first : first + count]


def copy_or_add_zero(src: np.ndarray, dst: np.ndarray, candidate: bool) -> str:
    use_copy = (
        candidate
        and src.size > 0
        and src.nbytes % 32 == 0
        and src.ctypes.data % 32 == 0
        and dst.ctypes.data % 32 == 0
    )
    if use_copy:
        np.copyto(dst, src)
        return "DataCopy"
    np.add(src, np.float32(0.0), out=dst)
    return "Adds"


def matched(actual: np.ndarray, golden: np.ndarray) -> tuple[float, float]:
    finite = np.isfinite(actual) & np.isfinite(golden)
    if not finite.all():
        raise AssertionError("finite-value correctness matrix produced non-finite output")
    error = np.abs(actual.astype(np.float64) - golden.astype(np.float64))
    passed = error <= ATOL + RTOL * np.abs(golden.astype(np.float64))
    return float(passed.mean()), float(error.max(initial=0.0))


def helper_matrix(seed: int) -> dict:
    rng = np.random.default_rng(seed)
    specs = [
        ("aligned_copy_8", 8, False, False),
        ("aligned_copy_64", 64, False, False),
        ("unaligned_count_7", 7, False, False),
        ("unaligned_source_16", 16, True, False),
        ("unaligned_destination_16", 16, False, True),
    ]
    rows = []
    for helper in ("ToFloat", "FromFloat"):
        for name, count, src_misaligned, dst_misaligned in specs:
            src = aligned_f32(count, src_misaligned)
            dst_candidate = aligned_f32(count, dst_misaligned)
            dst_parent = aligned_f32(count, dst_misaligned)
            src[:] = rng.uniform(-5.0, 5.0, count).astype(np.float32)
            if count >= 4:
                src[:4] = np.array([-0.0, 0.0, 1e-30, -1e-30], dtype=np.float32)
            candidate_api = copy_or_add_zero(src, dst_candidate, True)
            parent_api = copy_or_add_zero(src, dst_parent, False)
            ratio, max_abs = matched(dst_candidate, dst_parent)
            rows.append(
                {
                    "helper": helper,
                    "case": name,
                    "count": count,
                    "candidate_api": candidate_api,
                    "parent_api": parent_api,
                    "numeric_equal_ratio": ratio,
                    "max_abs_error": max_abs,
                    "bitwise_equal": bool(np.array_equal(dst_candidate.view(np.uint32), dst_parent.view(np.uint32))),
                }
            )
    return {"case_count": len(rows), "cases": rows}


def operator_output(x: np.ndarray, residual: np.ndarray, gamma: np.ndarray, bias: np.ndarray,
                    epsilon: float, candidate_helpers: bool) -> np.ndarray:
    x32 = np.empty_like(x)
    residual32 = np.empty_like(residual)
    copy_or_add_zero(x, x32, candidate_helpers)
    copy_or_add_zero(residual, residual32, candidate_helpers)
    y = np.add(x32, residual32, dtype=np.float32)
    mean_square = np.mean(y.astype(np.float64) ** 2, axis=-1, keepdims=True)
    rms = np.sqrt(mean_square + epsilon)
    result32 = ((y.astype(np.float64) / rms) * gamma.astype(np.float64) + bias.astype(np.float64)).astype(np.float32)
    output = np.empty_like(result32)
    copy_or_add_zero(result32, output, candidate_helpers)
    return output


def reference_matrix(seed: int) -> dict:
    rng = np.random.default_rng(seed)
    widths = [1, 7, 8, 9, 63, 64, 65, 127, 128, 129, 1024, 4096, 8192]
    cases = []
    for rank in (2, 3, 4):
        for width in widths:
            prefix = (3,) if rank == 2 else ((2, 2) if rank == 3 else (1, 2, 2))
            shape = prefix + (width,)
            x = rng.uniform(-5.0, 5.0, shape).astype(np.float32)
            residual = rng.uniform(-5.0, 5.0, shape).astype(np.float32)
            if width >= 2:
                x.reshape(-1, width)[0, :2] = 0.0
                residual.reshape(-1, width)[0, :2] = 0.0
            gamma = rng.uniform(-2.0, 2.0, width).astype(np.float32)
            bias = rng.uniform(-1.0, 1.0, width).astype(np.float32)
            epsilon = 1e-5

            y64 = x.astype(np.float64) + residual.astype(np.float64)
            ref = (y64 / np.sqrt(np.mean(y64 * y64, axis=-1, keepdims=True) + epsilon)
                   * gamma.astype(np.float64) + bias.astype(np.float64)).astype(np.float32)
            parent = operator_output(x, residual, gamma, bias, epsilon, False)
            candidate = operator_output(x, residual, gamma, bias, epsilon, True)
            parent_ratio, parent_max = matched(parent, ref)
            candidate_ratio, candidate_max = matched(candidate, ref)
            pair_ratio, pair_max = matched(candidate, parent)
            passed = (
                candidate_ratio >= REQUIRED_RATIO
                and candidate_max <= MAX_ABS_LIMIT
                and pair_ratio == 1.0
                and pair_max == 0.0
            )
            cases.append(
                {
                    "shape": list(shape),
                    "rank": rank,
                    "dtype": "FP32",
                    "epsilon": epsilon,
                    "candidate_matched_ratio": candidate_ratio,
                    "candidate_max_abs_error": candidate_max,
                    "parent_matched_ratio": parent_ratio,
                    "parent_max_abs_error": parent_max,
                    "candidate_vs_parent_ratio": pair_ratio,
                    "candidate_vs_parent_max_abs_error": pair_max,
                    "pass": passed,
                }
            )
    return {
        "case_count": len(cases),
        "dtypes": ["FP32"],
        "ranks": [2, 3, 4],
        "widths": widths,
        "precision": {
            "atol": ATOL,
            "rtol": RTOL,
            "required_matched_ratio": REQUIRED_RATIO,
            "max_abs_error_limit": MAX_ABS_LIMIT,
        },
        "cases": cases,
        "pass": all(case["pass"] for case in cases),
    }


def main() -> None:
    seed = 20260924
    output = {
        "route": "DTYPE-SPECIAL-X",
        "revision": "V001",
        "execution": "CPU reference only; no NPU execution and no timing",
        "seed": seed,
        "helper_ofat": helper_matrix(seed),
        "operator_reference": reference_matrix(seed + 1),
    }
    output["pass"] = output["operator_reference"]["pass"]
    print(json.dumps(output, indent=2))
    if not output["pass"]:
        raise SystemExit(1)


if __name__ == "__main__":
    main()
