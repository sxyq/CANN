#!/usr/bin/env python3
"""Reproduce the observed V001 reference difference without an NPU call."""

import json

import numpy as np


def main():
    rows, width = 16, 16384
    index = np.arange(rows * width, dtype=np.uint64)
    x = ((((index % 2047) * 37 + 11) % 2047).astype(np.int64) - 1023).astype(np.float32)
    residual = ((((index % 2047) * 17 + 3) % 2047).astype(np.int64) - 1023).astype(np.float32)
    x = (x / np.float32(1024)).astype(np.float16).reshape(rows, width)
    residual = (residual / np.float32(1024)).astype(np.float16).reshape(rows, width)
    col = np.arange(width, dtype=np.int64)
    gamma = (np.float32(0.75) + ((col * 13) % 100).astype(np.float32) / np.float32(200)).astype(np.float16)
    bias = (((col * 7) % 100).astype(np.float32) / np.float32(400) - np.float32(0.125)).astype(np.float16)
    value = x.astype(np.float32) + residual.astype(np.float32)
    rms = np.sqrt(np.mean(value * value, axis=-1, keepdims=True) + np.float32(1e-5))
    expected = (value / rms * gamma.astype(np.float32) + bias.astype(np.float32)).astype(np.float16)
    value_half = (x + residual).astype(np.float16)
    staged_rms = np.sqrt(np.mean(value_half.astype(np.float32) ** 2, axis=-1, keepdims=True) + np.float32(1e-5))
    staged = ((value_half.astype(np.float32) / staged_rms).astype(np.float16) * gamma + bias).astype(np.float16)
    errors = np.abs(staged.astype(np.float32) - expected.astype(np.float32))
    failures = np.flatnonzero(errors > np.float32(0.001) + np.float32(0.001) * np.abs(expected.astype(np.float32)))
    print(json.dumps({
        "shape": [rows, width], "dtype": "fp16", "numpy_version": np.__version__,
        "reference_source": "归档/历史工作区/WIDE-X/scripts/AddRmsNormBias.py:impl",
        "reference": "FP32 formula, final FP16", "comparison": "emulated staged FP16 arithmetic",
        "atol": 0.001, "rtol": 0.001, "failure_count": len(failures),
        "failure_samples": [{
            "index": int(i), "row": int(i // width), "col": int(i % width),
            "x": float(x.flat[i]), "residual": float(residual.flat[i]),
            "gamma": float(gamma[i % width]), "bias": float(bias[i % width]),
            "reference": float(expected.flat[i]), "staged": float(staged.flat[i]),
            "abs_error": float(errors.flat[i]),
        } for i in failures],
        "npu_invoked": False,
    }, ensure_ascii=False, indent=2))


if __name__ == "__main__":
    main()
