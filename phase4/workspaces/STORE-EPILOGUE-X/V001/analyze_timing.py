#!/usr/bin/env python3
"""Summarize paired timing for STORE-EPILOGUE-X V001.

Inputs: <prefix>-stats.txt (key\\tfield\\tvalue rows) and <prefix>-raw.tsv.
Primary metric: ALL_DEVICE median_us. Same-binary quality: MAD/median <= 0.15.
Clean pair: |delta| <= 50% and both medians present. Negative delta favors candidate.
"""
import glob
import os
import re
import statistics
import sys

OUT = sys.argv[1] if len(sys.argv) > 1 else "results-timing-v001-20260927"


def parse_stats(path):
    if not os.path.isfile(path):
        return None
    med = mad = None
    with open(path) as f:
        for line in f:
            p = line.strip().split("\t")
            if len(p) != 3:
                continue
            key, field, val = p
            if key == "ALL_DEVICE" and field == "median_us":
                med = float(val)
            elif key == "ALL_DEVICE" and field == "MAD_us":
                mad = float(val)
    if med is None:
        return None
    return med, mad


def load(prefix):
    return parse_stats(prefix + "-stats.txt")


def main():
    tags = sorted(
        {
            m.group(1)
            for p in glob.glob(os.path.join(OUT, "sb-P-*-stats.txt"))
            for m in [re.search(r"sb-P-(.+)-stats\.txt$", os.path.basename(p))]
            if m
        }
    )
    print(f"tags: {tags}\n")
    verdicts = {}
    for tag in tags:
        sp = load(os.path.join(OUT, f"sb-P-{tag}"))
        sc = load(os.path.join(OUT, f"sb-C-{tag}"))

        def sb_status(pair):
            if pair is None:
                return "MISSING"
            med, mad = pair
            if mad is None:
                return "PASS?"
            return "PASS" if mad / med <= 0.15 else "FAIL"

        ps, cs = sb_status(sp), sb_status(sc)
        print(f"== {tag} ==")
        print(f"  same-binary P: {sp} -> {ps} | C: {sc} -> {cs}")
        sb_gate = ps in ("PASS", "PASS?") and cs in ("PASS", "PASS?")
        deltas = []
        for i in range(1, 7):
            pp = load(os.path.join(OUT, f"pc{i}-P-{tag}"))
            cp = load(os.path.join(OUT, f"pc{i}-C-{tag}"))
            if pp is None or cp is None:
                print(f"  pair{i}: MISSING")
                continue
            d = (cp[0] - pp[0]) / pp[0] * 100.0
            order = "P->C" if i % 2 == 1 else "C->P"
            clean = abs(d) <= 50.0
            note = order + (" clean" if clean else " OUTLIER")
            print(f"  pair{i}: P={pp[0]:.3f} C={cp[0]:.3f} delta={d:+.2f}% [{note}]")
            if clean:
                deltas.append(d)
        if deltas:
            med = statistics.median(deltas)
            fav_c = sum(1 for d in deltas if d < 0)
            fav_p = sum(1 for d in deltas if d > 0)
            print(
                f"  PAIRED_DELTA median={med:+.2f}% n_clean={len(deltas)}"
                f" favor_C={fav_c} favor_P={fav_p}"
                f" range=[{min(deltas):+.2f},{max(deltas):+.2f}]"
            )
            verdicts[tag] = (med, len(deltas), fav_c, fav_p, sb_gate)
        else:
            print("  PAIRED_DELTA none")
            verdicts[tag] = None
        print(f"  same_binary_gate={'PASS' if sb_gate else 'FAIL'}\n")

    print("=== SUMMARY ===")
    for tag, v in verdicts.items():
        if v is None:
            print(f"{tag}: BLOCKED (no clean pairs)")
            continue
        med, n, fc, fp, gate = v
        side = "C" if med < 0 else "P"
        print(
            f"{tag}: median={med:+.2f}% n={n} favor_C={fc}/{n} favor_P={fp}/{n}"
            f" side={side} sb_gate={'PASS' if gate else 'FAIL'}"
        )


if __name__ == "__main__":
    main()
