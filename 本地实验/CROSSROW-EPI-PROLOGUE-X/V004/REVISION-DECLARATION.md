# W5-R01 CROSSROW-EPI-PROLOGUE-X V004 Revision declaration

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V004
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
STATUS: IN_PROGRESS

## Existing-result diagnosis

LARGEST_OBSERVED_LOSS: C12
LARGEST_OBSERVED_LOSS_EVIDENCE: V003 candidate median 10.98 us versus frozen anchor 6.58 us (+66.87%); same-window R31B/V011 parent was also 18.56 us. The C12 raw is highly dispersed and the parent drift means this is not a source-attributed regression claim.
SOURCE_ATTRIBUTION_CHECK: C12 is rows=8,width=8192,FP16 and dispatches to ProcessFp16FullRowOutputPipelined. V001/V002 generic Process() prologue edits are not executed there; V003 rowWidth-sized valueFp32Buf_ is equal to the prior kCacheElems allocation at width 8192. Existing V001-V003 diffs therefore do not explain the C12 anchor gap.

## Direct parent and source identity

DIRECT_PARENT: R31B/V011 reliable baseline used by the existing MAIN2-CANONICAL-V1 harness
DIRECT_PARENT_SOURCE_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
DIRECT_PARENT_SOURCE_PATH: existing route-owned canonical asset /tmp/w5-r01-canonical.Mk9W1S/V003/R31B-V011-LP-ROW-PIPELINE_kernel.asc
PARENT_SELECTION_REASON: V003 is POOR/LOCAL_NEUTRAL and is not used as Parent; the published R31B/V011 source is the measured reliable control asset already used by the route's canonical paired evidence.

## Selected single-factor hypothesis

HYPOTHESIS_ID: H1_WIDE_INDEX_STATE
HYPOTHESIS: ProcessWideLowPrecision() repeatedly derives batchRow and tile from the linear unit index with integer division and multiply/subtract operations in the pass-1 tile stream, and repeats the same derivation for the next prefetched unit. Carrying the row/tile state forward removes this scalar address-generation overhead while preserving every GM offset, valid length, buffer selection, event, arithmetic operation, and loop order.
TARGET_CASES: C14 (128x16384 FP16) and C16 (9x32768 FP16/BF16 dtype as registered by the canonical suite); C13 FP32 and C12 full-row FP16 are explicit non-target controls for source-path coverage.

## Orthogonality and scope

WHY_NOT_DUPLICATE: V001/V002 move an existing successor-row MTE2 prologue in generic Process(); V003 changes only non-wide valueFp32Buf_ allocation. V004 changes scalar integer index generation inside the wide low-precision pass-1 loop. It does not change cross-row MTE scheduling, W3 pipeline/retention, Main-1 MTE3 drain, Store-Epilogue, Sync/Fastpath, tiling constants, ABI, or buffer lifetime.
MECHANISM_OVERLAP: NO with the excluded W3/Main-1 MTE3 drain, Store-Epilogue, and Sync axes; no existing R1 committed diff contains this address-state transformation.
PATCH_SIMILARITY: TO_COMPUTE_AFTER_SOURCE_SNAPSHOT

ONLY_CHANGE: one contiguous loop body in ProcessWideLowPrecision(), replacing repeated u/tileCount index reconstruction with rolling batchRow/tile state.
UNCHANGED: Parent identity, runner/harness, ABI, shape suite, event timing, warmup/sample contract, Load/Store calls, event dependencies, arithmetic, and all V001-V003 evidence.

## Acceptance sequence

SOURCE_CHECKPOINT -> BUILD -> 7-CASE CORRECTNESS -> FOUR-CORE CANONICAL LOCAL -> RESULT -> COMMIT

FULL_LOCAL_SCORE: required if the existing canonical path completes; no shape-only result may be promoted or called COMPLETE.
