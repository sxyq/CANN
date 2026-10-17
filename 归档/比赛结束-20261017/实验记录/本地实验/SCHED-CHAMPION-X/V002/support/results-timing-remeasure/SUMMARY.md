# SCHED-CHAMPION-X V002 Remeasure — R2-SCHED-V002-TIMING, device 4

SOURCE_SHA: de1e93c74338802e646258ada08a8cac1031496a9985e585a5a89015b81bc990
Protocol: warmup=45, samples=41, same-binary 2 blocks, 6 interleaved P/C pairs (33x100), 4 pairs (17x257).

## 33x100 FP32 (primary, rowGroup=2, group split active)

### Same-binary
parent: B1=8.54 B2=8.94 med=8.74 mad/med=0.039 drift=0.046 PASS
cand:   B1=32.10 B2=42.44 med=40.12 mad/med=0.202 drift=0.258 FAIL

### Paired P/C (6 pairs)
p1: P=8.58 C=8.20  -4.4% CLEAN
p2: P=8.76 C=7.78  -11.2% CLEAN
p3: P=8.20 C=8.08  -1.5% CLEAN
p4: P=8.24 C=7.88  -4.4% CLEAN
p5: P=8.50 C=7.92  -6.8% CLEAN
p6: P=8.36 C=62.86 +651.9% OUTLIER (C MAD=25.36)

Clean pairs: 5/6, ALL favor C (5/5).
Clean median delta: -4.4%. Range: -1.5% to -11.2%.
Order-robust (3x P->C, 3x C->P, all favor C).

## 17x257 FP16 (regression control, rowGroup=16, parent row split)

### Same-binary
parent: B1=5.54 B2=6.74 med=6.62 mad/med=0.218 drift=0.181 FAIL
cand:   B1=6.26 B2=6.08 med=6.15 mad/med=0.098 drift=0.029 PASS

### Paired P/C (4 pairs)
p1: P=5.52 C=6.46  +17.0% CLEAN
p2: P=5.98 C=5.58  -6.7% CLEAN
p3: P=5.64 C=8.18  +45.0% CLEAN
p4: P=16.90 C=5.26 -68.9% OUTLIER (P MAD=11.60)

Clean median delta: +5.2%. NO V001 +145% regression. Gate works.

## Verdict

LOCAL_VERDICT = NEEDS_ONE_MORE_LOCAL
ONLINE_WORTHY = NO (delta -4.4% is below 10% threshold)

Rationale:
- 33x100: 5/5 clean pairs favor V002 (order-robust), median -4.4%.
  Direction is consistent but magnitude is below the 10% acceptance threshold.
  cand same-binary FAIL (drift 0.258) though P/C pairs themselves are clean.
- 17x257 FP16: NO parallelism collapse. Median +5.2% vs V001 +145%. Gate confirmed.
- Data quality much improved over first V002 run (5/6 clean vs 1/4).

LOCAL_BEST = NONE
