# CROSSROW-EPI-PROLOGUE-X V009 Revision declaration

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V009
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
STATUS: DECLARED

## Parent and source identity

DIRECT_PARENT: R31B/V011 (reliable baseline; V001-V008 are not used as Parent)
PARENT_SOURCE_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SNAPSHOT: ../V008/Parent.asc (route-owned exact snapshot of R31B/V011)
CANDIDATE_SOURCE_PATH: V009/Candidate.asc
CANDIDATE_SOURCE_SHA256: d8428ff7956242481ca5d2947ccd82934a9a007b13033e66a7d59db47677a473

## Selected single-factor hypothesis

HYPOTHESIS_ID: H1_INPUT_STREAM_L2_CACHE_BYPASS
HYPOTHESIS: In Init(), mark only xGm_ and residualGm_ with AscendC::CacheMode::CACHE_MODE_DISABLE. The canonical Core shapes consume each x/residual element once: C12 retains y in UB, and C13/C14/C16 use the wide retained-y path. Avoiding L2 allocation for these one-pass inputs may reduce cache pollution while leaving the smaller gamma/bias streams in the default cache mode.
TARGET_PATH: AddRmsNormBiasKernel<T>::Init(), immediately after the four input/output GlobalTensor SetGlobalBuffer bindings.
ONLY_CHANGE: two adjacent SetL2CacheHint(CACHE_MODE_DISABLE) calls for xGm_ and residualGm_; no load/store range or order, arithmetic, buffer, event, dispatch, ABI, or runner change.
EXPECTED_SHAPES: Canonical Core C12 (control), C13, C14, C16 (primary); complete seven-case correctness suite.
EXPECTED_EFFECT: possible reduction in L2 pollution from the streaming x/residual reads; neutral or slower result is plausible if the repeated benchmark inputs benefit from L2 residency or the hardware hint is ineffective.
FAILURE_MODE: cache hint may be unsupported/disabled by the compiler contract, making the source change ineffective; when active it may increase external-memory traffic and latency. Correctness should remain unchanged because only the cache address hint changes.

## API and source evidence

The existing route harness targets Ascend910B3 / dav-2201. The installed CANN 8.5 API declares GlobalTensor::SetL2CacheHint; the implementation conditionally encodes CACHE_MODE_DISABLE under L2_CACHE_HINT for NPU_ARCH 2201. The installed DataCopyPad implementation consumes the GlobalTensor cache hint. Build/artifact evidence must establish whether the production compile enables the hint path; no unsupported behavior is assumed.

## De-duplication

WHY_NOT_DUPLICATE: V001/V002 relocate the generic successor-row x/residual prologue; V003 changes non-wide valueFp32Buf_ capacity; V004 changes wide scalar tile-index state; V005 changes the FP16 local-copy primitive; V006 removes a duplicate BF16 conversion; V007 changes wide gamma/bias prefetch timing; V008 removes inactive invRmsValues zero-initialization. None changes GM/L2 cache policy for x/residual. This does not alter W3 crossrow scheduling, Main-1 MTE3 drain, Store/Epilogue, Sync/Fastpath, or parameter-prefetch timing.
MECHANISM_OVERLAP: NO with R1 V001-V008 and the explicitly excluded W3/Main-1 axes; cross-route private diffs were not read.
PATCH_SIMILARITY: CROSS_ROUTE_NUMERIC_VALUE_NOT_COMPUTABLE_WITHIN_ALLOWED_SOURCE_SCOPE; do not represent this field as audited or passed.

## Execution contract

SOURCE_CHECKPOINT -> BUILD -> 7-CASE CORRECTNESS -> FOUR-CORE CANONICAL LOCAL -> RESULT -> COMMIT
BUILD_ENTRY: existing R1 canonical CMake/Adapter under /tmp/w5-r01-canonical.Mk9W1S/V003; no new runner.
LOCAL_CONTRACT: existing MAIN2-CANONICAL-V1; C12/C13/C14/C16, 45 warmups, 31 device-event samples per side, three paired runs per case in P-C/C-P/P-C order; same R31B/V011 Parent artifact/source identity; retain raw and pre/post load snapshots.
LOCAL_BEST: unchanged unless valid canonical scoring and quality support promotion; V008 remains POOR/LOCAL_NEUTRAL/NO_PROMOTION.
