#!/usr/bin/env python3
"""Independent CPU reference matrix for FULL-R024.

This script validates the mathematical contract only. It never supplies an
output to the device kernel and cannot replace CANN/NPU evidence.
"""
import json
import math
import pathlib
import sys

try:
    import numpy as np
except ImportError as exc:
    print(f"numpy unavailable: {exc}", file=sys.stderr)
    raise SystemExit(2)

try:
    import ml_dtypes
except ImportError:
    ml_dtypes = None


EPSILONS = (1.0e-5, 1.0e-3)
WIDTHS = (1, 31, 32, 33, 67, 127, 128, 129, 1000, 4096)
OUTERS = (1, 2, 7, 64)
RANKS = (2, 3, 4)


def cast_dtype(values, dtype_name):
    if dtype_name == "fp32":
        return np.asarray(values, dtype=np.float32)
    if dtype_name == "fp16":
        return np.asarray(values, dtype=np.float16)
    if dtype_name == "bf16" and ml_dtypes is not None:
        return np.asarray(values, dtype=ml_dtypes.bfloat16)
    if dtype_name == "bf16":
        # Keep the matrix executable on hosts without ml_dtypes. This is a
        # deterministic round-to-nearest-even emulation for BF16 storage.
        raw = np.asarray(values, dtype=np.float32).view(np.uint32)
        rounded = raw + ((raw >> 16) & 1) + 0x7FFF
        return (rounded & 0xFFFF0000).view(np.float32)
    raise ValueError(dtype_name)


def to_fp32(values):
    return np.asarray(values, dtype=np.float32)


def reference(x, residual, gamma, bias, epsilon):
    y = to_fp32(x) + to_fp32(residual)
    rms = np.sqrt(np.mean(y * y, axis=-1, keepdims=True) + np.float32(epsilon))
    return y / rms * to_fp32(gamma) + to_fp32(bias)


def blocked_reference(x, residual, gamma, bias, epsilon, block=4096):
    y = to_fp32(x) + to_fp32(residual)
    sums = np.zeros(y.shape[:-1] + (1,), dtype=np.float32)
    for start in range(0, y.shape[-1], block):
        tile = y[..., start:start + block]
        sums += np.sum(tile * tile, axis=-1, keepdims=True, dtype=np.float32)
    rms = np.sqrt(sums / np.float32(y.shape[-1]) + np.float32(epsilon))
    return y / rms * to_fp32(gamma) + to_fp32(bias)


def make_shape(rank, outer, width):
    if rank == 2:
        return (outer, width)
    if rank == 3:
        return (1, outer, width)
    return (1, 1, outer, width)


def make_inputs(shape, dtype_name, seed):
    rng = np.random.default_rng(seed)
    x = rng.normal(0.0, 0.75, size=shape).astype(np.float32)
    residual = rng.normal(0.0, 0.25, size=shape).astype(np.float32)
    gamma = rng.normal(1.0, 0.05, size=(shape[-1],)).astype(np.float32)
    bias = rng.normal(0.0, 0.05, size=(shape[-1],)).astype(np.float32)
    return (cast_dtype(x, dtype_name), cast_dtype(residual, dtype_name),
            cast_dtype(gamma, dtype_name), cast_dtype(bias, dtype_name))


def one_case(dtype_name, rank, outer, width, epsilon, seed):
    shape = make_shape(rank, outer, width)
    x, residual, gamma, bias = make_inputs(shape, dtype_name, seed)
    expected = reference(x, residual, gamma, bias, epsilon)
    actual = blocked_reference(x, residual, gamma, bias, epsilon)
    diff = np.abs(actual - expected)
    scale = np.maximum(np.abs(expected), 1.0e-12)
    rel = diff / scale
    finite = np.isfinite(expected) & np.isfinite(actual)
    mismatch = (~finite) | ((diff > 1.0e-3) & (rel > 1.0e-3))
    return {
        "dtype": dtype_name,
        "rank": rank,
        "outer": outer,
        "D": width,
        "epsilon": epsilon,
        "max_abs": float(np.max(diff)),
        "max_rel": float(np.max(rel)),
        "mismatch_count": int(np.count_nonzero(mismatch)),
        "elements": int(expected.size),
        "pass": bool(not np.any(mismatch)),
    }


def main():
    rows = []
    seed = 24024
    for dtype_name in ("fp32", "fp16", "bf16"):
        for rank in RANKS:
            for outer in OUTERS:
                for width in WIDTHS:
                    for epsilon in EPSILONS:
                        rows.append(one_case(dtype_name, rank, outer, width,
                                             epsilon, seed))
                        seed += 1
    special = [
        ("zero", np.zeros((2, 33), dtype=np.float32),
         np.zeros((2, 33), dtype=np.float32)),
        ("large", np.full((2, 67), 32.0, dtype=np.float32),
         np.full((2, 67), -31.0, dtype=np.float32)),
    ]
    special_rows = []
    for name, x, residual in special:
        gamma = np.ones((x.shape[-1],), dtype=np.float32)
        bias = np.zeros((x.shape[-1],), dtype=np.float32)
        expected = reference(x, residual, gamma, bias, 1.0e-5)
        actual = blocked_reference(x, residual, gamma, bias, 1.0e-5)
        special_rows.append({"name": name, "max_abs": float(np.max(np.abs(actual - expected))),
                             "pass": bool(np.allclose(actual, expected, rtol=1e-3, atol=1e-3))})
    output = {
        "status": "complete",
        "route": "FULL-R024-CPU-VALIDATION-MATRIX",
        "matrix_cases": len(rows),
        "matrix_pass": sum(row["pass"] for row in rows),
        "matrix_fail": sum(not row["pass"] for row in rows),
        "special_cases": special_rows,
        "rows": rows,
        "note": "CPU/reference evidence only; no host result is used by the kernel.",
    }
    out = pathlib.Path(__file__).with_name("cpu_validation_matrix.json")
    out.write_text(json.dumps(output, indent=2, sort_keys=True) + "\n")
    print(json.dumps({key: output[key] for key in ("status", "matrix_cases", "matrix_pass", "matrix_fail")}, sort_keys=True))
    return 0 if output["matrix_fail"] == 0 else 1


if __name__ == "__main__":
    raise SystemExit(main())
