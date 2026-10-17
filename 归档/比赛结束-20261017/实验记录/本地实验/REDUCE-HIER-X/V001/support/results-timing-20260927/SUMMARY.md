# REDUCE-HIER-X V001 Timing — R2-REDUCE-V001-TIMING, device 5

Protocol: warmup=45, device-event primary (aclrtRecordEvent), samples=21, same-binary 2 blocks + 4 interleaved P/C pairs (order alternates PC/CP).
Harness: runner_ref.inc (SCHED reference), rhx_ref_{parent,candidate}_probe. Kernel source unchanged this phase.
Device: d5, AICore 0% pre/post, residual VLLM HBM ~91%. npu-smi pre/post retained.

## Same-binary qualification (MAD/med <= 0.10 AND drift <= 0.10)

| shape | binary | B1_med | B2_med | MAD/med | drift | verdict |
|---|---|---:|---:|---:|---:|---|
| 1x8192_fp32 | parent | 14.12 | 5.62 | 0.109 | 0.861 | FAIL |
| 1x8192_fp32 | cand | 5.98 | 5.84 | 0.041 | 0.024 | PASS |
| 1x4096_fp32 | parent | 7.28 | 6.14 | 0.125 | 0.170 | FAIL |
| 1x4096_fp32 | cand | 5.08 | 5.20 | 0.157 | 0.023 | FAIL |
| 1x2048_fp32 | parent | 6.22 | 3.84 | 0.184 | 0.473 | FAIL |
| 1x2048_fp32 | cand | 5.52 | 7.20 | 0.204 | 0.264 | FAIL |
| 2x256_fp32 | parent | 5.50 | 8.06 | 0.166 | 0.378 | FAIL |
| 2x256_fp32 | cand | 7.20 | 11.40 | 0.420 | 0.452 | FAIL |
| 8x8192_fp32 | parent | 6.42 | 6.92 | 0.048 | 0.075 | PASS |
| 8x8192_fp32 | cand | 6.98 | 7.42 | 0.056 | 0.061 | PASS |

Qualified shapes (both PASS): 8x8192_fp32
Blocked shapes: 1x8192_fp32, 1x4096_fp32, 1x2048_fp32, 2x256_fp32

## Paired P/C (4 pairs each)

### 1x8192_fp32 — BLOCKED_SHAPE (same-binary FAIL) — UNRELIABLE
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 6.26 | 0.26 | 7.54 | 2.08 | +20.4% | P | OUTLIER |
| 2 | C→P | 6.78 | 0.72 | 7.56 | 1.46 | +11.5% | P |  |
| 3 | P→C | 5.90 | 0.42 | 5.82 | 0.14 | -1.4% | C |  |
| 4 | C→P | 5.98 | 0.32 | 5.94 | 0.18 | -0.7% | = |  |

Clean pairs: n=3/4, median delta -0.7%, favC=1 favP=1

### 1x4096_fp32 — BLOCKED_SHAPE (same-binary FAIL) — UNRELIABLE
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 5.72 | 1.64 | 6.76 | 0.94 | +18.2% | P | OUTLIER |
| 2 | C→P | 4.84 | 0.72 | 5.48 | 0.84 | +13.2% | P |  |
| 3 | P→C | 7.12 | 0.44 | 5.28 | 0.84 | -25.8% | C |  |
| 4 | C→P | 4.72 | 0.40 | 4.82 | 0.44 | +2.1% | P |  |

Clean pairs: n=3/4, median delta +2.1%, favC=1 favP=2

### 1x2048_fp32 — BLOCKED_SHAPE (same-binary FAIL) — UNRELIABLE
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 6.10 | 0.82 | 6.36 | 0.44 | +4.3% | P |  |
| 2 | C→P | 6.26 | 0.80 | 8.46 | 1.56 | +35.1% | P |  |
| 3 | P→C | 6.70 | 1.28 | 5.52 | 0.86 | -17.6% | C |  |
| 4 | C→P | 5.46 | 1.72 | 6.32 | 0.60 | +15.8% | P | OUTLIER |

Clean pairs: n=3/4, median delta +4.3%, favC=1 favP=2

### 2x256_fp32 — BLOCKED_SHAPE (same-binary FAIL) — UNRELIABLE
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 5.96 | 0.70 | 5.82 | 1.08 | -2.3% | C |  |
| 2 | C→P | 14.12 | 9.62 | 5.52 | 0.36 | -60.9% | C | OUTLIER |
| 3 | P→C | 8.46 | 2.96 | 7.46 | 0.64 | -11.8% | C | OUTLIER |
| 4 | C→P | 6.44 | 0.52 | 5.10 | 0.96 | -20.8% | C |  |

Clean pairs: n=2/4, median delta -11.6%, favC=2 favP=0

### 8x8192_fp32 — QUALIFIED
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 6.72 | 0.24 | 6.76 | 0.34 | +0.6% | = |  |
| 2 | C→P | 7.04 | 0.30 | 6.46 | 0.20 | -8.2% | C |  |
| 3 | P→C | 6.18 | 0.24 | 81.06 | 74.36 | +1211.7% | P | OUTLIER |
| 4 | C→P | 6.92 | 0.46 | 19.00 | 13.00 | +174.6% | P | OUTLIER |

Clean pairs: n=2/4, median delta -3.8%, favC=1 favP=0

## Diagnostic: fast-cluster (device_us <= 10) counts (SECONDARY, not for decisions)

| shape | binary | n | fast | slow>50 |
|---|---|---:|---:|---:|
| 8x8192_fp32 | P | 42 | 36 | 5 |
| 8x8192_fp32 | C | 42 | 31 | 10 |

## Notes

- d5 shows residual VLLM interference: samples are bimodal (fast cluster ~6-7 us vs slow spikes 50-550 us) on BOTH parent and candidate. Medians land in the fast cluster; MAD and drift absorb spikes.
- 1x8192 parent same-binary FAIL is driven by B1=14.12 vs B2=5.62 (drift 0.861) — first-block settling outlier, not a source defect.
- 4/5 shapes fail same-binary on at least one binary under this window. Only 8x8192 FP32 qualifies.
- On 8x8192 clean pairs the candidate is slightly faster (median -3.8%) but two pairs show extreme candidate spikes (device contention), so the direction is not stable.

## Verdict

LOCAL_VERDICT = NEEDS_ONE_MORE_LOCAL

Rationale: only 1/5 shapes (8x8192 FP32) has valid same-binary on both binaries. Clean paired deltas there are small and mixed (median -3.8% favor C on n=2 clean pairs, plus two contention-outlier pairs). Shapes 1x8192/1x4096/1x2048/2x256 are MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE (same-binary FAIL) — not evidence for or against H1. No LOCAL_ACCEPTED and no LOCAL_REJECTED.

Recommended next action: re-run same-binary + P/C on 8x8192 FP32 in a quieter window (or more pairs) to confirm the small negative delta; if the fold is real it should show most clearly at tileCount>=2 shapes.
