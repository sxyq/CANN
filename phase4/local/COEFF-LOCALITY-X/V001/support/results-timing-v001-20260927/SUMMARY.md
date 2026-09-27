# COEFF-LOCALITY-X V001 Timing — R2-COEFF-V001-TIMING, device 4

Protocol: warmup=45, device-event primary, samples=41, same-binary 2 blocks + 6 interleaved P/C pairs (order alternates PC/CP).
Harness: runner_ref.inc, clx_ref_{parent,candidate}_probe rebuilt against V001 SOURCE_SHA 6e47a9a0. Kernel source unchanged this phase.
Device: d4, AICore 0% pre/post, residual VLLM HBM ~90%.

H1 = 2-deep gamma/bias param MTE2 prefetch in ProcessWideFp32FullCacheRows pass 2 (V011 pattern). vs frozen parent.
Note: UB staging cost — D=32768 tileElems 4096→2560 (tileCount 8→13) as a consequence of 2-deep staging. See local-result.json STAGING_RESIDENCE_NOTE.

## Same-binary qualification (MAD/med <= 0.10 AND drift <= 0.10)

| shape | binary | B1_med | B2_med | MAD/med | drift | verdict |
|---|---|---:|---:|---:|---:|---|
| 1x32768_fp32 | parent | 11.90 | 12.90 | 0.042 | 0.081 | PASS |
| 1x32768_fp32 | cand | 21.68 | 86.00 | 0.561 | 1.195 | FAIL |
| 1x16384_fp32 | parent | 8.04 | 8.14 | 0.048 | 0.012 | PASS |
| 1x16384_fp32 | cand | 8.08 | 8.14 | 0.021 | 0.007 | PASS |
| 8x32768_fp32 | parent | 12.96 | 23.32 | 0.066 | 0.571 | FAIL |
| 8x32768_fp32 | cand | 14.92 | 15.22 | 0.039 | 0.020 | PASS |
| 1x4096_fp32 | parent | 5.48 | 7.96 | 0.237 | 0.369 | FAIL |
| 1x4096_fp32 | cand | 7.88 | 50.30 | 0.257 | 1.458 | FAIL |
| 1x8192_fp32 | parent | 6.04 | 6.30 | 0.076 | 0.042 | PASS |
| 1x8192_fp32 | cand | 5.92 | 12.54 | 0.125 | 0.717 | FAIL |

Qualified (both PASS): 1x16384_fp32
Blocked: 1x32768_fp32, 8x32768_fp32, 1x4096_fp32, 1x8192_fp32

## Paired P/C (6 pairs each)

### 1x32768_fp32 — BLOCKED_SHAPE — UNRELIABLE
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 11.96 | 0.28 | 14.36 | 0.46 | +20.1% | P |  |
| 2 | C→P | 11.90 | 0.40 | 16.40 | 3.38 | +37.8% | P |  |
| 3 | P→C | 12.58 | 0.80 | 14.62 | 0.58 | +16.2% | P |  |
| 4 | C→P | 12.46 | 1.08 | 13.84 | 0.28 | +11.1% | P |  |
| 5 | P→C | 12.00 | 0.52 | 14.30 | 0.54 | +19.2% | P |  |
| 6 | C→P | 34.04 | 21.90 | 24.00 | 10.32 | -29.5% | C | OUTLIER |

Clean pairs: n=5/6, median delta +19.2%, favC=0 favP=5

### 1x16384_fp32 — QUALIFIED
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 7.92 | 0.26 | 8.28 | 0.32 | +4.5% | P |  |
| 2 | C→P | 8.26 | 0.30 | 8.12 | 0.62 | -1.7% | C |  |
| 3 | P→C | 8.44 | 0.46 | 8.44 | 0.98 | +0.0% | = |  |
| 4 | C→P | 8.22 | 0.22 | 7.76 | 0.20 | -5.6% | C |  |
| 5 | P→C | 8.32 | 0.28 | 8.62 | 0.92 | +3.6% | P |  |
| 6 | C→P | 8.16 | 0.62 | 8.46 | 0.34 | +3.7% | P |  |

Clean pairs: n=6/6, median delta +1.8%, favC=2 favP=3

### 8x32768_fp32 — BLOCKED_SHAPE — UNRELIABLE
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 12.78 | 0.18 | 14.34 | 0.48 | +12.2% | P |  |
| 2 | C→P | 17.34 | 4.70 | 14.40 | 0.26 | -17.0% | C | OUTLIER |
| 3 | P→C | 22.06 | 10.32 | 14.84 | 0.36 | -32.7% | C | OUTLIER |
| 4 | C→P | 12.64 | 0.26 | 18.64 | 4.44 | +47.5% | P |  |
| 5 | P→C | 23.96 | 11.80 | 14.10 | 0.88 | -41.2% | C | OUTLIER |
| 6 | C→P | 12.88 | 0.38 | 14.08 | 0.36 | +9.3% | P |  |

Clean pairs: n=3/6, median delta +12.2%, favC=0 favP=3

### 1x4096_fp32 — BLOCKED_SHAPE — UNRELIABLE
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 5.66 | 0.98 | 5.72 | 1.24 | +1.1% | P |  |
| 2 | C→P | 12.90 | 7.96 | 5.52 | 0.66 | -57.2% | C | OUTLIER |
| 3 | P→C | 5.64 | 0.84 | 5.64 | 1.50 | +0.0% | = | OUTLIER |
| 4 | C→P | 7.14 | 1.80 | 5.62 | 0.42 | -21.3% | C | OUTLIER |
| 5 | P→C | 5.32 | 0.76 | 95.50 | 64.22 | +1695.1% | P | OUTLIER |
| 6 | C→P | 6.06 | 0.84 | 5.36 | 0.80 | -11.6% | C |  |

Clean pairs: n=2/6, median delta -5.2%, favC=1 favP=1

### 1x8192_fp32 — BLOCKED_SHAPE — UNRELIABLE
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 6.24 | 0.44 | 5.82 | 0.30 | -6.7% | C |  |
| 2 | C→P | 7.04 | 1.16 | 6.08 | 0.40 | -13.6% | C |  |
| 3 | P→C | 6.10 | 0.34 | 6.30 | 0.54 | +3.3% | P |  |
| 4 | C→P | 6.04 | 0.16 | 6.28 | 0.88 | +4.0% | P |  |
| 5 | P→C | 15.22 | 6.10 | 6.52 | 0.56 | -57.2% | C | OUTLIER |
| 6 | C→P | 6.30 | 0.54 | 6.00 | 0.44 | -4.8% | C |  |

Clean pairs: n=5/6, median delta -4.8%, favC=3 favP=2

## Interpretation

- **1x32768 FP32 (PRIMARY):** clean median delta **+19.2% (candidate SLOWER)**, 5/5 clean pairs favor parent, MADs small (0.28-1.08 P, 0.28-0.58 C). This is a clear, consistent regression — not noise. Candidate same-binary FAIL (MAD/med 0.561, drift 1.195) reflects instability from the same cause. **The UB-staging cost (tileElems 4096→2560, tileCount 8→13) outweighs the prefetch benefit at this shape.**
- **1x16384 FP32 (only QUALIFIED):** clean median delta **+1.8%**, 3P/2C/1neutral — noise-level, slight regression direction. tileElems unchanged (4096) at this shape, so this is a clean read: prefetch alone is ~0 here.
- **8x32768 FP32 (blocked):** 3 clean pairs median +12.2% favor parent, 3 outlier pairs. Same regression direction as 1x32768 (tileElems shrinks at D=32768).
- **1x4096 FP32 control (blocked):** 2 clean pairs −5.2% mixed; expect ≈0, inconclusive.
- **1x8192 FP32 (blocked):** 5 clean pairs median −4.8% (3C/2P) — small candidate-favoring signal at mid D where tileElems is unchanged. Not reliable (candidate same-binary FAIL).

**Falsification note:** Main's criterion was "|Δ|<=2% on 32768 → param MTE2 not the bottleneck". We observed +19.2% instead — a regression. This is NOT clean falsification of the prefetch mechanism because the tileElems confound is present at D=32768. What IS established: **as delivered, V001 regresses its primary shape.**

## Verdict

LOCAL_VERDICT = **LOCAL_REJECTED**

Rationale: primary shape 1x32768 FP32 shows a clean, consistent regression (+19.2%, 5/5 favor parent, small MADs). The delivered revision is slower than the frozen parent on its own primary target. The only qualified shape (1x16384) is noise-level (+1.8%). Per Main's acceptance ("Clean win on 32768/16384 beyond noise → strong ONLINE_WORTHY"), we have the opposite.

ONLINE_WORTHY: **NO**.

**Confound for Main's next decision:** at D=32768 the 2-deep staging forces tileElems 4096→2560 (UB budget), which alone likely causes the regression (13 vs 8 tiles = more pass-1 ReduceSum calls, more store drains, more loop overhead). The prefetch mechanism itself is NOT cleanly falsified — it was never measured at constant tileElems. Two salvage options, both new revisions:
- (a) Hold tileElems=4096 and free UB another way (e.g. reduce wideFullYRows_ — but it is already 1 at D=32768; would need to steal from y or reduceBytes). 
- (b) Abandon param-prefetch on this parent and pivot (Main's stated fallback if large-D loses).

Given V001 as delivered LOSES and the REDUCE-HIER-X series also failed to win on large-D, the evidence now points to: neither reduction topology nor param-MTE2 prefetch (at this UB budget) moves large-D. The remaining lever per the load map is H2 (cross-batch stripe residency) or a different axis entirely.
