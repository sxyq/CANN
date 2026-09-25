# ASYNC-TRIPLE-X V001 revision declaration

ROUTE=ASYNC-TRIPLE-X
REVISION=V001
DIRECT_PARENT=R013-derived route seed
PARENT_SOURCE_SHA=ba1167079cf54277506e4b4fa192a91a036a6b8d992650e8532fb182c555bda0 (SHA-256 of the exact parent kernel source)
PARENT_COMMIT_SHA=fe04b33ed02a7fb4b4e725ec27ed3c203fa8f6cb
PARENT_SCORE=N/A (route seed has no formal score)
SINGLE_HYPOTHESIS=only MTE3 overlap
CONTEXT_CLASS=HISTORICAL_DERIVED

PARENT_SOURCE_PATH=phase4/archive/historical-branches-20260924/independent__full-r013-double-buffer-pipeline-i001/files/提交/独立实现/FULL-R013-DOUBLE-BUFFER-PIPELINE/I001/

V001 scope: preserve the R013-derived two-stage MTE2+V pipeline and add only MTE3 store overlap so that MTE2 prefetch N+1, Vector compute N, and MTE3 store N-1 can overlap.

Out of scope: reduction changes, mathematical precision changes, dtype strategy changes, parameter reuse changes, core mapping changes, mode threshold changes, other performance mechanisms, copied external code, CANNJudge, shared scheduler edits, Main push, and V002 creation.

This declaration was written before any V001 source modification.

## Build-fix declaration note (before build-structure changes)

2026-09-24: The first server3 attempt compiled the ASC translation unit but linked an ASC executable without a host main, producing undefined symbol: main. The subsequent mixed target also routed the ASC object through the ordinary CXX linker and could not resolve the CANN libraries. This note authorizes only a build/tooling correction within V001: restore the task-ABI host entry point and separate device compilation, submission assembly, and full-link validation. The kernel pipeline and its single MTE3-overlap hypothesis remain unchanged.

The next build must use the server3 CANN 8.5.0.alpha002 environment with dav-2201; the failed logs remain retained. No targeted correctness or timing is authorized until device, submission, and full-link stages pass.

## Compiler-managed correctness probe note

2026-09-24: Add route-owned support-only ASC probe translation units that include this V001 `main.asc` or the exact direct-parent `parent_main.asc`, rename their dummy `main`, and call the existing `run_kernel` wrapper for FP32 rows=1,width=1024. The probe will use fixed C-style buffers, check stream synchronization, and compare against a CPU reference. Candidate kernel, parent kernel, task ABI, and the declared MTE3-overlap hypothesis remain unchanged. Probe/build outputs are retained separately from prior native-ACL and ASC attempts; no timing is authorized.
