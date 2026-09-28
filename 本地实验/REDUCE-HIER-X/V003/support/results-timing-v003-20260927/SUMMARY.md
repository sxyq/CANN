# REDUCE-HIER-X V003 Timing — R2-REDUCE-V003-TIMING, device 4

Protocol: warmup=45, device-event primary, samples=41, same-binary 2 blocks + 6 interleaved P/C pairs (order alternates PC/CP).
Harness: runner_ref.inc, rhx_ref_{parent,candidate}_probe rebuilt against V003 SOURCE_SHA 79f910e4. Kernel source unchanged this phase.
Device: d4, AICore 0% pre/post, residual VLLM HBM ~90%.

H3 = FoldToReduceSpan (pairwise-add fold to 128, zero intermediate PipeBarrier) + short-span ReduceSum. vs frozen parent.

## Same-binary qualification (MAD/med <= 0.10 AND drift <= 0.10)

| shape | binary | B1_med | B2_med | MAD/med | drift | verdict |
|---|---|---:|---:|---:|---:|---|
| 1x32768_fp32 | parent | 12.54 | 12.56 | 0.040 | 0.002 | PASS |
| 1x32768_fp32 | cand | 12.56 | 12.42 | 0.040 | 0.011 | PASS |
| 1x16384_fp32 | parent | 11.48 | 7.72 | 0.083 | 0.392 | FAIL |
| 1x16384_fp32 | cand | 8.16 | 7.78 | 0.042 | 0.048 | PASS |
| 8x8192_fp32 | parent | 6.60 | 7.18 | 0.073 | 0.084 | PASS |
| 8x8192_fp32 | cand | 7.02 | 7.34 | 0.053 | 0.045 | PASS |
| 1x4096_fp32 | parent | 5.48 | 4.78 | 0.139 | 0.136 | FAIL |
| 1x4096_fp32 | cand | 6.30 | 9.28 | 0.309 | 0.383 | FAIL |

Qualified (both PASS): 1x32768_fp32, 8x8192_fp32
Blocked: 1x16384_fp32, 1x4096_fp32

## Paired P/C (6 pairs each)

### 1x32768_fp32 — QUALIFIED
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 12.56 | 0.58 | 12.14 | 0.88 | -3.3% | C |  |
| 2 | C→P | 12.20 | 0.66 | 12.42 | 0.64 | +1.8% | P |  |
| 3 | P→C | 12.22 | 0.18 | 12.16 | 0.40 | -0.5% | = |  |
| 4 | C→P | 12.72 | 0.64 | 12.40 | 0.32 | -2.5% | C |  |
| 5 | P→C | 11.76 | 0.34 | 20.80 | 9.38 | +76.9% | P | OUTLIER |
| 6 | C→P | 12.06 | 0.18 | 12.38 | 0.54 | +2.7% | P |  |

Clean pairs: n=5/6, median delta -0.5%, favC=2 favP=2

### 1x16384_fp32 — BLOCKED_SHAPE — UNRELIABLE
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 8.30 | 0.30 | 8.04 | 0.32 | -3.1% | C |  |
| 2 | C→P | 8.06 | 0.42 | 7.84 | 0.14 | -2.7% | C |  |
| 3 | P→C | 11.34 | 4.02 | 8.06 | 0.18 | -28.9% | C | OUTLIER |
| 4 | C→P | 8.08 | 0.22 | 7.96 | 0.16 | -1.5% | C |  |
| 5 | P→C | 8.02 | 0.26 | 7.92 | 0.28 | -1.2% | C |  |
| 6 | C→P | 8.22 | 0.32 | 8.28 | 0.34 | +0.7% | = |  |

Clean pairs: n=5/6, median delta -1.5%, favC=4 favP=0

### 8x8192_fp32 — QUALIFIED
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 7.90 | 2.14 | 7.22 | 0.54 | -8.6% | C | OUTLIER |
| 2 | C→P | 6.70 | 0.64 | 17.40 | 5.30 | +159.7% | P | OUTLIER |
| 3 | P→C | 6.74 | 0.30 | 7.06 | 0.30 | +4.7% | P |  |
| 4 | C→P | 6.80 | 0.24 | 6.56 | 0.28 | -3.5% | C |  |
| 5 | P→C | 7.54 | 0.40 | 7.20 | 0.70 | -4.5% | C |  |
| 6 | C→P | 7.28 | 0.38 | 7.06 | 0.42 | -3.0% | C |  |

Clean pairs: n=4/6, median delta -3.3%, favC=3 favP=1

### 1x4096_fp32 — BLOCKED_SHAPE — UNRELIABLE
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 5.92 | 1.54 | 6.50 | 1.90 | +9.8% | P | OUTLIER |
| 2 | C→P | 7.58 | 2.18 | 5.80 | 1.04 | -23.5% | C | OUTLIER |
| 3 | P→C | 76.92 | 71.12 | 5.74 | 0.76 | -92.5% | C | OUTLIER |
| 4 | C→P | 5.90 | 1.46 | 4.64 | 0.68 | -21.4% | C |  |
| 5 | P→C | 6.44 | 1.26 | 8.14 | 2.28 | +26.4% | P | OUTLIER |
| 6 | C→P | 5.02 | 0.60 | 7.94 | 2.06 | +58.2% | P | OUTLIER |

Clean pairs: n=1/6, median delta -21.4%, favC=1 favP=0

## Interpretation

- **1x32768 FP32 (tileCount=8, PRIMARY):** clean median delta **-0.5%** — essentially ZERO. 5/6 clean pairs split 2C/2P/1neutral. H3's short-span ReduceSum does **not** deliver a measurable win on the primary shape, but it does **not** regress either (unlike V002's +6.6%).
- **8x8192 FP32 (qualified):** clean median delta **-3.3%**, 3/4 clean pairs favor candidate. Small positive signal, near noise floor.
- **1x16384 FP32 (blocked — parent same-binary FAIL):** clean median -1.5%, 4/5 favor candidate. Consistent small-win direction but shape blocked; unreliable.
- **1x4096 FP32 (blocked — both FAIL):** only 1 clean pair. No usable evidence; control inconclusive.
- Contrast with V002: V002 (12 barriers/tile) showed +6.6% regression on 32768. V003 (0 intermediate barriers) is neutral on the same shape. This confirms the barrier chain was the V002 regression source — but eliminating it does not by itself produce a win; the short-span ReduceSum saves little at tileCount=8 where ReduceSum work is already small relative to DMA + output pass.

## Verdict

LOCAL_VERDICT = **NEEDS_ONE_MORE_LOCAL**

Rationale: primary shape (1x32768) is neutral (-0.5%, mixed direction). 8x8192 shows a small clean win (-3.3%) but near noise. 1x16384 blocked despite consistent -1.5% direction. Not LOCAL_ACCEPTED (no clear win beyond noise on the primary shape), not LOCAL_REJECTED (no regression; V003 does not repeat V002's failure).

ONLINE_WORTHY: **NOT YET**. Signal too weak. Recommend one more local round (re-qualify 1x16384 + add pairs on 32768) or accept H3 as neutral and pivot.

Pivot note per Main acceptance: H3 did not LOSE (no regression), so the "abandon reduction-topology" trigger does not fire. But the expected large win also did not materialize. Reduction topology on this parent appears to be a weak lever: two variants (V001 fold, V003 short-span) are both ~neutral, and V002 (tree) regressed. The bottleneck is likely DMA + output pass, not the reduction V/S path.
