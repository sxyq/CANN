# SCHED-CHAMPION-X V001 Timing — R2-SCHED-V001-TIMING, device 4

Protocol: warmup=45, device-event primary, samples=21, same-binary 2 blocks + 4 interleaved P/C pairs.

## Same-binary qualification (MAD/med <= 0.10 AND drift <= 0.10)

| shape | binary | B1_med | B2_med | MAD/med | drift | verdict |
|---|---|---:|---:|---:|---:|---|
| 33x100 FP32 | parent | 8.48 | 8.72 | 0.035 | 0.028 | PASS |
| 33x100 FP32 | cand | 7.62 | 8.34 | 0.093 | 0.088 | PASS |
| 17x256 FP32 | parent | 38.40 | 7.26 | 0.377 | 3.878 | FAIL |
| 17x256 FP32 | cand | 8.12 | 7.58 | 0.301 | 0.069 | FAIL |
| 17x257 FP16 | parent | 6.36 | 9.26 | 0.347 | 0.329 | FAIL |
| 17x257 FP16 | cand | 14.36 | 14.88 | 0.033 | 0.035 | PASS |
| 7x65 FP32 | parent | 6.02 | 6.82 | 0.207 | 0.119 | FAIL |
| 7x65 FP32 | cand | 7.94 | 7.98 | 0.074 | 0.005 | PASS |

Only 33x100 FP32 qualifies (both PASS). Others: MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE.

## Paired P/C on qualified shape: 33x100 FP32 (rowGroup=2)

| pair | P_med | P_MAD | C_med | C_MAD | delta | fav | note |
|---|---:|---:|---:|---:|---:|---|---|
| 1 | 94.14 | 84.96 | 7.62 | 0.22 | -91.9pct | C | OUTLIER |
| 2 | 9.18 | 0.26 | 8.08 | 0.54 | -12.0pct | C | clean |
| 3 | 8.84 | 1.72 | 8.06 | 0.38 | -8.8pct | C | clean |
| 4 | 8.10 | 0.10 | 8.10 | 0.90 | +0.0pct | = | clean |

Clean pairs: median delta = -8.8pct, favor C 2/3, neutral 1/3.

## Paired P/C on blocked shapes (UNRELIABLE)

### 17x256 FP32 (rowGroup=1 control)
p1 -5.6pct C, p2 +6.0pct P, p3 +30.2pct P, p4 +24.1pct P. Median +15.0pct favor P 3/4. Parent sb FAIL (drift 3.878).

### 17x257 FP16 (rowGroup=16)
p1 +150.0pct P, p2 +52.7pct P, p3 +141.8pct P, p4 +159.7pct P. Median +145.9pct favor P 4/4.
Clear regression: rowGroup=16 on 17 rows gives totalGroups=2, V001 uses 2 cores vs parent 17 cores.

### 7x65 FP32 (rowGroup=8 edge)
p1 -60.7pct C (OUTLIER), p2 +17.5pct P, p3 -79.1pct C (OUTLIER), p4 +6.8pct P.
Clean pairs favor P. Parent has severe outliers. Parent sb FAIL.

## Verdict

LOCAL_VERDICT = NEEDS_ONE_MORE_LOCAL

Rationale: Only 33x100 FP32 has valid same-binary. Clean delta -8.8pct favors V001 but one pair is neutral and cand MAD/med (0.093) is near threshold. 17x257 FP16 shows consistent 4/4 regression (+145.9pct) from parallelism collapse (rowGroup=16 -> 2 cores vs 17). Two shapes blocked by parent same-binary failure. Mixed and incomplete.

LOCAL_BEST = NONE
