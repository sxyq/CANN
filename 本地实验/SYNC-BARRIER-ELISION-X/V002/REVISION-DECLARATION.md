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

- Compile: PASS on `hwnput3`, CANN `8.5.0.alpha002`, in the owner-handoff run recorded by `logs/server3-compile-owner-20261007T001500Z.log`.
- Correctness: PASS on device `3`, FP16 shapes `8x128`, `8x256`, `8x1024`, `8x2048`, `8x4096`; parent/candidate bit mismatch count `0` for all five cases. Evidence: `logs/correctness-owner-20261007T001500Z.log`.
- Local: two corrected 31-pair runs on device `3`, FP16 `8x2048`, from `2026-10-07T00:19:56Z` through `2026-10-07T00:20:13Z`; all 62 raw pairs were retained. The route-local score is `geomean(parent_device_us / candidate_device_us) - 1` across all pairs: `1.403170394945x`, or `LOCAL_SCORE=+40.317039494%` and `LOCAL_DELTA=+40.317039494%`. Pooled medians are Parent/Candidate `5.50/5.22 us`; effective throughput is `2.978909091/3.138697318 G elements/s`; HBM stayed `91%`, AICore `0%`, and HBM bandwidth `0%`. Device-time CV is `2.387398/2.722910`, with paired-delta p10..p90 `-49.894..+1.514 us`. `LOCAL_SCORE_TYPE=ROUTE_LOCAL_GEOMEAN_PAIRED_DEVICE_SPEEDUP_PERCENT`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `LOAD_QUALITY=LOW/NOISY`. No samples or outliers were removed. Verdict: `LOCAL_REJECTED`; current Local Best remains `R31B-V011`.

## Superseded probe

The earlier `logs/local-device3-superseded-two-change-run1.log` and `logs/local-device3-superseded-two-change-repeat.log` are preserved raw evidence from an invalid two-change harness copy. They are excluded from the V002 result and aggregate.

## Evidence boundary

All V002 evidence is under this route directory. No shared records were modified, no other worktree was accessed, and no Online action was taken.

## Owner handoff revalidation

The existing one-change V002 was recompiled, rechecked, and locally measured after owner replacement. The fresh gate and Local evidence are recorded in `owner-handoff-result.json` and the `*-owner-20261007T*.log` files. The fresh Local aggregate is `+40.317039494%` from all 62 raw paired device samples, with pooled medians `5.50/5.22 us` (Parent/Candidate), `LOAD_QUALITY=LOW/NOISY`, and `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`. This noisy numeric result does not promote V002; the current Local Best remains `R31B-V011`.
