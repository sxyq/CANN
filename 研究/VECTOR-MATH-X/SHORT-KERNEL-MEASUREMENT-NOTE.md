# SHORT-KERNEL MEASUREMENT NOTE — VECTOR-MATH-X V002

DATE: 2026-09-27
CONTEXT: C2C overnight qualification, VECTOR-MATH-X V002 (SOURCE_SHA 06095762…).

## Finding

**5–6 µs kernels do not meet the same-binary MAD/median ≤ 0.10 threshold** on devices d4, d5, d6. Observed MAD/med 0.10–0.39 across 3 serious independent attempts, all calibration modes, and both block orders. Sparse host-interference outliers (VLLM resident, HBM ~60/65 GB) inflate full-distribution MAD regardless of sample count.

| kernel length | best MAD/med | threshold | devices |
|---|---:|---|---|
| 5–6 µs (4×2048, 8×1024, 8×256) | 0.104 | FAIL | d4/d5/d6 |
| 12 µs (1×32768) | 0.019 | PASS | d4/d5/d6 |

## Calibration attempts (all on PARENT same-binary first)

| method | result |
|---|---|
| samples=81 | MAD/med 0.104–0.220 — no material improvement |
| blocks=3 | outlier blocks worsen drift (up to 4.16) |
| batch_n=4 repeat-batch | **REJECTED** — MAD/med 0.32–0.62 (worse) |

## Recommendation for Main policy

1. **Length-stratified same-binary thresholds**: e.g. MAD/med ≤ 0.25 for kernels < 10 µs, ≤ 0.10 for ≥ 10 µs. The current flat ≤ 0.10 threshold is unachievable in the 5–6 µs regime under shared-VLLM load.
2. **Alternative measurement layer**: per-shape noise floor via trimmed-MAD (protocol already allows as secondary) or interquartile-based statistic, promoted to primary for short kernels only.
3. **Do not use repeat-batch** for short kernels on this hardware — it worsens noise.
