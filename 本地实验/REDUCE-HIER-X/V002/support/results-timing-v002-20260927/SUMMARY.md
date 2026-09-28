# REDUCE-HIER-X V002 Timing — R2-REDUCE-V002-TIMING, device 4

Protocol: warmup=45, device-event primary, samples=41, same-binary 2 blocks + 6 interleaved P/C pairs (order alternates PC/CP).
Harness: runner_ref.inc, rhx_ref_{parent,candidate}_probe rebuilt against V002 SOURCE_SHA e4a80f18. Kernel source unchanged this phase.
Device: d4, AICore 0% pre/post, residual VLLM HBM ~90%. npu-smi pre/post retained.

## Same-binary qualification (MAD/med <= 0.10 AND drift <= 0.10)

| shape | binary | B1_med | B2_med | MAD/med | drift | verdict |
|---|---|---:|---:|---:|---:|---|
| 1x32768_fp32 | parent | 12.20 | 11.98 | 0.021 | 0.018 | PASS |
| 1x32768_fp32 | cand | 12.24 | 13.14 | 0.081 | 0.071 | PASS |
| 1x16384_fp32 | parent | 8.16 | 9.18 | 0.056 | 0.118 | FAIL |
| 1x16384_fp32 | cand | 8.38 | 8.80 | 0.064 | 0.049 | PASS |
| 1x8192_fp32 | parent | 5.74 | 6.44 | 0.085 | 0.115 | FAIL |
| 1x8192_fp32 | cand | 5.74 | 8.90 | 0.124 | 0.432 | FAIL |
| 1x4096_fp32 | parent | 5.62 | 36.06 | 0.275 | 1.461 | FAIL |
| 1x4096_fp32 | cand | 6.86 | 7.12 | 0.303 | 0.037 | FAIL |
| 8x8192_fp32 | parent | 6.96 | 7.26 | 0.073 | 0.042 | PASS |
| 8x8192_fp32 | cand | 7.20 | 6.94 | 0.062 | 0.037 | PASS |

Qualified (both PASS): 1x32768_fp32, 8x8192_fp32
Blocked: 1x16384_fp32, 1x8192_fp32, 1x4096_fp32

## Paired P/C (6 pairs each)

### 1x32768_fp32 — QUALIFIED
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 12.16 | 0.24 | 12.96 | 0.82 | +6.6% | P |  |
| 2 | C→P | 11.96 | 0.30 | 12.68 | 0.24 | +6.0% | P |  |
| 3 | P→C | 12.04 | 0.16 | 15.60 | 3.60 | +29.6% | P |  |
| 4 | C→P | 12.14 | 0.80 | 12.78 | 0.30 | +5.3% | P |  |
| 5 | P→C | 11.80 | 0.42 | 12.78 | 0.42 | +8.3% | P |  |
| 6 | C→P | 11.94 | 0.16 | 43.84 | 15.02 | +267.2% | P | OUTLIER |

Clean pairs: n=5/6, median delta +6.6%, favC=0 favP=5

### 1x16384_fp32 — BLOCKED_SHAPE — UNRELIABLE
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 17.34 | 8.52 | 8.78 | 1.28 | -49.4% | C | OUTLIER |
| 2 | C→P | 8.20 | 0.36 | 8.72 | 0.46 | +6.3% | P |  |
| 3 | P→C | 8.50 | 0.34 | 8.40 | 0.40 | -1.2% | C |  |
| 4 | C→P | 8.06 | 0.28 | 7.84 | 0.34 | -2.7% | C |  |
| 5 | P→C | 8.28 | 0.34 | 8.42 | 0.44 | +1.7% | P |  |
| 6 | C→P | 8.26 | 0.24 | 8.54 | 0.72 | +3.4% | P |  |

Clean pairs: n=5/6, median delta +1.7%, favC=2 favP=3

### 1x8192_fp32 — BLOCKED_SHAPE — UNRELIABLE
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 5.76 | 0.22 | 6.20 | 0.16 | +7.6% | P |  |
| 2 | C→P | 17.88 | 5.26 | 6.28 | 0.66 | -64.9% | C | OUTLIER |
| 3 | P→C | 6.04 | 0.22 | 5.80 | 0.32 | -4.0% | C |  |
| 4 | C→P | 6.66 | 0.46 | 6.44 | 0.82 | -3.3% | C |  |
| 5 | P→C | 8.60 | 3.66 | 7.88 | 0.84 | -8.4% | C | OUTLIER |
| 6 | C→P | 7.32 | 1.60 | 5.88 | 0.14 | -19.7% | C |  |

Clean pairs: n=4/6, median delta -3.6%, favC=3 favP=1

### 1x4096_fp32 — BLOCKED_SHAPE — UNRELIABLE
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 6.38 | 0.90 | 6.46 | 0.78 | +1.3% | P |  |
| 2 | C→P | 16.34 | 10.02 | 6.66 | 1.48 | -59.2% | C | OUTLIER |
| 3 | P→C | 4.86 | 0.88 | 5.12 | 0.94 | +5.3% | P |  |
| 4 | C→P | 5.60 | 0.70 | 6.30 | 1.64 | +12.5% | P | OUTLIER |
| 5 | P→C | 4.44 | 0.26 | 5.78 | 0.74 | +30.2% | P |  |
| 6 | C→P | 5.10 | 0.62 | 13.34 | 8.52 | +161.6% | P | OUTLIER |

Clean pairs: n=3/6, median delta +5.3%, favC=0 favP=3

### 8x8192_fp32 — QUALIFIED
| pair | order | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---|---:|---:|---:|---:|---:|---|---|
| 1 | P→C | 6.80 | 0.34 | 6.90 | 0.56 | +1.5% | P |  |
| 2 | C→P | 6.42 | 0.26 | 7.16 | 0.24 | +11.5% | P |  |
| 3 | P→C | 7.00 | 0.22 | 6.88 | 0.30 | -1.7% | C |  |
| 4 | C→P | 6.70 | 0.18 | 6.98 | 0.24 | +4.2% | P |  |
| 5 | P→C | 6.70 | 0.32 | 6.72 | 0.14 | +0.3% | = |  |
| 6 | C→P | 19.54 | 3.40 | 7.36 | 0.70 | -62.3% | C |  |

Clean pairs: n=6/6, median delta +0.9%, favC=2 favP=3

## Interpretation

- **1x32768 FP32 (tileCount=8, primary target):** clean median delta **+6.6%** (candidate SLOWER), 5/5 clean pairs favor parent. Small MADs on both sides — this is a real regression, not noise. The H2 win estimate (-18..-35%) is **refuted**.
- **8x8192 FP32 (qualified):** median +0.9%, mixed direction (favP=3 favC=2). Within noise / neutral-to-slightly-regressed.
- Blocked shapes (16384/8192/4096) cannot be used as evidence for or against H2.
- Mechanism note: `VectorReduceTo8` issues log2(tile)=12 pairwise `Add` levels per 4096-elem tile, each followed by `PipeBarrier<PIPE_V>`. That barrier chain costs more than the `ReduceSum` internal V/S handoff it removes. The 8-lane accumulator + one collapse `ReduceSum` per row is not enough to offset it.

## Verdict

LOCAL_VERDICT = **LOCAL_REJECTED**

Rationale: the only qualified large-D shape (1x32768 FP32, tileCount=8) shows a clean, consistent regression (+6.6% median, 5/5 favor parent, MADs well under threshold). The primary hypothesis — that eliminating per-tile `ReduceSum` V/S would yield a large win — is refuted on this toolchain. 8x8192 is neutral-to-slightly-regressed. No shape shows a clean win.

ONLINE_WORTHY recommendation: **NO**. Do not submit V002.

Per execution-contract §M: LOCAL_REJECTED returns the next independent Revision to the preceding Local Best (V001 / frozen parent). Per Main rules: roll back to frozen parent; do not stack another performance change on V002.
