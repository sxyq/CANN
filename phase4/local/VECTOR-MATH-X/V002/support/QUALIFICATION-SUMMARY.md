# VECTOR-MATH-X V002 OVERNIGHT QUALIFICATION SUMMARY

DATE: 2026-09-27 (overnight, C2C)
ROUTE: VECTOR-MATH-X / V002 (Duplicate broadcast + vector Mul)
SOURCE_SHA: 06095762d6ff919f7aa260168150c23e0eeecf464a87c1b26e513cb267587427 (FROZEN, no edits)
PARENT_EXE_SHA: efff9b99…; CANDIDATE_EXE_SHA: ed3786d5… (unchanged from prior session)

## Budget: 3 serious attempts across ≥2 devices — ALL USED

| attempt | device | method | 4×2048 | 8×1024 | 8×256 | 1×32768 |
|---|---|---|---|---|---|---|
| 1 | d6 | blocks=2, s=41 | FAIL (MAD/med=0.395, drift=17.0) | FAIL (0.139/0.121) | FAIL (0.129/0.042) | **PASS** (0.031/0.023) |
| 1-cal | d6 | s=81 | FAIL (0.153/0.193) | FAIL (0.220/0.158) | — | — |
| 1-cal | d6 | blocks=3 | FAIL (0.190/4.16) | FAIL (0.330/2.48) | — | — |
| 2 | d5 | blocks=2, s=41 | FAIL (0.210/0.149) | FAIL (0.210/0.083) | FAIL (0.190/0.004) | **PASS** (0.019/0.017) |
| 2-cal | d5 | s=81 | FAIL (0.198/1.53) | FAIL (**0.104**/0.007) | FAIL (**0.104**/0.067) | — |
| 2-cal | d5 | batch_n=4 | — | FAIL (0.618/0.431) | FAIL (0.320/0.607) | — |
| 3 | d4 | blocks=2, s=81 | FAIL (0.227/0.082) | FAIL (0.152/0.177) | FAIL (0.158/0.055) | **PASS** (0.052/0.063) |

## Key observations

1. **1×32768 FP32 (12 µs) passes same-binary on all 3 devices** (MAD/med 0.019–0.052, drift 0.017–0.063).
2. **4×2048, 8×1024, 8×256 (5–6 µs) consistently fail MAD/med ≤ 0.10** across d4/d5/d6 and all calibration modes. Best observed: MAD/med=0.104 (d5, s=81) — 4% over threshold.
3. Sparse outliers inflate MAD/med. Full-distribution MAD counts outliers per protocol; trimmed CV cannot override.
4. **Repeat-batch (batch_n=4) REJECTED** — materially worse MAD/med (0.32–0.62).
5. Larger samples (81) and blocks (3) do not fix the noise — inherent to 5–6 µs kernels under VLLM host interference.

## Paired data (from prior session, diagnostic only — same-binary not qualified)

| shape | clean-pair Δ% | median | direction |
|---|---|---:|---|
| 4×2048 FP32 | −6.5, −22.4, −14.0, −2.4, −14.7 (5/5) | **−14.0%** | favor C |
| 8×1024 FP32 | −9.2, −8.8, −23.1 (3/3) | **−9.2%** | favor C |
| 1×32768 FP32 | −1.5, −2.3, +0.2, +4.3, −2.5 | −1.5% | noise |

Medium-shape P/C gains are outside ±5% noise band with consistent direction, but **not formally admissible** because same-binary does not qualify.

## DISPOSITION

**MEASUREMENT_BLOCKED** for medium shapes (4×2048, 8×1024) after 3 serious independent attempts (d6, d5, d4) with authorized calibration (s=81, blocks=3, batch_n=4). Infrastructure noise on 5–6 µs kernels prevents MAD/med ≤ 0.10 qualification.

**1×32768**: same-binary PASS on all devices, but P/C delta within noise (−1.5% median). Not a win.

**LOCAL_VERDICT = MEASUREMENT_BLOCKED**
**ONLINE_WORTHY = NO** (same-binary PASS required for medium shapes — not met)

The paired signal (−9% to −14%, 5/5 and 3/3 direction, no medium regression) is the strongest observed on this route, but protocol does not permit LOCAL_ACCEPTED without same-binary qualification. Recommend Main consider whether the noise floor criterion should be adjusted for the 5–6 µs regime, or whether a different measurement layer (e.g. wider timing window) is needed.

## Evidence
- `qa1-d6/samebinary.csv`, `qa1-d6/samebinary-cal81.csv`, `qa1-d6/` raw
- `qa2-d5/samebinary.csv`, `qa2-d5/samebinary-cal81.csv`, `qa2-d5/` raw
- `qa3-d4/samebinary.csv`, `qa3-d4/` raw
- `results-timing-v002/paired-deltas.csv` (prior session P/C)
