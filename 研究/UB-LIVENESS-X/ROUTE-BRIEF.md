# ROUTE UB-LIVENESS-X

ROUTE: UB-LIVENESS-X
REVISION: V001
CONTEXT_CLASS: GUIDED_FRESH / EXTERNAL_DERIVED
WORKTREE: /Users/sunyiyang/Desktop/Project/cann-next6/UB-LIVENESS-X
BRANCH: exp/next6-ub-liveness-x
DIRECT_PARENT: none (do not copy existing candidate skeleton)
PARENT_SOURCE_SHA: n/a
PARENT_SCORE: n/a
SINGLE_HYPOTHESIS: Redesigning buffer lifetime/aliasing so the same UB region serves different roles in different phases frees UB for larger tile, more parameter residency, or true pipeline buffers.

## Scope
V001 validates UB lifetime / alias architecture ONLY.
Do NOT simultaneously introduce triple pipeline, new reduction, new scheduling, or new dtype math.

## Allowed inputs
Math semantics, hardware limits, Ascend C API, historical architecture evidence, public kernel architecture ideas (vLLM-Ascend, FlashInfer, FlashAttention, Triton/CUDA RMSNorm). Do NOT copy external code.

## External idea record (if used)
EXTERNAL_IDEA / SOURCE / MECHANISM / WHY_DIFFERENT_FROM_EXISTING_29 / EXPECTED_BOTTLENECK / PROVENANCE_CLASS

## Loop
Local-first; no CANNJudge; one conceptual change. Same agent owns V001-V003.

