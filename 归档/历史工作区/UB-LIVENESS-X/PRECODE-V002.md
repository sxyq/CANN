ROUTE: UB-LIVENESS-X
REVISION: V002
DIRECT_PARENT: UB-LIVENESS-X-V001
PARENT_SOURCE_SHA: 1a6ae30a5eff8f839d08a741c412378acabc936466fb5a07d95502ec69c18c8d
PARENT_SCORE: n/a (local-only, LOCAL_REJECTED)
SINGLE_HYPOTHESIS: Correctness-only repair of numeric path (pass1 accumulator and full u*inv*gamma+bias emit) so NPU bad=0; phase-role UB liveness/alias architecture unchanged; no performance variable.
CONTEXT_CLASS: GUIDED_FRESH / EXTERNAL_DERIVED
SCOPE: Fix correctness only. Keep UB_LIVENESS_ALIAS design. No triple pipeline, no new reduction, no scheduling change, no dtype math redesign beyond matching golden quantization of u.
ISOLATION_START: identity emit out=u already matches golden.
SOURCE_SHA: 888bd60c1efbde1c5f90b1fa13daaaee59a057673315caa8437d16c2b23e3929
COMPILE: PASS
CORRECTNESS: FAIL (improved FP16/BF16 vs V001; FP32 still fail)
DECISION: LOCAL_REJECTED
