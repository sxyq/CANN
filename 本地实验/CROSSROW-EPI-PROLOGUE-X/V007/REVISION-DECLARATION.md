# W5-R01 CROSSROW-EPI-PROLOGUE-X V007 Revision declaration

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V007
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
STATUS: COMPLETE_MEASURED
SOURCE_FIX_COMMIT: 901ecf4c5f30f4761dc6d0ad921e9a7a8071a11d
BUILD: PASS_RC0
CORRECTNESS: PASS_7_OF_7
LOCAL_SCORE: 72.149204
LOCAL_DELTA_VS_R31B_V011: -27.850796
LOCAL_VERDICT: LOCAL_NEUTRAL
MEASUREMENT_QUALITY: POOR
NO_PROMOTION: YES

## Direct parent and source identity

DIRECT_PARENT: R31B/V011 reliable Champion baseline
DIRECT_PARENT_SOURCE_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
DIRECT_PARENT_SOURCE_PATH: V007/Parent.asc, copied from the published R31B/V011 canonical asset
CANDIDATE_SOURCE_SHA256: c09344af4e9ffb78ca50bf8f23f75a6dca1e24640b803ef1d415777cb46381b5

## Single-factor hypothesis

HYPOTHESIS_ID: H1_WIDE_PARAM_PREFETCH_OVERLAP
HYPOTHESIS: In ProcessWideLowPrecision(), the first gamma/bias tile is loaded only after the scalar RMS calculation. Issuing that independent first parameter MTE2 load immediately after the pass-1 V-to-MTE2 handoff, before the scalar RMS V/S tail, should overlap parameter DMA with denominator calculation while the existing MTE2_V/V_MTE2 event dependencies and pass-2 loop remain unchanged.
TARGET_PATH: ProcessWideLowPrecision(), immediately after the pass-1 per-buffer `V_MTE2` release waits and before the scalar RMS loop; move only the existing Pass-2 parameter event allocation, first gamma/bias Load, and initial ready flag to this earlier window. The existing post-RMS `SyncVToMTE2()` and Pass-2 first `WaitFlag<MTE2_V>` remain in place.
TARGET_CASES: C14 and C16 wide BF16; C12 is a non-target control and C13 exercises the same wide scheduling path with FP16 parameter type.

## Orthogonality and scope

WHY_NOT_DUPLICATE: Within R1 V001-V006, V007 changes only the timing of the existing first gamma/bias parameter prefetch and differs from successor-row input prefetch, buffer sizing, scalar index generation, FP16 local copy, and duplicate conversion. A read-only check of published `origin/w2/main2/control` records found ASYNC-OVERLAP-CHAMPION-X V001 publicly describes the same pass-2 gamma/bias MTE2 hoist above the invRms scalar loop. No other Route source was read or used as donor. This V007 is therefore route-local distinct from R1 V001-V006 and the explicitly excluded W3/Main-1 axes, but not globally novel against that published record.
MECHANISM_OVERLAP: NO with W3 Crossrow, Main-1 MTE3 drain, Store-Epilogue, Sync/Fastpath, or R1 V001-V006; YES with the published ASYNC-OVERLAP-CHAMPION-X V001 mechanism.
PATCH_SIMILARITY: ROUTE_LOCAL_SINGLE_LOCATION; no other Route Candidate is used as donor.

ONLY_CHANGE: move one contiguous Pass-2 parameter-prefetch initialization block earlier within ProcessWideLowPrecision(); the per-buffer release waits establish xBuf_/residualBuf_ availability before the Load, and the existing prd0 wait preserves first-use ordering. Parent.asc remains the R31B/V011 baseline.
UNCHANGED: runner/harness, ABI, shapes, warmup/event protocol, parameter values, event types, reduction, output arithmetic, and all V001-V006 evidence.

## Acceptance sequence

SOURCE_CHECKPOINT -> BUILD -> 7-CASE CORRECTNESS -> FOUR-CORE CANONICAL LOCAL -> RESULT -> COMMIT

FULL_LOCAL_SCORE: 72.149204; complete four-Core canonical measurement. No promotion.
