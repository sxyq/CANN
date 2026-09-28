#!/usr/bin/env python3
import math
import os
import random
import struct
import subprocess
import sys


def pack(values, dtype):
    if dtype == 0:
        return struct.pack("<%df" % len(values), *values)
    out = bytearray()
    for value in values:
        bits = struct.unpack("<I", struct.pack("<f", float(value)))[0]
        if dtype in (2, 27):
            bits = (bits + 0x7FFF + ((bits >> 16) & 1)) >> 16
            out += struct.pack("<H", bits & 0xFFFF)
        else:
            out += struct.pack("<e", float(value))
    return bytes(out)


def unpack(data, dtype):
    if dtype == 0:
        return struct.unpack("<%df" % (len(data) // 4), data)
    values = []
    for (half,) in struct.iter_unpack("<H", data):
        if dtype in (2, 27):
            values.append(struct.unpack("<f", struct.pack("<I", half << 16))[0])
        else:
            values.append(struct.unpack("<e", struct.pack("<H", half))[0])
    return values


def run_case(exe, name, rows, dim, dtype, tolerance):
    random.seed(1000 + rows * 17 + dim * 3 + dtype)
    count = rows * dim
    x = [random.uniform(-2.0, 2.0) for _ in range(count)]
    residual = [random.uniform(-2.0, 2.0) for _ in range(count)]
    gamma = [random.uniform(0.8, 1.2) for _ in range(dim)]
    bias = [random.uniform(-0.3, 0.3) for _ in range(dim)]
    os.makedirs("input", exist_ok=True)
    os.makedirs("output", exist_ok=True)
    for filename, values in (("x.bin", x), ("residual.bin", residual), ("gamma.bin", gamma), ("bias.bin", bias)):
        with open(os.path.join("input", filename), "wb") as file:
            file.write(pack(values, dtype))
    reference = []
    for row in range(rows):
        start = row * dim
        u = [x[start + i] + residual[start + i] for i in range(dim)]
        inv = 1.0 / math.sqrt(sum(value * value for value in u) / dim + 1e-5)
        reference.extend(u[i] * inv * gamma[i] + bias[i] for i in range(dim))
    subprocess.run([exe, str(rows), str(dim), str(dtype)], check=True, timeout=30)
    with open("output/output.bin", "rb") as file:
        actual = unpack(file.read(), dtype)
    expected = unpack(pack(reference, dtype), dtype)
    errors = [abs(a - b) for a, b in zip(actual, expected)]
    max_abs = max(errors) if errors else 0.0
    bad = sum(error > tolerance for error in errors)
    print("%s rows=%d dim=%d dtype=%d max_abs=%.6g bad=%d" % (name, rows, dim, dtype, max_abs, bad))
    return bad == 0 and len(actual) == len(expected)


def main():
    exe = sys.argv[1] if len(sys.argv) > 1 else "./epix_full"
    cases = [
        ("wide_epilogue", 1, 32768, 1, 0.05),
        ("adjacent_dtype_f16", 2, 2048, 1, 0.05),
        ("adjacent_dtype_bf16_27", 2, 2048, 27, 0.12),
        ("tail_fallback", 3, 511, 0, 0.01),
        ("tail_fallback_low", 3, 511, 1, 0.05),
    ]
    return 0 if all(run_case(exe, *case) for case in cases) else 1


if __name__ == "__main__":
    sys.exit(main())
