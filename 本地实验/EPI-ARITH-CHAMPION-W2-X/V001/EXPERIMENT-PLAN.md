# EPI-ARITH-CHAMPION-W2-X V001 Local Experiment Plan

- STATUS: Candidate source prepared from the exact V011 parent; no build, correctness run, or timing has been performed.
- ROUTE / REVISION: `EPI-ARITH-CHAMPION-W2-X` / `V001`
- DIRECT_PARENT: R31B V011, Official `45.16`
- PARENT_SOURCE_SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SELECTED_HYPOTHESIS: `EAW2-H3-ROW-OP-GROUP`
- DEVICE_AND_JOB: Await Main assignment; no device lease or server connection created by this route.
- HARNESS: `TEST-HARNESS.md`; exact-source ASC/ACL build and correctness commands are prepared, not executed.

## Target

- INPUT: `rows=2,D=12288,FP32`
- LAUNCH: `blockCount=1`
- PATH: Expected `widePath` and `ProcessWideFp32FullCacheRows`; confirm from the assigned run.
- BATCH_ROWS: Source-derived value is `2`: `ChooseWideFullYRows(12288, ...)` returns 2 and `localRows=2` for `blockCount=1`. Runtime observation is pending the assigned run.
- CONTROL: `rows=2,D=8192,FP32,blockCount=1`. Since V011 uses `rowWidth > kCacheElems` with `kCacheElems=8192`, this remains on the existing non-wide path and receives no H3 change.

## One-Factor Change

In the wide-FP32 full-cache output pass, keep the existing per-row Muls loop. Replace only the row-interleaved `Mul, barrier, Add, barrier` loop with a resident-row Mul loop, one `PIPE_V` barrier, a resident-row Add loop, and one `PIPE_V` barrier. Each element remains `Muls -> Mul -> Add`.

## Probe Order

1. Wait for Main to assign device 7 and the job, then synchronize the committed Route tree and V011 parent source to the documented server root.
2. Run the exact-source build command in `TEST-HARNESS.md`; preserve its source checks, artifact identities, and build log.
3. Run the separate correctness command for Parent and Candidate across the documented FP32 matrix before any performance timing.
4. Confirm device, `blockCount`, selected path, and logged source-derived effective `batchRows` against the matrix. This does not instrument the kernel-local variable; Main may verify the compiled execution only after the assigned correctness run.
5. Only after correctness passes, wait for a separate Main-assigned performance job before any measurement.

No probe output exists yet. Do not start a server job or create a device lease before Main assignment.
