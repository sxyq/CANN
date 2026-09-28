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
                rows.append(float(parts[2]))
            except ValueError:
                pass
    return rows

def med(path):
    v = load_raw(path)
    return st.median(v) if v else None

print("shape\tpair\tP_med\tC_med\tdelta%\tclean")
for shape in SHAPES:
    pm, cm = {}, {}
    for pair in range(1, 7):
        pp = os.path.join(OUT, "pc%d-P-%s-raw.tsv" % (pair, shape))
        cp = os.path.join(OUT, "pc%d-C-%s-raw.tsv" % (pair, shape))
        if os.path.exists(pp):
            pm[pair] = med(pp)
        if os.path.exists(cp):
            cm[pair] = med(cp)
    if not pm or not cm:
        continue
    pcenter = st.median(list(pm.values()))
    ccenter = st.median(list(cm.values()))
    clean_d = []
    for pair in sorted(set(pm) & set(cm)):
        d = (cm[pair] - pm[pair]) / pm[pair] * 100.0
        clean = (abs(pm[pair] - pcenter) / pcenter <= 0.15 and
                 abs(cm[pair] - ccenter) / ccenter <= 0.15)
        if clean:
            clean_d.append(d)
        print("%s\t%d\t%.3f\t%.3f\t%+.2f\t%s" % (shape, pair, pm[pair], cm[pair], d, clean))
    if clean_d:
        print("%s\tCLEAN_MEDIAN\t(n=%d)\t\t%+.2f\tfavP=%d favC=%d" % (
            shape, len(clean_d), st.median(clean_d),
            sum(1 for x in clean_d if x > 0), sum(1 for x in clean_d if x < 0)))
    else:
        print("%s\tCLEAN_MEDIAN\tno clean pairs" % shape)
    print()
