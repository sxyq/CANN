#!/usr/bin/env python3
import hashlib
import json
import math
import struct
import sys
from pathlib import Path


ROWS = 2
WIDTH = 16384
EPSILON = struct.unpack("<f", struct.pack("<f", 1.0e-5))[0]
ATOL = 1.0 / 65536.0
RTOL = 1.0 / 1024.0
MAX_ABS_LIMIT = 1.0e-2
ELEMENTS = ROWS * WIDTH


def read_bytes(directory, name, expected_bytes):
    data = (directory / name).read_bytes()
    if len(data) != expected_bytes:
        raise ValueError(f"{name}: expected {expected_bytes} bytes, got {len(data)}")
    return data


def unpack_f32(data):
    count = len(data) // 4
    return struct.unpack(f"<{count}f", data)


def evaluate(inputs, output_bytes):
    output = unpack_f32(output_bytes)
    x, residual, gamma, bias = inputs
    max_abs = 0.0
    bad = 0
    nonfinite = 0

    for row in range(ROWS):
        start = row * WIDTH
        values = [float(x[i]) + float(residual[i])
                  for i in range(start, start + WIDTH)]
        square_sum = math.fsum(value * value for value in values)
        inv_rms = 1.0 / math.sqrt(square_sum / WIDTH + EPSILON)
        for col, value in enumerate(values):
            expected = value * inv_rms * float(gamma[col]) + float(bias[col])
            actual = float(output[start + col])
            error = abs(actual - expected)
            if not math.isfinite(actual) or not math.isfinite(expected):
                error = math.inf
                bad += 1
                nonfinite += 1
                max_abs = max(max_abs, error)
                continue
            max_abs = max(max_abs, error)
            if error > ATOL + RTOL * abs(expected):
                bad += 1

    matched_ratio = 1.0 - bad / ELEMENTS
    return {
        "output_sha256": hashlib.sha256(output_bytes).hexdigest(),
        "max_abs": max_abs if math.isfinite(max_abs) else None,
        "nonfinite": nonfinite,
        "bad": bad,
        "matched_ratio": matched_ratio,
        "pass": matched_ratio >= 0.99 and max_abs <= MAX_ABS_LIMIT,
    }


def main():
    if len(sys.argv) != 2:
        raise ValueError("usage: verify_cpu_golden.py invocation_dir")
    directory = Path(sys.argv[1])
    data_bytes = ELEMENTS * 4
    param_bytes = WIDTH * 4
    names = ("x", "residual", "gamma", "bias")
    sizes = (data_bytes, data_bytes, param_bytes, param_bytes)
    host = [read_bytes(directory, f"host_pre_h2d_{name}.bin", size)
            for name, size in zip(names, sizes)]
    device = [read_bytes(directory, f"device_after_h2d_{name}.bin", size)
              for name, size in zip(names, sizes)]
    input_equal = {name: host_bytes == device_bytes
                   for name, host_bytes, device_bytes in zip(names, host, device)}
    inputs = [unpack_f32(data) for data in host]

    first = read_bytes(directory, "output_d2h_first.bin", data_bytes)
    second = read_bytes(directory, "output_d2h_second.bin", data_bytes)
    results = {
        "shape": [ROWS, WIDTH],
        "dtype": "FP32",
        "h2d_readback_equal": input_equal,
        "d2h_copies_equal": first == second,
        "first": evaluate(inputs, first),
        "second": evaluate(inputs, second),
    }
    results["pass"] = (
        all(input_equal.values()) and results["d2h_copies_equal"] and
        results["first"]["pass"] and results["second"]["pass"]
    )
    result_path = directory / "cpu-golden.json"
    with result_path.open("x", encoding="utf-8") as target:
        json.dump(results, target, indent=2, allow_nan=False)
        target.write("\n")
    print(json.dumps(results, indent=2, allow_nan=False))
    return 0 if results["pass"] else 3


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except Exception as error:
        print(f"cpu golden verifier error: {error}", file=sys.stderr)
        raise SystemExit(2)
