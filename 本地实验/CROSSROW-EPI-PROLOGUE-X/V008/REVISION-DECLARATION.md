# W5-R01 CROSSROW-EPI-PROLOGUE-X V008 Revision declaration

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V008
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow
STATUS: COMPLETE_MEASURED
DIRECT_PARENT: R31B/V011 valid Best; V007 is not Parent
PARENT_SOURCE_SHA256: a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SNAPSHOT: V008/Parent.asc (exact snapshot of V007/Parent.asc)
CANDIDATE_SOURCE_SHA256_EXPECTED: 7c6f620ce231e9ebad71b7c8a37bcc8e2ae45ce56086e6f978f02cd1ca9cce06
SOURCE_COMMIT: 5bea6f480e8359abb82a69506862f035aded8e21
BUILD: PASS_RC0 (CMake process; wrapper status discrepancy documented)
PARENT_ARTIFACT_SHA256: e5d05ac104c99450904a4dda4417d6d5ecbd50d04412bc129622b51a56e2cf64
CANDIDATE_ARTIFACT_SHA256: 9c2e0df98f22e9fd4cd6c94bd2743dc08ee98448413014b145bbd96e24c6809c
RUNNER_REF_SHA256: 2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7
CORRECTNESS: PASS_7_OF_7 (14/14 Parent/Candidate RC=0; bad=0)
LOCAL: COMPLETE_FOUR_CORE (24/24 side runs RC=0; bad=0; 744 event samples)
LOCAL_SCORE: 54.312261
LOCAL_DELTA_VS_R31B_V011: -45.687739
PAIRED_PARENT_DIAGNOSTIC_SCORE: 87.528380
MEASUREMENT_QUALITY: POOR
LOCAL_VERDICT: LOCAL_NEUTRAL
NO_PROMOTION: YES
LOCAL_BEST: UNCHANGED (R31B/V011 remains the valid baseline)
RESULT_COMMIT: recorded by the containing route-local result commit (reported separately)

## Single-factor hypothesis

HYPOTHESIS_ID: H1_WIDE_INVRMS_UNUSED_ZERO_INIT
TARGET: `ProcessWideLowPrecision()` in `V008/Candidate.asc`, the single declaration `float invRmsValues[kWideFullYMaxRows] = {};`.
ONLY_CHANGE: remove `= {}` from that declaration; no other source location changes.
MECHANISM: `kWideFullYMaxRows` is 8. `batchLimit` is `wideFullYRows_` returned by `ChooseWideFullYRows()` in the range 1..8, and `batchRows` is capped at `batchLimit`. The Pass-1 scalar RMS loop writes `invRmsValues[0..batchRows)` before Pass 2 reads exactly those indices; inactive tail elements are never read. The initializer therefore performs redundant zeroing of values that are overwritten or unused.
EXPECTED_EFFECT: potentially remove per-batch scalar/local zero-initialization work in the wide low-precision path; strongest opportunity is repeated multi-batch wide shapes. If the compiler already eliminates the stores, expect neutral performance.
FAILURE_MODE: a hidden read of an active value before its Pass-1 assignment would produce incorrect output; the full 7-case correctness suite checks this. If generated code has no redundant initialization, the OFAT is expected to be performance-neutral.

## De-duplication and scope

WHY_NOT_DUPLICATE: R1 V001/V002 change generic next-row input-prologue placement; V003 changes non-wide `valueFp32Buf_` capacity; V004 changes wide pass-1 scalar index generation; V005 changes the FP16 local-copy primitive; V006 removes a duplicated wide pass-2 conversion; V007 moves a pass-2 parameter prefetch. V008 changes only initialization of the active-row scalar result array, starting from R31B/V011 rather than any unpromoted V001-V007 Candidate.
MECHANISM_OVERLAP: NO with R1 V001-V007; NO with W3 CROSSROW, Main-1 MTE3 drain, Store/Epilogue, or Sync/Fastpath; NO with the published ASYNC-OVERLAP-CHAMPION-X V001 parameter-prefetch mechanism.
PATCH_SIMILARITY: one changed source line versus the pinned common Parent; R1 V001-V007 changed-location overlap is zero. Cross-route numeric similarity is not asserted because no other Route private diff was accessed.
UNCHANGED: ABI, tiling, buffer allocation/lifetime, event dependencies, arithmetic, shape matrix, harness, and timing protocol.

## Planned execution

Use the existing published MAIN2-CANONICAL-V1 harness only: Build, seven correctness cases (C01/C08/C11/C12/C13/C14/C16), then four Core cases (C12/C13/C14/C16; 45 warmups, 31 device-event samples, three paired runs per case). Retain all raw/load evidence. V007 remains `72.149204 / POOR / LOCAL_NEUTRAL / NO_PROMOTION` and is not the Parent.

## Measured result

The pinned pure `MAIN2-CANONICAL-V1 scoreVectors` call returned `54.312261`, delta `-45.687739` vs the R31B/V011 anchor, empirical range `23.969468..123.065795`, and paired-Parent diagnostic `87.528380`. Verdict: `POOR / LOCAL_NEUTRAL / NO_PROMOTION`; no Local Best change. Same-binary qualification was `NOT_RUN`. Full result and all raw identities are in `canonical-paired-20261011/`.
