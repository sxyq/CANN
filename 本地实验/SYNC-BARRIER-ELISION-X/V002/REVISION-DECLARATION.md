# SYNC-BARRIER-ELISION-X V002

## Parent

- Direct Parent: `R31B V011`
- Parent source: `线上结果/R31B/V011/submission.asc`
- Parent SHA-256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Parent copy: `parent.asc` (byte-identical to the canonical source)
- Parent Official Score: `45.16`

## Single change

Delete the FP16 `AscendC::PipeBarrier<PIPE_V>()` between the output `Mul` and output `Add` in `ProcessNarrowMidOverlap`. V002 is based directly on the current Local Best `R31B V011`; it does not inherit V001's deletion. No other kernel operation, pipeline stage, mode dispatch, buffer, math expression, or whole-pipeline structure changed.

- Candidate: `submission.asc`
- Candidate SHA-256: `262b4693bf7f24f6a470760ac74c56d443a590a5200ce3a13755d514cd54e37d`
- Exact source diff: `diff.patch`

## Gate results

- Compile: PASS on `hwnput3`, CANN `8.5.0.alpha002`, corrected one-change retry retained in `logs/server3-compile-rerun.log`.
- Correctness: PASS on device `3`, FP16 shapes `8x128`, `8x256`, `8x1024`, `8x2048`, `8x4096`; parent/candidate bit mismatch count `0` for all five cases. Evidence: `logs/correctness-device3-rerun.log`.
- Local: two corrected 31-pair runs on device `3`, FP16 `8x2048`; runner diagnostics were `+12.962958%` and `-2.713181%`. The all-sample route-local aggregate is the geometric mean of `parent_device_us / candidate_device_us` over all 62 pairs: `0.924972624611x`, or `LOCAL_SCORE=-7.502737539%` and `LOCAL_DELTA=-7.502737539%`. `LOCAL_SCORE_TYPE=ROUTE_LOCAL_GEOMEAN_PAIRED_DEVICE_SPEEDUP_PERCENT`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `LOAD_QUALITY=NOISY`. No samples or outliers were removed. Verdict: `LOCAL_REJECTED`; current Local Best remains `R31B-V011`.

## Superseded probe

The earlier `logs/local-device3-superseded-two-change-run1.log` and `logs/local-device3-superseded-two-change-repeat.log` are preserved raw evidence from an invalid two-change harness copy. They are excluded from the V002 result and aggregate.

## Evidence boundary

All V002 evidence is under this route directory. No shared records were modified, no other worktree was accessed, and no Online action was taken.
