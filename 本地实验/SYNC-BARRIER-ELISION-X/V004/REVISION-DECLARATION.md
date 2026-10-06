# SYNC-BARRIER-ELISION-X V004

## Parent

- Direct Parent: `R31B V011` (current route Local Best; V001–V003 did not promote).
- Parent SHA-256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Parent copy: `parent.asc`.

## Single change

Remove the FP16 `AscendC::PipeBarrier<PIPE_V>()` immediately after the input `Add(xLocal, xLocal, residualLocal, valid)` and before `ToFloat(valueLocal, xLocal, valid)` in `ProcessNarrowMidOverlap`. No other kernel operation or pipeline behavior is changed. This V004 candidate was already present in the assigned route worktree when ownership was resumed; this declaration records that exact candidate before further testing.

- Candidate SHA-256: `179c85f15163c1e41d1d87d3d61d96485b10183f6cc33036c34ee922cf8358bb`.
- Exact source diff: `diff.patch`.
- Build executable SHA-256: `db79236f011bfcff8786d48503a6bab38b9acb2c1d615793b94e6efc64b752ec`.
- Correctness executable SHA-256: `fa0dcc66b235104ace41e9afe47c39d7978195eb0eb4e5804fd82ab07ec384ae`.

## Current gates

- Compile/link: PASS on `hwnput3`, CANN `8.5.0.alpha002`; see `logs/server3-compile.log`.
- Correctness: PASS on device 3, FP16 `8x128`, `8x256`, `8x1024`, `8x2048`, `8x4096`; all parent/candidate bit mismatch counts are zero. See `logs/correctness-device3.log`.
- Local measurement: two 31-pair runs on device `3`, FP16 `8x2048`, completed at `2026-10-06T23:39:33Z` and `2026-10-06T23:40:58Z`. All 62 raw pairs were retained with no outlier filtering. The pooled geometric mean of `parent_device_us / candidate_device_us` is `0.710888012440x`, giving `LOCAL_SCORE=-28.911198756%` and `LOCAL_DELTA=-28.911198756%`. `LOCAL_SCORE_TYPE=ROUTE_LOCAL_GEOMEAN_PAIRED_DEVICE_SPEEDUP_PERCENT`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `LOAD_QUALITY=LOW/NOISY`. Parent/candidate pooled medians are `4.98/5.81 us`; pooled paired-delta median is `+0.56 us`; current Local Best remains `R31B-V011`. Full raw samples and jitter are in `local-result.json` and the two `logs/local-device3-*-full.log` files.
- Online: NOT RUN.
