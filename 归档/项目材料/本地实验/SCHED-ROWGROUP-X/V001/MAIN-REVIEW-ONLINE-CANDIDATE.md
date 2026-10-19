# MAIN-REVIEW — SCHED-ROWGROUP-X V001 ONLINE_CANDIDATE (2026-09-25)

## Identity
- Candidate: 0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c
- Direct Parent: 62de32df… (score 17.14)
- SINGLE_CHANGE_AUDIT=PASS (parent-inherited row-group + R016 band lift; no new mechanism in this revision)

## Evidence accepted
- Same-binary PASS: 33x100 MAD/med 0.0536 drift 0.0990; 17x256 MAD/med 0.0262 drift 0.0247
- P/C 33x100: −51.68 / −51.07 / −52.04 / −50.99 (PC/CP/PC/CP), median −51.38%, 4/4, no reverse pair
- P/C 17x256 control: median −2.18%, ex-p1 −0.2% (neutral, as expected for already-multicore shape)
- Correctness: bad=0 on all 18 runs
- Raw samples retained under results-lh3-sb33/ and results-lh3-pc/

## Decision
ONLINE_CANDIDATE (local). Local −51.38% is NOT Official Score.
Recorded in online-candidate-pool.tsv with frozen SHA.
Judge Owner (unified, MAIN-1) submits only. MAIN-2 does not self-submit.

## Next
Do not implement H1 core-fill / H9 sweep until formal Online result returns and Main issues NEXT_HYPOTHESIS.
