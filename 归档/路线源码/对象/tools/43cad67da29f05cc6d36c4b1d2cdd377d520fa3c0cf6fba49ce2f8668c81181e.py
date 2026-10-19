#!/usr/bin/env python3
"""Multi-case pure-python golden for WIDE-X local check (no numpy)."""
import math
import os
import random
import struct
import sys

sys.path.insert(0, os.path.dirname(__file__))


def pack_f16(vals):
    return b"".join(struct.pack("<e", float(v)) for v in vals)


def pack_f32(vals):
    return b"".join(struct.pack("<f", float(v)) for v in vals)


def unpack_f16(buf):
    n = len(buf) // 2
    return list(struct.unpack("<%de" % n, buf))


def unpack_f32(buf):
    n = len(buf) // 4
    return list(struct.unpack("<%df" % n, buf))


def pack_bf16(vals):
    out = bytearray()
    for v in vals:
        f = float(v)
        bits = struct.unpack("<I", struct.pack("<f", f))[0]
        # round-to-nearest-even when truncating to bf16
        rounding_bias = 0x7FFF + ((bits >> 16) & 1)
        bits = (bits + rounding_bias) >> 16
        out += struct.pack("<H", bits & 0xFFFF)
    return bytes(out)


def unpack_bf16(buf):
    n = len(buf) // 2
    vals = []
    for i in range(n):
        (h,) = struct.unpack_from("<H", buf, i * 2)
        vals.append(struct.unpack("<f", struct.pack("<I", h << 16))[0])
    return vals


def golden_rows(x, residual, gamma, bias, rows, d, eps=1e-5):
    out = []
    for r in range(rows):
        u = [float(x[r * d + i]) + float(residual[r * d + i]) for i in range(d)]
        acc = 0.0
        for v in u:
            acc += v * v
        rms = math.sqrt(acc / d + eps)
        inv = 1.0 / rms
        for i in range(d):
            out.append(u[i] * inv * float(gamma[i]) + float(bias[i]))
    return out


def run_case(tag, rows, d, dtype, pack, unpack, es):
    random.seed(42 + rows + d + dtype)
    n = rows * d
    x = [random.uniform(-2, 2) for _ in range(n)]
    r = [random.uniform(-2, 2) for _ in range(n)]
    g = [random.uniform(0.8, 1.2) for _ in range(d)]
    b = [random.uniform(-0.3, 0.3) for _ in range(d)]
    os.makedirs("input", exist_ok=True)
    os.makedirs("output", exist_ok=True)
    open("input/x.bin", "wb").write(pack(x))
    open("input/residual.bin", "wb").write(pack(r))
    open("input/gamma.bin", "wb").write(pack(g))
    open("input/bias.bin", "wb").write(pack(b))
    ref = golden_rows(x, r, g, b, rows, d)
    open("output/golden_output.bin", "wb").write(pack(ref))
    return tag


def check(tag, rows, d, tol, unpack):
    got = unpack(open("output/output.bin", "rb").read())
    ref = unpack(open("output/golden_output.bin", "rb").read())
    if len(got) != len(ref):
        print("%s FAIL len got=%d ref=%d" % (tag, len(got), len(ref)))
        return 1
    max_abs = 0.0
    bad = 0
    for a, b in zip(got, ref):
        da = abs(a - b)
        max_abs = max(max_abs, da)
        if da > tol:
            bad += 1
    print("%s max_abs=%.6g bad=%d n=%d" % (tag, max_abs, bad, len(got)))
    return 0 if bad == 0 else 1


def main():
    which = sys.argv[1] if len(sys.argv) > 1 else "gen"
    rows, d, dtype = int(sys.argv[2]), int(sys.argv[3]), int(sys.argv[4])
    if dtype == 0:
        pack, unpack, tol = pack_f32, unpack_f32, 5e-3
    elif dtype == 2:
        pack, unpack, tol = pack_bf16, unpack_bf16, 2e-2
    else:
        pack, unpack, tol = pack_f16, unpack_f16, 2e-2
    tag = "r%dd%ddt%d" % (rows, d, dtype)
    if which == "gen":
        run_case(tag, rows, d, dtype, pack, unpack, 4 if dtype == 0 else 2)
        print("generated", tag)
        return 0
    return check(tag, rows, d, tol, unpack)


if __name__ == "__main__":
    sys.exit(main())
