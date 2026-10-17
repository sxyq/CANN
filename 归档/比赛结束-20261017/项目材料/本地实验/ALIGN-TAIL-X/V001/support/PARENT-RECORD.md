# ALIGN-TAIL-X V001 pre-code record

```text
ROUTE=ALIGN-TAIL-X
REVISION=V001
DIRECT_PARENT=PURE-R009-V001-ALIGNED-DATACOPY
PARENT_SOURCE_SHA=c8d0f010f8fc68b90d69c6d2bcbf76ec7bb927bd9dc4f0fcac32425e5288c63c
PARENT_SCORE=17.64
PARENT_CORRECTNESS=15/15 online historical
PARENT_SOURCE_PATH=phase4/workspaces/ALIGN-TAIL-X/parent/PURE-R009-V001-ALIGNED-DATACOPY_kernel.txt
SINGLE_HYPOTHESIS=From correct R009 aligned-copy baseline, ONLY add explicit tail specialization: aligned bulk uses direct DataCopy; non-aligned remainder uses a minimal DataCopyPad tail path. Main compute, reduction, scheduling, and parameter policy unchanged. No R012 row-group in V001.
CONTEXT_CLASS=HISTORICAL_DERIVED
SINGLE_CHANGE_AUDIT=PASS (planned)
```

## WHY_NOT_DUPLICATE

- R009 parent only branches on whole-transfer size: non-aligned size always takes full DataCopyPad for the entire `valid` span (including its aligned bulk prefix).
- R010 donor is a separate tail-centric independent implementation with different tiling/scheduling and scalar SetValue compute; it is not this baseline.
- No other next6 route in this worktree owns aligned-bulk + minimal-tail split on the R009 baseline.

## WHY_THIS_COMBINATION_IS_NEW

- Keeps the proven R009 two-pass FP32 compute, 4096 tile, and host scheduling intact.
- Adds only the R010 idea of an explicit tail path, expressed as bulk DataCopy + remainder pad, so the single conceptual change is measurable against the aligned-copy parent.
- Excludes R012 row-group ownership so any local delta is attributable to tail specialization alone.
