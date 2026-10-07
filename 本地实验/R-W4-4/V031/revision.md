# R-W4-4 V031

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`
- REVISION: `V031`
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4096 -> 192`.
- SOURCE_SHA256: `16a7778ee5dc27e251bfd820da0f5b45111c68079faa584d43ab151f42d712e5`
- COMPILE: PASS for `device` and `submission`; first environment-script attempt failed under `set -u`, retry passed. Logs preserve both attempts.
- PROBE_BUILD: PASS for exact Parent/Candidate local probes after preserving failed setup attempts and supplying the existing CANN HCC C++ include path.
- CORRECTNESS: Parent and Candidate PASS (`rc=0`, `bad=0`) on `128x136`, `128x192`, and `128x200` FP32. C15 was not run.
- LOCAL: 6 interleaved pairs per shape; device-event timing, warmup 45, 31 samples x 2 blocks, batch 64. All 36 invocations returned `rc=0`, `bad=0`.
- PARTIAL_CORRECTNESS: YES; the known route Parent C15 failure remains untested and is not classified as a Candidate regression.
- LOCAL_SCORE: `0.992717501551x` partial route-local speedup geomean; `LOCAL_DELTA=-0.72824984%`.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: NO.
- LOAD_QUALITY / MEASUREMENT_QUALITY: NOISY; `NEEDS_ONE_MORE_LOCAL=YES`.
- CURRENT_LOCAL_BEST: NONE. V029/V030/V031 are not promoted.
- NEXT_PARENT: exact R31B-V011 sibling baseline, not V029/V030/V031.
- EVIDENCE: `本地实验/R-W4-4/V031/partial-ranking/`.
