# W5-R01 CROSSROW-EPI-PROLOGUE-X V005 Revision declaration

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V005
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
STATUS: SOURCE_CHECKPOINT_READY

## Direct parent and source identity

DIRECT_PARENT: R31B/V011 reliable baseline
DIRECT_PARENT_SOURCE_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
DIRECT_PARENT_SOURCE_PATH: V005/Parent.asc, copied from the published R31B/V011 canonical asset
CANDIDATE_SOURCE_SHA256: 5e749c8acb9b1054e614a93002901bcf411ca9064adde950333851a18842fc64

## Single-factor hypothesis

HYPOTHESIS_ID: H1_FP16_Y_RETENTION_COPY
HYPOTHESIS: In ProcessWideLowPrecision()'s FP16 pass-1, y retention uses a vector multiply by scalar 1.0 solely to copy xLocal into the retained half yTile. Replacing that no-op arithmetic with the supported local-to-local DataCopy path should remove the redundant multiply while preserving the exact half values, y-retention layout, reduction input, and all later dependencies.
TARGET_PATH: ProcessWideLowPrecision(), FP16 branch, pass-1 y-retention at the former Muls(yTile, xLocal, half(1.0f), valid) location.
TARGET_CASE: C13 (128x16384 FP16); C12/C14/C16 remain coverage controls because their canonical dtype/path does not execute this FP16 branch.

## Orthogonality and scope

WHY_NOT_DUPLICATE: V001/V002 relocate the generic successor-row MTE2 prologue; V003 changes non-wide valueFp32Buf_ capacity; V004 changes wide pass-1 scalar tile-index reconstruction. V005 changes only the FP16 pass-1 copy primitive. It does not change MTE scheduling, successor prefetch, W3 row-retention/pipeline ordering, Main-1 MTE3 drain, Store-Epilogue, Sync/Fastpath, tiling, ABI, arithmetic after retention, or buffer lifetime.
MECHANISM_OVERLAP: NO with the known excluded axes.
PATCH_SIMILARITY: ROUTE_LOCAL_SINGLE_LOCATION; cross-route numeric gate remains a review field; no other R1 Candidate is used as a donor.

ONLY_CHANGE: one contiguous source location in `Candidate.asc`; `Parent.asc` remains the R31B/V011 baseline.
UNCHANGED: runner/harness, ABI, shapes, warmup/event protocol, Load/Store boundaries, reduction, output arithmetic, dependencies, and all V001-V004 evidence.

## Acceptance sequence

SOURCE_CHECKPOINT -> BUILD -> 7-CASE CORRECTNESS -> FOUR-CORE CANONICAL LOCAL -> RESULT -> COMMIT

FULL_LOCAL_SCORE: required if the existing complete canonical path succeeds; no diagnostic-only result may be promoted.
