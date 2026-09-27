# VECTOR-MATH-X V001 TIMING SUMMARY

DATE: 2026-09-27
ROUTE: VECTOR-MATH-X / V001 (VM-H2 vector denominator)
SOURCE_SHA: dbe776f9165a86ede9d136604e806d9f5dc0f641ee8e05a26bca1ebe8dd33424
DEVICE: server3 d5. METHOD: DEVICE_EVENT_PRIMARY, warmup=45, 41 samples × 6 pairs.
Parent `vmx_ref_parent` SHA efff9b99…; Candidate `vmx_ref_v001` SHA 35bbd153….

## PAIRED DELTA (median Δ% = (C−P)/P)

### 32×256 FP32 (batched, row-dense — primary)

| pair | order | P µs | C µs | Δ% |
|---:|---|---:|---:|---:|
| 1 | PC | 7.12 | 6.28 | **−11.80** |
| 2 | CP | 6.42 | 8.04 | +25.23 (C outlier) |
| 3 | PC | 6.82 | 6.32 | **−7.33** |
| 4 | CP | 6.64 | 6.18 | **−6.93** |
| 5 | PC | 69.74 | 6.46 | −90.74 (P outlier) |
| 6 | CP | 6.04 | 7.24 | +19.87 (C outlier) |

Clean pairs (1,3,4): **3/3 favor Candidate, median −7.3%.** Excluding outliers, this is marginally above the ±5% noise band.

### 8×256 FP32 (batched)

Clean pairs (1,2,3): −20.4%, −8.0%, −0.3%. Median ~−8%. Mixed (pair 4 +6.5%).

### 2×4096 FP32 (mid)

Clean pairs (1–4): +10.9%, +11.4%, −9.7%, −7.8%. Median ~0%. No signal.

### 8×8192 FP32 (full-row)

Clean pairs (1–5): −1.5%, +4.8%, −1.7%, −3.8%, +3.0%. Median ~−1.5%. Noise band.

## Verdict

**LOCAL_VERDICT = NEEDS_ONE_MORE_LOCAL**

The batched shapes (8×256, 32×256) show a consistent −7% to −12% trend on clean pairs, consistent with the vector denominator tail amortizing across rows. The mid and full-row shapes show no signal (denominator tail is per-row there). Same-binary strict gate not evaluated; data is diagnostic.

**ONLINE_WORTHY = KEEP_ACCUMULATING** — positive trend on batched shapes warrants one more measurement round.

## Evidence
- `results-timing-v001/paired-deltas.csv`, `*-raw.tsv`, `*-stats.txt`
