#!/usr/bin/env python3
import math
import struct
import sys


def read_f32(path):
    data = open(path, "rb").read()
    if len(data) % 4:
        raise ValueError(f"{path}: byte length is not divisible by 4")
    return struct.unpack("<%df" % (len(data) // 4), data)


baseline = read_f32(sys.argv[1])
candidate = read_f32(sys.argv[2])
if len(baseline) != 64 or len(candidate) != 64:
    raise SystemExit("PAIR_GUARD=FAIL expected exactly 64 outputs per run")
max_abs = max(abs(a - b) for a, b in zip(baseline, candidate))
if not all(math.isfinite(v) for v in baseline + candidate):
    raise SystemExit("PAIR_GUARD=FAIL non-finite output")
if max_abs > 1.0e-4:
    raise SystemExit(f"PAIR_GUARD=FAIL baseline_delta={max_abs:.8g}")
print(f"PAIR_GUARD=PASS elements={len(baseline)} max_abs={max_abs:.8g}")
