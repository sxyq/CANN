# N05 V005

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V005
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n05-scalar-address-generation
BRANCH=route/n05-scalar-address-generation-x
PARENT_COMMIT=0020dafb77638d4cd4283037d240f4bce8d29eb8
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16
PARENT_SOURCE=本地实验/N05-SCALAR-ADDRESS-GENERATION-X/V004/submission.asc
PARENT_SOURCE_SHA256=c3b936548900cdf623887d73f1a816e145d751f43b330011c9eee9e61837a74e
PROPOSED_SOURCE_FOR_SCREENING=本地实验/N05-SCALAR-ADDRESS-GENERATION-X/V005/submission.asc
PROPOSED_SOURCE_SHA256=8f5aae77cc549527109ebdcf104c799f81d29052492f434d043f6f85c2e46ed7
CANDIDATE_DECLARED=NO
SOURCE_STATUS=RETAINED_FOR_AUDIT_ONLY

## RULE_REFRESH_RECEIPT

HEAD=0020dafb77638d4cd4283037d240f4bce8d29eb8
PARENT_REVISION=V004
ROUTE_MECHANISM=ProcessWideLowPrecision scalar address generation
EXCLUDED_MECHANISMS=N01 register lifetime; N02 code layout/inline; N03 vector instruction scheduling; N04 GM/L2 access policy
SCOPE=Only the N05 worktree and branch; no other Route worktree or branch
EXECUTION_LIMIT=V005 source screening only; Compile and runtime stopped after duplicate evidence; no new Runner; no aclrtResetDevice; no push
RULES_READ=AGENTS.md;.agents/skills/cann-route-executor/SKILL.md;项目规则/服务器实验规范.md;项目规则/Git工作流程.md;项目规则/本地性能测试规范.md
V004_EVIDENCE_READ=V004/REVISION-DECLARATION.md;V004/COMPILE-RESULT.md;V004/CORRECTNESS-RESULT.md;V004/LOCAL-RESULT.md;V004/runtime-run-npu5.log

## Single change

The V005 source was screened as a proposed change. Its flat-unit scalar coordinate decoding inside `ProcessWideLowPrecision` is already present in the known pre-V004 history and is therefore rejected as a new mechanism.

- The proposed Pass 1 path derives `batchRow = u / tileCount` and `tile = u - batchRow * tileCount`.
- The proposed Pass 2 path derives parameter and output columns from the tile ordinal.
- `V001/diff.patch` already records the same flat-unit formulas in the prior address-generation path.
- No new accepted mechanism is retained; no Vector ILP, cache hint, register-lifetime, or runtime entry change was made.

SOURCE_REVIEW=DUPLICATE_KNOWN_PARENT
DUPLICATE_KNOWN_PARENT=YES
KNOWN_PARENT_EVIDENCE=本地实验/N05-SCALAR-ADDRESS-GENERATION-X/V001/diff.patch:16-17
COMPILE=NOT_RUN
CORRECTNESS=NONE
LOCAL_SCORE=NONE
DEVICE_ID=NONE
DEVICE_SCOPE=NPU5_ONLY; no device run
NEGATIVE_EVIDENCE=DUPLICATE-KNOWN-PARENT.md; compile-attempt-01.log
PUSH=NO
