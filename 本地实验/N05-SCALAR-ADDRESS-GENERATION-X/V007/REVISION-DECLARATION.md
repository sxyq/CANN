# N05 V007

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V007
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n05-scalar-address-generation
BRANCH=route/n05-scalar-address-generation-x
START_HEAD=1251982394eaf6803b34e86e4d01df4e8ba56d9d
PARENT_REVISION=V006
PARENT_COMMIT=1251982394eaf6803b34e86e4d01df4e8ba56d9d
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16
PARENT_SOURCE=本地实验/N05-SCALAR-ADDRESS-GENERATION-X/V006/submission.asc
PARENT_SOURCE_BYTES=193973
PARENT_SOURCE_SHA256=81e8b3e953321c43d4e0f4838558221497d377ddf21e6104016f6dbdaf9aa149
CANDIDATE_SOURCE=本地实验/N05-SCALAR-ADDRESS-GENERATION-X/V007/submission.asc
CANDIDATE_SOURCE_BYTES=194309
CANDIDATE_SOURCE_SHA256=be089220d95db821bd2042d8ae42ff6dd6db4f996fe1e7aa5173bd4a5ce05d0d

## RULE_REFRESH_RECEIPT

HEAD=1251982394eaf6803b34e86e4d01df4e8ba56d9d
PARENT_REVISION=V006
ROUTE_MECHANISM=ProcessWideLowPrecision scalar address generation
EXCLUDED_MECHANISMS=V001 flat-unit decode; V003 output-row carry; V004 WideLowPrecisionCursor; V005 duplicate proposal; V006 Pass2 valid-length parameter carry; N01 register lifetime; N02 code layout/inline; N03 vector instruction scheduling; N04 GM/L2 access policy
SCOPE=Only the N05 worktree and branch; no other Route worktree or branch
EXECUTION_LIMIT=V007 source audit, existing V016 standalone Compile, and NPU5-only Correctness/Local through a temporary no-reset copy; no new formal Runner; no aclrtResetDevice; no push
RULES_READ=AGENTS.md;.agents/skills/cann-route-executor/SKILL.md;项目规则/服务器实验规范.md;项目规则/Git工作流程.md;项目规则/本地性能测试规范.md;~/.agent-global/environment.md
HISTORY_READ=V004/diff.patch;V004/REVISION-DECLARATION.md;V006/diff.patch;V006/REVISION-DECLARATION.md;V005/DUPLICATE-KNOWN-PARENT.md

## RULE_REFRESH_RECEIPT_CURRENT_RUN

CURRENT_HEAD=697535db3c913671f78844a27915291ce30877a0
CURRENT_BRANCH=route/n05-scalar-address-generation-x
CURRENT_WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n05-scalar-address-generation
CURRENT_SCOPE=Only this N05 worktree, branch, existing V016 input, and NPU5
CURRENT_CANDIDATE_SOURCE_SHA256=be089220d95db821bd2042d8ae42ff6dd6db4f996fe1e7aa5173bd4a5ce05d0d
CURRENT_ENTRY=existing V016 paired entry copied temporarily with device=5 and reset call removed
CURRENT_TEMP_ENTRY_SOURCE_SHA256=98338315ac52595cbd705e5c1b1f411462e3aed0569fc9d06d759650ea4cbe72
CURRENT_FORMAL_RUNNER_CREATED=NO
CURRENT_RESET_DEVICE_EXECUTION=NONE

## Source audit

SOURCE_AUDIT=PASS
AUDIT_INPUT=V006/submission.asc to V007/submission.asc; V004 and V006 diff records; V001/V003/V005 exclusion records
NEW_FACTOR=Pass1 retained-y UB address stream advances by the current tile valid span
IMPLEMENTATION=retainedStreamOffset starts at zero, supplies the retained y/value tile base, and advances by valid after each row-major tile
ADDRESS_DOMAIN=Pass1 retained-y local storage only
GM_INPUT_ADDRESS=UNCHANGED; WideLowPrecisionCursor.InputOffset()
GM_OUTPUT_ADDRESS=UNCHANGED; WideLowPrecisionCursor.OutputOffset()
PASS2_PARAMETER_ADDRESS=UNCHANGED; V006 parameterOffset and parameterRemaining retained
HISTORICAL_FLAT_UNIT=EXCLUDED; V001/V002
HISTORICAL_OUTPUT_ROW_CARRY=EXCLUDED; V003
HISTORICAL_CURSOR=EXCLUDED; V004
V005_STATUS=EXCLUDED; DUPLICATE_KNOWN_PARENT
VECTOR_ILP_CHANGED=NO
CACHE_HINT_CHANGED=NO
REGISTER_LIFETIME_CHANGED=NO
TILE_ORDER_CHANGED=NO
ARITHMETIC_CHANGED=NO
DMA_CHANGED=NO
EVENT_OR_SYNC_CHANGED=NO
ROW_CORE_OWNERSHIP_CHANGED=NO

## Validation state

COMPILE=PASS; device/submission/full_link; existing V007 evidence reused
CORRECTNESS=PASS; four cases; V011/V016 failures=0
LOCAL_EXECUTION=PASS; four cases; 21 paired samples per case
LOCAL_SCORE=NONE; VLLM load and high outlier dispersion
DEVICE_ID=5
RUNTIME_LOG=runtime-run-npu5.log
ENTRY_AUDIT=runtime-entry-audit.log
RUNNER=TEMPORARY_NO_RESET_COPY_OF_EXISTING_V016_ENTRY; formal runner not created
RUNTIME_FAILURE=NONE; no OOM or runtime error
PUSH=NO
