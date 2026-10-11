# W5-R01 CROSSROW-EPI-PROLOGUE-X V007 Revision declaration

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V007
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
STATUS: SOURCE_CHECKPOINT_READY

## Direct parent and source identity

DIRECT_PARENT: R31B/V011 reliable Champion baseline
DIRECT_PARENT_SOURCE_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
DIRECT_PARENT_SOURCE_PATH: V007/Parent.asc, copied from the published R31B/V011 canonical asset
CANDIDATE_SOURCE_SHA256: 320436ef3364be8de6b8ca2bb4dd62c7dfcf17e48f0b690de0c217e2678fa3d4

## Single-factor hypothesis

HYPOTHESIS_ID: H1_WIDE_PARAM_PREFETCH_OVERLAP
HYPOTHESIS: In ProcessWideLowPrecision(), the first gamma/bias tile is loaded only after the scalar RMS calculation. Issuing that independent first parameter MTE2 load immediately after the pass-1 V-to-MTE2 handoff, before the scalar RMS V/S tail, should overlap parameter DMA with denominator calculation while the existing MTE2_V/V_MTE2 event dependencies and pass-2 loop remain unchanged.
TARGET_PATH: ProcessWideLowPrecision(), between the pass-1 `SyncVToMTE2()` and the scalar RMS loop; move only the existing Pass-2 parameter event allocation, first gamma/bias Load, and initial ready flag to this earlier window.
TARGET_CASES: C14 and C16 wide BF16; C12 is a non-target control and C13 exercises the same wide scheduling path with FP16 parameter type.

## Orthogonality and scope

WHY_NOT_DUPLICATE: V001/V002 relocate generic successor-row input MTE2 prologue issue points; V003 changes non-wide valueFp32Buf_ capacity; V004 changes wide pass-1 scalar index reconstruction; V005 changes the FP16 pass-1 local-copy primitive; V006 removes a duplicate BF16 parameter conversion. V007 changes only the timing of the existing first gamma/bias parameter prefetch. It does not change successor-row prefetch, arithmetic, MTE3 drain, Store/Epilogue, Sync/Fastpath calls, tiling, ABI, or buffer lifetime.
MECHANISM_OVERLAP: NO with W3 Crossrow, Main-1 MTE3 drain, Store-Epilogue, Sync/Fastpath, or V001-V006.
PATCH_SIMILARITY: ROUTE_LOCAL_SINGLE_LOCATION; no other Route Candidate is used as donor.

ONLY_CHANGE: move one contiguous Pass-2 parameter-prefetch initialization block earlier within ProcessWideLowPrecision(). Parent.asc remains the R31B/V011 baseline.
UNCHANGED: runner/harness, ABI, shapes, warmup/event protocol, parameter values, event types, reduction, output arithmetic, and all V001-V006 evidence.

## Acceptance sequence

SOURCE_CHECKPOINT -> BUILD -> 7-CASE CORRECTNESS -> FOUR-CORE CANONICAL LOCAL -> RESULT -> COMMIT

FULL_LOCAL_SCORE: required if the existing complete canonical path succeeds; no diagnostic-only result may be promoted.
