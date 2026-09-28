# VECTOR-MATH-X V002 TIMING SUMMARY

DATE: 2026-09-27
ROUTE: VECTOR-MATH-X / V002 (Duplicate broadcast + vector Mul)
SOURCE_SHA: 06095762d6ff919f7aa260168150c23e0eeecf464a87c1b26e513cb267587427
DEVICE: server3 d4 (lease R2-VM-V002-TIMING). METHOD: DEVICE_EVENT_PRIMARY, warmup=45, 41 samples × 6 pairs.
Parent `vmx_ref_parent` SHA efff9b99…; Candidate `vmx_ref_v002` SHA ed3786d5….

## Same-binary

All 11 shapes FAIL the strict gate (B2 missing — same-binary ran with blocks=1 instead of 2). MAD/med ranges 0.014–0.70. Data is diagnostic; P/C pairs are the primary evidence.

## PAIRED_DELTA by shape class

### MEDIUM (the regressed Official band — primary question)

| shape | clean-pair Δ% | median | direction |
|---|---|---:|---|
| 2×4096 FP32 | −7.1, −5.3 (2 clean; pair1 C outlier +350%) | **−6.2%** | favors C |
| 4×2048 FP32 | −6.5, −22.4, −14.0, −2.4, −14.7 (5/5) | **−14.0%** | **favors C, 5/5** |
| 8×1024 FP32 | −9.2, −8.8, −23.1 (3/3) | **−9.2%** | **favors C, 3/3** |

**No medium-shape regression.** V002 improves the medium band (Candidate faster), opposite of the INTEGRATION-X regression pattern.

### SMALL

| shape | clean-pair Δ% | median | direction |
|---|---|---:|---|
| 8×256 FP32 | −1.2, −6.4, −4.1, −4.3 (4/4) | −4.2% | favors C |
| 32×256 FP32 | +4.3, −3.4, +0.9, 0.0, −6.9, +11.3 | +0.9% | mixed |
| 2×100 FP32 | noisy (P outliers) | — | no signal |
| FP16 8×256 | noisy (C outliers) | — | no signal |
| BF16 8×256 | noisy | — | no signal |

### LARGE

| shape | clean-pair Δ% | median | direction |
|---|---|---:|---|
| 8×8192 FP32 | −6.4, +1.8, 0.0, +4.3 | −2.3% | mixed |
| 2×8192 FP32 | −15.5, −13.6 (2 clean) | −14.5% | favors C |
| 1×32768 FP32 | −1.5, −2.3, +0.2, +4.3, −2.5 | −1.5% | mixed |

## SHAPE_DIRECTION

| class | direction | confidence |
|---|---|---|
| SMALL | slight favor C (8×256 −4.2%) | low (noise) |
| **MEDIUM** | **favor C (−6% to −14%)** | **medium (4×2048 5/5, 8×1024 3/3)** |
| LARGE | mixed to slight favor C | low (noise) |

## Verdict

**LOCAL_VERDICT = NEEDS_ONE_MORE_LOCAL**

Medium shapes (the Primary question) show clear favor for Candidate — **no regression**. 4×2048 FP32 has 5/5 clean pairs at −14.0% median. This is above the ±5% noise band. The same-binary gate failed (B2 missing), so data is diagnostic.

**ONLINE_WORTHY = KEEP_ACCUMULATING** — medium-shape signal is strong enough to warrant one more round with proper same-binary qualification.

**BLOCKER CHECK:** No medium shape shows regression. V002 does NOT repeat the INTEGRATION-X medium-shape regressions.

## Evidence
- `results-timing-v002/samebinary-verdict.csv`, `paired-deltas.csv`, `*-raw.tsv`, `*-stats.txt`
