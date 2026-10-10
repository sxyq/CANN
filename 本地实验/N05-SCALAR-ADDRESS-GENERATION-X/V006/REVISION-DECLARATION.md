# N05 V006

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V006
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n05-scalar-address-generation
BRANCH=route/n05-scalar-address-generation-x
START_HEAD=5e890330bcbb1363523ee7b7322c2d12d27e06ca
PARENT_REVISION=V004
PARENT_COMMIT=0020dafb77638d4cd4283037d240f4bce8d29eb8
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16
PARENT_SOURCE=本地实验/N05-SCALAR-ADDRESS-GENERATION-X/V004/submission.asc
PARENT_SOURCE_SHA256=c3b936548900cdf623887d73f1a816e145d751f43b330011c9eee9e61837a74e
CANDIDATE_SOURCE=本地实验/N05-SCALAR-ADDRESS-GENERATION-X/V006/submission.asc
CANDIDATE_SOURCE_SHA256=81e8b3e953321c43d4e0f4838558221497d377ddf21e6104016f6dbdaf9aa149

## RULE_REFRESH_RECEIPT

HEAD=5e890330bcbb1363523ee7b7322c2d12d27e06ca
PARENT_REVISION=V004
REJECTED_REVISION=V005; DUPLICATE_KNOWN_PARENT
ROUTE_MECHANISM=ProcessWideLowPrecision scalar address generation
EXCLUDED_MECHANISMS=N01 register lifetime; N02 code layout/inline; N03 vector instruction scheduling; N04 GM/L2 access policy
SCOPE=Only the N05 worktree and branch; no other Route worktree or branch
EXECUTION_LIMIT=V006 source audit, existing V016 Compile, and NPU5-only runtime evidence when available; no new Runner; no aclrtResetDevice; no push
RULES_READ=AGENTS.md;.agents/skills/cann-route-executor/SKILL.md;项目规则/服务器实验规范.md;项目规则/Git工作流程.md;项目规则/本地性能测试规范.md
V004_EVIDENCE_READ=V004/REVISION-DECLARATION.md;V004/COMPILE-RESULT.md;V004/CORRECTNESS-RESULT.md;V004/LOCAL-RESULT.md;V004/runtime-run-npu5.log
V005_EVIDENCE_READ=V005/REVISION-DECLARATION.md;V005/DUPLICATE-KNOWN-PARENT.md

## Source audit

SOURCE_AUDIT=PASS
HISTORICAL_FLAT_UNIT=V001/V002 history; excluded
HISTORICAL_PASS2_OUTPUT_CARRY=V003 `outputOffset += rowWidth`; excluded
HISTORICAL_CURSOR=V004 `WideLowPrecisionCursor`; retained only as parent context, not the new factor
V005_STATUS=DUPLICATE_KNOWN_PARENT; not used as the parent source

## Single change

V006 changes only Pass 2 parameter address formation inside `ProcessWideLowPrecision`.

- `parameterOffset` starts at zero and advances by the current tile's actual `valid` element count.
- `parameterRemaining` carries the unconsumed row width and forms the current and next parameter DMA lengths.
- The next gamma/bias DMA uses `parameterOffset + valid`, rather than deriving the next parameter address from the V004 cursor's column step.
- Pass 1, output GM address formation, vector arithmetic, DMA order, event allocation/wait/release order, synchronization, Tile order, and Row/Core ownership remain unchanged.
- No Vector ILP, cache hint, or register-lifetime change is included.

SOURCE_REVIEW=PASS
COMPILE=PASS; CONFIGURE_RC=0; BUILD_RC=0; FINAL_RC=0; SSH_RC=0
COMPILE_TARGETS=device,submission,full_link
CORRECTNESS=PASS; four cases, both V011 and V016 binaries, failures=0
LOCAL_EXECUTION=PASS; four cases, 21 paired samples per case and binary
LOCAL_SCORE=NONE; VLLM load and large outliers/high dispersion make the timing score unusable
DEVICE_ID=5
RESOURCE_LOG=runtime-run-npu5.log; pre NPU5 HBM 59877/65536 MB with PID 3355594 VLLMWorker_TP 56354 MB; post 59879/65536 MB with PID 3355594 VLLMWorker_TP 56354 MB
COMPILE_LOG=compile-attempt-01.log
RUNTIME_LOG=runtime-run-npu5.log; CONFIGURE_RC=0; BUILD_RC=0; RUN_RC=0; FINAL_RC=0; SSH_RUNTIME_RC=0; REMOTE_TEMP_CLEANUP_RC=0
CANDIDATE_REMOTE_ALIAS_SHA256=81e8b3e953321c43d4e0f4838558221497d377ddf21e6104016f6dbdaf9aa149
PARENT_REMOTE_ALIAS_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
TEMP_RUNNER_SOURCE_SHA256=98338315ac52595cbd705e5c1b1f411462e3aed0569fc9d06d759650ea4cbe72
TEMP_RUNNER_ARTIFACT_SHA256=b21f841b5aef72aa873346e5e0c1b314ba69af2efa3a690c29fc6437cdf0a74e
RESET_DEVICE_EXECUTION=NONE
RUNTIME_FAILURE=NONE; no OOM or runtime error
PUSH=NO
