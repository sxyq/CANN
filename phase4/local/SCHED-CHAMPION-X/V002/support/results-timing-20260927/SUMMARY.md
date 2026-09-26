# SCHED-CHAMPION-X V002 Timing — R2-SCHED-V002-TIMING, device 4

SOURCE_SHA: de1e93c74338802e646258ada08a8cac1031496a9985e585a5a89015b81bc990
Protocol: warmup=45, device-event, same-binary 2 blocks x 21 samples + 4 interleaved P/C pairs.

## Same-binary qualification

| shape | binary | B1 | B2 | MAD/med | drift | verdict |
|---|---|---:|---:|---:|---:|---|
| 33x100 FP32 | parent | 8.66 | 8.72 | 0.031 | 0.007 | PASS |
| 33x100 FP32 | cand | 47.78 | 7.62 | 0.622 | 2.191 | FAIL |
| 17x257 FP16 | parent | 6.52 | 20.42 | 0.226 | 1.955 | FAIL |
| 17x257 FP16 | cand | 5.60 | 6.06 | 0.078 | 0.078 | PASS |
| 17x256 FP32 | parent | 6.58 | 7.28 | 0.245 | 0.100 | FAIL |
| 17x256 FP32 | cand | 5.70 | 31.72 | 0.407 | 3.012 | FAIL |
| 7x65 FP32 | parent | 6.14 | 7.04 | 0.178 | 0.134 | FAIL |
| 7x65 FP32 | cand | 5.10 | 27.22 | 0.157 | 3.867 | FAIL |

NO shape has both parent and candidate same-binary PASS. Device heavily contaminated.

## Paired P/C (UNRELIABLE — same-binary failures)

### 33x100 FP32 (rowGroup=2, primary)
p1: P=136.52(MAD=25.62) C=8.12(MAD=0.42) -94.1% OUTLIER
p2: P=8.84(MAD=0.26) C=16.04(MAD=8.00) +81.4% OUTLIER
p3: P=8.00(MAD=0.26) C=7.80(MAD=0.36) -2.5% CLEAN
p4: P=9.02(MAD=0.48) C=46.58(MAD=7.52) +416.4% OUTLIER
Only 1 clean pair. Cannot establish win/loss.

### 17x257 FP16 (rowGroup=16, regression control)
p1: P=5.96 C=5.90 -1.0% CLEAN
p2: P=15.42(MAD=10.18) C=6.40 -58.5% OUTLIER
p3: P=5.50 C=6.26 +13.8% CLEAN
p4: P=6.52 C=7.48 +14.7% CLEAN
Clean pairs: -1.0%, +13.8%, +14.7%. Median ~+13.8%.
KEY: NO V001 +145% regression. Gate preserves parent parallelism.

### 17x256 FP32 (rowGroup=1 control)
p1: P=5.48 C=5.48 +0.0% CLEAN
p2: P=5.50 C=6.30 +14.5% CLEAN
p3: P=7.08 C=6.72 -5.1% CLEAN
p4: P=6.36 C=5.74 -9.7% CLEAN
Median -2.5%. Mixed, near noise.

### 7x65 FP32 (rowGroup=8 edge)
p1: P=4.86 C=5.34 +9.9% CLEAN
p2: P=5.14 C=68.94(MAD=62.28) +1241% OUTLIER
p3: P=5.16 C=6.14 +19.0% CLEAN
p4: P=5.88 C=5.72 -2.7% CLEAN
Clean: +9.9%, +19.0%, -2.7%. Slight regression signal but within noise.

## Verdict

LOCAL_VERDICT = NEEDS_ONE_MORE_LOCAL

Key finding: 17x257 FP16 shows NO parallelism collapse (clean delta ~+14% vs V001 +145%). Gate works as designed. But 33x100 primary shape is too contaminated to measure (cand same-binary FAIL, 3/4 pairs outlier). Need cleaner window.

LOCAL_BEST = NONE
