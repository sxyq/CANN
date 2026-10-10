# N04 V007 Revision Declaration

ROUTE=N04-GM-CACHE-STREAMING-X
REVISION=V007
DIRECT_PARENT=V006 / commit 8103d6fc1a5cf53448b62864cf0a24182e5d0339
PERFORMANCE_ANCHOR=R31B V011 / Official 45.16
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n04-gm-cache-streaming
BRANCH=route/n04-gm-cache-streaming-x

## RULE_REFRESH_RECEIPT

Current AGENTS.md, cann-route-executor SKILL.md, V003 PASS evidence, V005 shared rows=2,width=32768 probe mismatch evidence, V006 PASS evidence, and the installed CANN cache-hint headers were read before this V007 revision. V005's shared shape failure remains retained and is not replayed. No other Route worktree was read, no Candidate or Runner was created, and PUSH=NO.

## Capability Audit

AUDIT_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V007/capability-audit-v007.log
CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
SOC_VERSION=Ascend910B3
ARCH_BRANCH=__NPU_ARCH__=2201

The installed enum exposes DISABLE, NORMAL, LAST, and PERSISTENT. In the active 2201 `L2CacheAlter` implementation, only DISABLE adds the configured L2 offset; NORMAL, LAST, and PERSISTENT return the original GM address. The `CacheRwMode` template accepts READ, WRITE, and RW, but the same implementation does not branch on that value. V003, V005, and V006 already exercised the two distinct behaviors, DISABLE and NORMAL, on the FP32 input tensors.

AUDIT_RESULT=No untested legal hint with a distinct runtime meaning was found in the installed API for this route. LAST/PERSISTENT and READ/WRITE were retained as negative capability evidence, not used to manufacture a Candidate.

## Boundary

SHAPE=The V003 verified FP32 rows=2,width=8192 remains the next runnable shape, but no V007 source was created because the capability audit found no real new hint factor.
UNCHANGED=parameter residency; DMA descriptors; cross-row prefetch; Vector emission; Scalar address generation; synchronization; ownership; arithmetic; producer/consumer structure; row-then-tile order; N01/N02/N03/N05 mechanisms.
V005_EVIDENCE_RETAINED=rows=2,width=32768 Candidate/Parent shared probe mismatch; no cache-hint conclusion.
NEW_RUNNER=NO
PUSH=NO

## Status

CANDIDATE=NONE
SOURCE_CHANGE=NONE
COMPILE=NOT_RUN; no distinct legal SetL2CacheHint factor was available to compile.
CORRECTNESS=NOT_RUN
LOCAL_SCORE=NONE
STATUS=STOPPED_AFTER_CAPABILITY_AUDIT
