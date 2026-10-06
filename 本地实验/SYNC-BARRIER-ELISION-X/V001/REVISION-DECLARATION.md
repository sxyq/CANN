# SYNC-BARRIER-ELISION-X V001

## Parent

- Direct Parent: `R31B V011`
- Parent source: `线上结果/R31B/V011/submission.asc`
- Parent SHA-256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Parent copy: `parent.asc` (byte-identical to the canonical source)
- Parent Official Score: `45.16`

## Single change

Delete the FP16 `AscendC::PipeBarrier<PIPE_V>()` immediately after the resident-row `Add` and immediately before the existing `SyncVToMTE3()` in `ProcessNarrowMidOverlap`. No other kernel operation, pipeline stage, mode dispatch, buffer, math expression, or whole-pipeline structure changed.

- Candidate: `submission.asc`
- Candidate SHA-256: `ba3eec48d4a4f2b845367219f130829d16e9d6e6fd64de5165d9936cbb35ad86`
- Exact source diff: `diff.patch`

## Gate results

- Compile: PASS on `hwnput3`, CANN `8.5.0.alpha002`, final retry retained in `logs/server3-compile-retry2-pass.log`.
- Correctness: PASS on device `3`, FP16 shapes `8x128`, `8x256`, `8x1024`, `8x2048`, `8x4096`; parent/candidate bit mismatch count `0` for all five cases.
- Local: two 31-pair runs on device `3`, FP16 `8x2048`; runner-reported run diagnostics are `+0.446432%` and `+2.000006%`, with paired median deltas `-0.0400 us` and `-0.1000 us`. The declared route-local aggregate is the geometric mean of `parent_device_us / candidate_device_us` over all 62 pairs: `0.902471005533x`, or `LOCAL_SCORE=-9.752899447%` and `LOCAL_DELTA=-9.752899447%`. `LOCAL_SCORE_TYPE=ROUTE_LOCAL_GEOMEAN_PAIRED_DEVICE_SPEEDUP_PERCENT`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `LOAD_QUALITY=LOW/NOISY`. No samples or outliers were removed. Verdict: `LOCAL_REJECTED`; no Local Best promotion.

## Evidence boundary

All route evidence is under this V001 directory. Server artifacts were copied from `/home/data4t2/lelinfeng/server_runs/SYNC-BARRIER-ELISION-X/V001/`. No other worktree was accessed or modified. No Online action was taken.
