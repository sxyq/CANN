# SYNC-BARRIER-ELISION-X V003

## Parent

- Direct Parent: `R31B V011`
- Parent SHA-256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Parent copy: `parent.asc` (byte-identical to the canonical source)
- Parent Official Score: `45.16`

## Single change

Delete the FP16 `AscendC::PipeBarrier<PIPE_V>()` between `FromFloat(outputLocal, valueLocal, valid)` and the output `Mul` in `ProcessNarrowMidOverlap`. V003 is based directly on the current Local Best `R31B V011`; it does not inherit V001 or V002 deletions.

- Candidate SHA-256: `e2e9ed435e36855f493010aaa2d6ae10b8108922e631d36ee3acf3df604506b7`
- Exact source diff: `diff.patch`

## Gate results

- Compile: PASS on `hwnput3`, CANN `8.5.0.alpha002`, `logs/server3-compile.log`.
- Correctness: PASS on device `3`, FP16 shapes `8x128`, `8x256`, `8x1024`, `8x2048`, `8x4096`; parent/candidate bit mismatch count `0` for all five cases.
- Local: two 31-pair runs on device `3`, FP16 `8x2048`; runner diagnostics were `+0.800006%` and `-2.840911%`. The all-sample route-local aggregate is the geometric mean of `parent_device_us / candidate_device_us` over all 62 pairs: `0.967573056306x`, or `LOCAL_SCORE=-3.242694369%` and `LOCAL_DELTA=-3.242694369%`. `LOCAL_SCORE_TYPE=ROUTE_LOCAL_GEOMEAN_PAIRED_DEVICE_SPEEDUP_PERCENT`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `LOAD_QUALITY=NOISY`. No samples or outliers were removed. Verdict: `LOCAL_REJECTED`; current Local Best remains `R31B-V011`.

## Evidence boundary

All V003 evidence is under this route directory. No shared records were modified, no other worktree was accessed, and no Online action was taken.
