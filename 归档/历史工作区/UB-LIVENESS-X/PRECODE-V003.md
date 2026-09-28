ROUTE: UB-LIVENESS-X
REVISION: V003
DIRECT_PARENT: UB-LIVENESS-X-V002
PARENT_SOURCE_SHA: 888bd60c1efbde1c5f90b1fa13daaaee59a057673315caa8437d16c2b23e3929
PARENT_SCORE: n/a (local-only, LOCAL_REJECTED)
SINGLE_HYPOTHESIS: Correctness-only — make pass1 SumSq/computed inv equal pass2 FuseU u^2 sum (isolation: FP32 D=64 golden acc=60.197), then restore full inv*gamma+bias emit. Keep phase-role UB alias. No performance variable.
CONTEXT_CLASS: GUIDED_FRESH / EXTERNAL_DERIVED
ISOLATION_CARRIED: identity out=u PASS; u*inv_h PASS; res0_ staging and EVENT_ID7 already fixed in V002.
SOURCE_SHA: 2eb9b5d087267a54fb84f8734847ecb68cf94b967102693c0d150fd57d6da7cd
COMPILE: PASS
CORRECTNESS: PASS bad=0 (8/8 + alias0)
ISOLATION: FP32 D=64 acc=60.197 PASS
DECISION: ONLINE_CANDIDATE
