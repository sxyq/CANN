# N05 V004

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V004
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n05-scalar-address-generation
BRANCH=route/n05-scalar-address-generation-x
DIRECT_PARENT=707da119369f46435c19239d29192c2d2c35d3b1
HEAD_AT_START=707da119369f46435c19239d29192c2d2c35d3b1
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16
PARENT_SOURCE=本地实验/N05-SCALAR-ADDRESS-GENERATION-X/V003/submission.asc
PARENT_SOURCE_SHA256=c4d44c2ce4da85741d74f6148940242c9d3c56a8f59d22f660e0d3b14f1b076a
CANDIDATE_SOURCE=本地实验/N05-SCALAR-ADDRESS-GENERATION-X/V004/submission.asc
CANDIDATE_SOURCE_SHA256=c3b936548900cdf623887d73f1a816e145d751f43b330011c9eee9e61837a74e

## RULE_REFRESH_RECEIPT

HEAD=707da119369f46435c19239d29192c2d2c35d3b1
DIRECT_PARENT=707da119369f46435c19239d29192c2d2c35d3b1
ROUTE_MECHANISM=ProcessWideLowPrecision scalar address generation
EXCLUDED_MECHANISMS=N01 register lifetime; N02 code layout/inline; N03 vector instruction scheduling; N04 GM/L2 access policy
SCOPE=Only the N05 worktree and branch; no other Route worktree or branch
EXECUTION_LIMIT=V004 runtime evidence only; existing V016 entry; NPU5 only; no new Runner; no aclrtResetDevice; no push
RULES_READ=AGENTS.md;.agents/skills/cann-route-executor/SKILL.md;项目规则/服务器实验规范.md;项目规则/Git工作流程.md
V001_EVIDENCE_READ=本地实验/N05-SCALAR-ADDRESS-GENERATION-X/V001/REVISION-DECLARATION.md;COMPILE-RESULT.md;CORRECTNESS-RESULT.md;LOCAL-RESULT.md;diff.patch
RUNTIME_REFRESH=V004 standalone compile commit 1cc9c23f; NPU5 is the only permitted device for this run

## Single change

`WideLowPrecisionCursor` is a real executable cursor microkernel structure used by both passes of `ProcessWideLowPrecision`.

- Pass 1 obtains unit, row, tile, column, valid length, and row-major input GM offset from `inputCursor`.
- Pass 2 obtains parameter column, next-tile state, and output GM offset from `outputCursor`.
- `Advance()` carries one tile step or the remaining row tail, preserving the flattened row-major stream.
- The existing tile order, arithmetic, DMA calls, event allocation/wait/release sequence, and row/core ownership remain unchanged.
- Vector instruction scheduling and cache hints remain unchanged.

SOURCE_REVIEW=PASS; V003 to V004 is a substantive executable diff (154 added lines, 52 removed lines); V004 is not byte-identical to V003
COMPILE=PASS; CONFIGURE_RC=0; BUILD_RC=0; FINAL_RC=0; SSH_RC=0
COMPILE_TARGETS=device,submission,full_link
CORRECTNESS=PASS; four cases, both V011 and V016 binaries, failures=0
LOCAL_EXECUTION=PASS; four cases, 21 paired samples per case and binary
LOCAL_SCORE=NONE; VLLM load and large outliers/high dispersion make the timing score unusable
DEVICE_ID=5
RESOURCE_LOG=runtime-preflight-npu5.log; pre 59737/65536 MB with PID 3355594 VLLMWorker_TP 56358 MB; post 59880/65536 MB with PID 3355594 VLLMWorker_TP 56354 MB
RUNTIME_ENTRY_AUDIT=runtime-entry-audit.log; temporary copy of existing V016 paired entry used on device 5 with aclrtResetDevice removed
RUNTIME_LOG=runtime-run-npu5.log; CONFIGURE_RC=0; BUILD_RC=0; RUN_RC=0; FINAL_RC=0; SSH_RUNTIME_RC=0; REMOTE_TEMP_CLEANUP_RC=0
CANDIDATE_SOURCE_SHA256=c3b936548900cdf623887d73f1a816e145d751f43b330011c9eee9e61837a74e
COMPILE_INPUT_ALIAS_SHA256=c3b936548900cdf623887d73f1a816e145d751f43b330011c9eee9e61837a74e
SOURCE_INPUT_MATCH=PASS
TEMP_RUNNER_SOURCE_SHA256=98338315ac52595cbd705e5c1b1f411462e3aed0569fc9d06d759650ea4cbe72
TEMP_RUNNER_ARTIFACT_SHA256=fc2fde3b1ad55898f70e5c1e4d7595cc828c4b04061803ef73ebe0a990bff7b6
RUNTIME_FAILURE=NONE; no OOM or runtime error; no device reset executed
PUSH=NO
