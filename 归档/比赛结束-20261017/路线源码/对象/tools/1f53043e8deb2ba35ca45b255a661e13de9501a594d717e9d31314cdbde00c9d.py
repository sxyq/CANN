#!/usr/bin/env python3
import os, statistics as st

OUT = "/home/data4t2/lelinfeng/phase4-workspaces/COEFF-LOCALITY-X/results-timing-v003-20260929"
SHAPES = ["1x32768_fp32", "1x16384_fp32", "8x32768_fp32", "1x4096_fp32", "1x8192_fp32"]

def load_raw(path):
    rows = []
    with open(path) as f:
        for line in f:
            parts = line.strip().split("\t")
            if len(parts) < 3 or parts[0] == "block":
                continue
            try:
                rows.append((int(parts[0]), int(parts[1]), float(parts[2])))
            except ValueError:
                pass
    return rows

def all_stats(rows):
    vals = [r[2] for r in rows]
    med = st.median(vals)
    mad = st.median([abs(v - med) for v in vals])
    return len(vals), med, mad, (mad / med if med else None)

def block_med(rows, b):
    vals = [r[2] for r in rows if r[0] == b]
    return st.median(vals) if vals else None

print("=== SAME-BINARY ===")
print("shape\tvariant\tn\tmed_us\tMAD_us\tMAD/med\tdrift\tqual")
for shape in SHAPES:
    for v in ("P", "C"):
        p = os.path.join(OUT, "sb-" + v + "-" + shape + "-raw.tsv")
        if not os.path.exists(p):
            print(shape + "\t" + v + "\tMISSING")
            continue
        rows = load_raw(p)
        n, med, mad, mm = all_stats(rows)
        b1 = block_med(rows, 1)
        b2 = block_med(rows, 2)
        drift = (b2 - b1) / b1 if (b1 and b2) else float("nan")
        ok = (mm is not None and mm <= 0.10 and abs(drift) <= 0.10)
        print("%s\t%s\t%d\t%.3f\t%.3f\t%.4f\t%.4f\t%s" % (shape, v, n, med, mad, mm, drift, ok))

print()
print("=== PAIRED P/C ===")
print("shape\tpair\tP_med\tC_med\tdelta%\tfav")
for shape in SHAPES:
    deltas = []
    for pair in range(1, 7):
        pp = os.path.join(OUT, "pc%d-P-%s-raw.tsv" % (pair, shape))
        cp = os.path.join(OUT, "pc%d-C-%s-raw.tsv" % (pair, shape))
        if not (os.path.exists(pp) and os.path.exists(cp)):
            continue
        _, pm, _, _ = all_stats(load_raw(pp))
        _, cm, _, _ = all_stats(load_raw(cp))
        if pm <= 0:
            continue
        d = (cm - pm) / pm * 100.0
        fav = "P" if d > 0 else "C"
        deltas.append(d)
        print("%s\t%d\t%.3f\t%.3f\t%+.2f\t%s" % (shape, pair, pm, cm, d, fav))
    if deltas:
        med_d = st.median(deltas)
        favP = sum(1 for d in deltas if d > 0)
        favC = sum(1 for d in deltas if d < 0)
        print("%s\tMEDIAN\t\t\t%+.2f\tfavP=%d favC=%d n=%d" % (shape, med_d, favP, favC, len(deltas)))
    print()
