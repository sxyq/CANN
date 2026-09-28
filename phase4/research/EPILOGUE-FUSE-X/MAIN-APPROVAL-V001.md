# MAIN APPROVAL — EPILOGUE-FUSE-X V001

Date: 2026-09-27
Approver: MAIN-2
DIRECT_PARENT=FROZEN_R31B_V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
OFFICIAL_ANCHOR=45.16

APPROVED SINGLE HYPOTHESIS:
HYPOTHESIS-1 SCALE-FOLD: replace (Muls value invRms) then (Mul value gamma) with (Muls scale gamma invRms) then (Mul value scale); Add bias unchanged. One arithmetic identity (y*a)*b -> y*(a*b). Uniform across T; FP32 intermediates; no dtype split; no sync change; no reduction/scheduling/mode change.

OFAT=PASS
NON_DUPLICATION_AUDIT=PASS
MAIN_APPROVAL=YES

RULES:
- Exactly one conceptual performance change in V001.
- Build/correctness-only repairs may ride in the same revision if they add no performance mechanism.
- After implementation: local SHA -> server3 source SHA -> compile (numeric RC) -> link (numeric RC) -> executable SHA -> NPU correctness (FP16/BF16/FP32 + relevant shapes).
- Do not wait for a performance window to compile/link/correctness.
- Timing only under phase4/control/local-timing-protocol.md.
- No CANNJudge self-submit.
- If correctness fails, only correctness/build repair is allowed next.
- If LOCAL_REJECTED, roll back to frozen parent; do not stack another performance change.

STOP CONDITIONS:
CONCEPT_COLLISION_WITH_MAIN1
DO_NOT_IMPLEMENT_DUPLICATE
INPUT_IDENTITY_MISMATCH
