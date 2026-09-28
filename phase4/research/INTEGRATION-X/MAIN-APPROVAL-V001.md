# MAIN APPROVAL — INTEGRATION-X V001

Date: 2026-09-27
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16

## APPROVED HYPOTHESIS (integration stack)

Combine exactly two independently validated mechanisms on the frozen parent:

1. SCHED-CHAMPION-X V002 gated group-aligned ownership
   SOURCE_SHA de1e93c74338802e646258ada08a8cac1031496a9985e585a5a89015b81bc990
   Local: 33x100 FP32 -4.4% 5/5, no regression on 17x257 FP16

2. VECTOR-MATH-X V001 vector denominator
   SOURCE_SHA dbe776f9165a86ede9d136604e806d9f5dc0f641ee8e05a26bca1ebe8dd33424
   Local: batched 8x256/32x256 ~-7% to -8%, 3/3 favor on 32x256

No other mechanism. OFAT does not apply to integration; this is a
declared two-mechanism merge of two LOCAL signals.

## Expected

- 33x100 FP32: ~= SCHED alone (-4%)
- batched small rows: ~= VECTOR alone (-7%)
- 17x257 FP16: ~= 0 (gated ownership)
- wide path: unchanged

## After implementation

Correctness full matrix. Timing on 33x100, 8x256, 32x256, 17x257 control, 8x8192.
If stack shows wins on multiple shape classes without regression → ONLINE_WORTHY
and prepare Judge package (Main submits).
