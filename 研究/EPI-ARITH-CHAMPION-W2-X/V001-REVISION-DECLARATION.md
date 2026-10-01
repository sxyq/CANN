# EPI-ARITH-CHAMPION-W2-X V001 Revision Declaration

- ROUTE: `EPI-ARITH-CHAMPION-W2-X`
- REVISION: `V001`
- DIRECT_PARENT: `R31B V011`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- PARENT_SCORE: Official `45.16` (`15/15`)
- SINGLE_HYPOTHESIS: `EAW2-H3-ROW-OP-GROUP`: for each parameter tile, issue `Mul` for all resident rows, one `PIPE_V` barrier, then `Add` for all resident rows and one `PIPE_V` barrier. Keep each element's order `Muls -> Mul -> Add`.
- CONTEXT_CLASS: `WAVE2_OFFICIAL_AWARE_TRACK_A`
- WHY_NOT_DUPLICATE: V011 currently interleaves each resident row's `Mul`, barrier, `Add`, barrier. H3 changes only cross-row issue grouping and keeps each element's operation order. Wave-1 EPI V001 hoists `Muls`; EPI V002 changes affine order; EPILOGUE-FUSE V003 combines arithmetic and changes its dataflow; R31A V028 removes terminal barriers before event setup without grouping these arithmetic operations. Main confirms Wave-2 SYNC H2 is limited to low-precision execution.

## One-Factor Boundary

- TARGET: `rows=2,D=12288,FP32,blockCount=1`; record runtime `batchRows` and confirm the wide-FP32 full-cache path is reached.
- CONTROL: `rows=2,D=8192,FP32,blockCount=1`; preserve its existing path and make no control-specific source change.
- CHANGE: Replace only the second-pass row-wise `Mul`/`Add` issue loop in `ProcessWideFp32FullCacheRows` with one resident-row `Mul` loop, one `PIPE_V` barrier, one resident-row `Add` loop, and one `PIPE_V` barrier.
- PRESERVE: `Muls`, arithmetic order, Loads, Stores, dispatch, tiling, buffers, row ownership, reduction, and all other synchronization.
- PRECISION: No reassociation or dtype change. Precision validation must pass before any performance timing.
- VALIDATION: Await Main device/job assignment. No compile, precision run, timing, or server connection is included in this declaration.

## Evidence

- Selection and Main review: `研究/EPI-ARITH-CHAMPION-W2-X/MAIN-SELECTION-H3.md`, backed by `研究/主代理/MAIN-1-W2/campaign-status.md` at commit `2a27be0bbaa81b7f67777f7d8e99277cbe23946a`.
- Parent source and Official evidence: `线上结果/R31B/V011/submission.asc`, `线上结果/R31B/V011/source-meta.json`.
- Duplicate mechanisms: `线上结果/EPILOGUE-ARITH-CHAMPION-X/V001/submission.asc`, `线上结果/EPILOGUE-ARITH-CHAMPION-X/V002/diff.patch`, `研究/EPILOGUE-FUSE-X/TRACK-B-HYPOTHESES-V003.md`, `线上结果/R31A/V028/diff.patch`.
