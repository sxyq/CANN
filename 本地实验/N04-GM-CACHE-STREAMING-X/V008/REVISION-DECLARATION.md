# N04 V008 Revalidation Declaration

ROUTE=N04-GM-CACHE-STREAMING-X
REVISION=V008
DIRECT_RECORD_PARENT=V007 / commit 1538abca
REUSED_CANDIDATE=V006 / commit 8103d6fc1a5cf53448b62864cf0a24182e5d0339
PERFORMANCE_ANCHOR=R31B V011 / Official 45.16
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n04-gm-cache-streaming
BRANCH=route/n04-gm-cache-streaming-x

## RULE_REFRESH_RECEIPT

The visible root AGENTS.md, cann-route-executor SKILL.md, server experiment rules, Local measurement rules, and Git workflow were read from this worktree before the V008 revalidation. Git was at V007 commit 1538abca with only untracked .codegraph/; .codegraph/ is excluded. V006 Candidate and V007 SetL2CacheHint capability evidence were reused. No other Route worktree, branch source, or unpublished Route experiment was read. No Candidate, Runner, ledger, scheduler, or remote submission was created.

## Source

SOURCE_PATH=本地实验/N04-GM-CACHE-STREAMING-X/V006/Candidate.asc
SOURCE_SHA=ac67a949fa3ec42e7688a9e709278a4b1286f7ef1a8f7f1623b579ed01cf3960
SOURCE_CHANGE=NONE; this record validates the existing V006 Candidate and does not introduce a new hint.
HINT_BOUNDARY=V007 capability evidence remains separate from Compile, Correctness, and Local results; it is not a performance result.
SHAPE=FP32 rows=2,width=8192
DEVICE=server3 NPU3 only

## Compile

COMPILE=PASS
COMPILE_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V008/compile-v008-reverify.log
COMPILE_INPUT=Existing V006 R31B V016 device/submission/full_link CMake input with the existing V002 provider entry and -DL2_CACHE_HINT.
COMPILE_RESULT=configure RC=0; device/submission/full_link RC=0; all four nm -u queries returned no unresolved symbols.
NO_RESET_COMPILE=PASS; configure RC=0; probe_fp32_v016 and probe_fp32_v011 build RC=0.

## Correctness

CORRECTNESS=PASS
CORRECTNESS_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V008/correctness-v008-npu3.log
CANDIDATE_RESULT=PASS; failures=0; max_abs=3.5762787e-07; max_rel=6.0680854e-06; RC=0
PARENT_RESULT=PASS; failures=0; max_abs=3.5762787e-07; max_rel=6.0680854e-06; RC=0
RUNTIME_RETRY=The first launcher attempt returned RC=127 because libgraph.so was not resolved. The existing CANN set_env.sh plus system aarch64 libstdc++ precedence fixed only the shell environment; the retry entered the same no-reset Runner and both probes passed.
V005_EVIDENCE_RETAINED=rows=2,width=32768 Candidate/Parent shared probe mismatch remains in V005 and is not replayed.

## Local

LOCAL=NONE
LOCAL_SCORE=NONE
LOCAL_RAW_SAMPLES=NONE
LOCAL_REASON=The existing Runner has no timing output or device-event measurement. No new Runner was created, and correctness output is not used as Local data.
LOAD_SAMPLES=pre HBM=96% AICore=56% AIVector=51% HBM_BW=8%; between HBM=96% AICore=67% AIVector=34% HBM_BW=41%; post HBM=96% AICore=53% AIVector=51% HBM_BW=27%

## Boundary And Stop

UNCHANGED=parameter residency; DMA descriptors; cross-row prefetch; Vector emission; Scalar address generation; synchronization; ownership; arithmetic; producer/consumer structure; row-then-tile order; N01/N02/N03/N05 mechanisms.
NEW_HINT=NONE; V007 capability result remains a stop condition, not a performance result.
NEW_RUNNER=NO
PUSH=NO
STATUS=V006_VALIDATED_STOPPED_NO_NEW_HINT
NEXT_ACTION=Keep N04 stopped until a new legally distinct GM/L2 runtime semantic is identified; do not create a duplicate Candidate.
