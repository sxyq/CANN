#!/usr/bin/env python3
"""Pure-python smoke data for rank-3 FP16 D=2048 (no numpy)."""
import math
import os
import random
import struct
import sys

R0, R1, D = 2, 3, 2048
ROWS = R0 * R1
EPS = 1e-5
SEED = 42


def pack_f16(vals):
    return b"".join(struct.pack("<e", float(v)) for v in vals)


def unpack_f16(buf):
    n = len(buf) // 2
    return list(struct.unpack("<%de" % n, buf))


def golden(x, residual, gamma, bias):
    out = []
    for r in range(ROWS):
        row_x = x[r * D:(r + 1) * D]
        row_r = residual[r * D:(r + 1) * D]
        u = [float(a) + float(b) for a, b in zip(row_x, row_r)]
        acc = 0.0
        for v in u:
            acc += v * v
        rms = math.sqrt(acc / D + EPS)
        inv = 1.0 / rms
        for i in range(D):
            out.append(u[i] * inv * float(gamma[i]) + float(bias[i]))
    return out


def main():
    random.seed(SEED)
    os.makedirs("input/case0", exist_ok=True)
    os.makedirs("output/golden_case0", exist_ok=True)

    def runi(lo, hi, n):
        return [random.uniform(lo, hi) for _ in range(n)]

    x = runi(-2, 2, ROWS * D)
    residual = runi(-2, 2, ROWS * D)
    gamma = runi(0.8, 1.2, D)
    bias = runi(-0.3, 0.3, D)

    open("input/case0/x.bin", "wb").write(pack_f16(x))
    open("input/case0/residual.bin", "wb").write(pack_f16(residual))
    open("input/case0/gamma.bin", "wb").write(pack_f16(gamma))
    open("input/case0/bias.bin", "wb").write(pack_f16(bias))

    g = golden(x, residual, gamma, bias)
    open("output/golden_case0/golden_output.bin", "wb").write(pack_f16(g))
    print("generated pure-python smoke case rows=%d d=%d" % (ROWS, D))


if __name__ == "__main__":
    main()
