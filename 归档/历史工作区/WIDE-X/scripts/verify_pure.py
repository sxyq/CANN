#!/usr/bin/env python3
"""Pure-python verify for smoke case 0 (no numpy)."""
import math
import struct
import sys


def unpack_f16(buf):
    n = len(buf) // 2
    return list(struct.unpack("<%de" % n, buf))


def main():
    got = unpack_f16(open("output/output.bin", "rb").read())
    ref = unpack_f16(open("output/golden_output.bin", "rb").read())
    if len(got) != len(ref):
        print("FAIL len got=%d ref=%d" % (len(got), len(ref)))
        return 1
    max_abs = 0.0
    max_rel = 0.0
    bad = 0
    for a, b in zip(got, ref):
        da = abs(a - b)
        max_abs = max(max_abs, da)
        denom = max(abs(b), 1e-6)
        rel = da / denom
        max_rel = max(max_rel, rel)
        if da > 5e-2 and rel > 5e-2:
            bad += 1
    print("max_abs=%.6g max_rel=%.6g bad=%d n=%d" % (max_abs, max_rel, bad, len(got)))
    if bad > 0:
        print("FAIL")
        return 1
    print("PASS")
    return 0


if __name__ == "__main__":
    sys.exit(main())
