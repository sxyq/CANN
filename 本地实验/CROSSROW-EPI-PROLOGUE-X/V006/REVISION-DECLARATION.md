# W5-R01 CROSSROW-EPI-PROLOGUE-X V006 Revision declaration

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V006
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
STATUS: SOURCE_CHECKPOINT_READY

## Direct parent and source identity

DIRECT_PARENT: R31B/V011 reliable baseline
DIRECT_PARENT_SOURCE_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
DIRECT_PARENT_SOURCE_PATH: V006/Parent.asc, copied from the published R31B/V011 canonical asset
CANDIDATE_SOURCE_SHA256: d9297e337f4aee09994ff6d7f41ad3d55ff172a1382ee042b44f791dea114c1c

## Single-factor hypothesis

HYPOTHESIS_ID: H1_WIDE_PARAM_CONVERSION_DUPLICATE
HYPOTHESIS: In ProcessWideLowPrecision() Pass-2, the non-FP16 path converts the same gammaLocal and biasLocal tiles into xFp32/residualFp32 twice consecutively, with the same inputs, outputs, and PipeBarrier. Removing the second identical conversion block should eliminate redundant Vector conversion and barrier work without changing values, buffers, events, or output dependencies.
TARGET_PATH: ProcessWideLowPrecision(), Pass-2 parameter preparation, immediately before the batchRow loop (the second identical ToFloat/PIPE_V block after the first block).
TARGET_CASES: C14 and C16 (wide BF16); C12 is a non-target control and C13 FP16 is compile-time excluded from this block.

## Orthogonality and scope

WHY_NOT_DUPLICATE: V001/V002 relocate the generic successor-row MTE2 prologue; V003 changes non-wide valueFp32Buf_ allocation; V004 changes wide pass-1 scalar index generation; V005 changes the FP16 pass-1 local-copy primitive. V006 changes only one duplicated non-FP16 gamma/bias conversion block in wide Pass-2. It does not change MTE2 scheduling, successor prefetch, MTE3 drain, Store-Epilogue, Sync/Fastpath, tiling, ABI, arithmetic inputs, or buffer lifetime.
MECHANISM_OVERLAP: NO with the known excluded axes.
PATCH_SIMILARITY: ROUTE_LOCAL_SINGLE_LOCATION; no other Route Candidate is used as donor.

ONLY_CHANGE: delete one contiguous duplicate ToFloat/PIPE_V block in Candidate.asc; Parent.asc remains the R31B/V011 baseline.
UNCHANGED: runner/harness, ABI, shapes, warmup/event protocol, Load/Store boundaries, event dependencies, reduction, output arithmetic, and all V001-V005 evidence.

## Acceptance sequence

SOURCE_CHECKPOINT -> BUILD -> 7-CASE CORRECTNESS -> FOUR-CORE CANONICAL LOCAL -> RESULT -> COMMIT

FULL_LOCAL_SCORE: required if the existing complete canonical path succeeds; no diagnostic-only result may be promoted.
