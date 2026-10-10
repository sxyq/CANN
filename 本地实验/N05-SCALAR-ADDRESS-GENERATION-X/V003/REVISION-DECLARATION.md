# N05 V003

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V003
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n05-scalar-address-generation
BRANCH=route/n05-scalar-address-generation-x
DIRECT_PARENT=d81121c3da417888ec6a36807be6715c79ab22ae
HEAD_AT_START=d81121c3da417888ec6a36807be6715c79ab22ae
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16
PARENT_SOURCE=本地实验/N05-SCALAR-ADDRESS-GENERATION-X/V002/submission.asc
PARENT_SOURCE_SHA256=82d2432143f514caedc652fe9ac57b17c3cf1b4aa0012300c2314e0c99d0a0d9
CANDIDATE_SOURCE=本地实验/N05-SCALAR-ADDRESS-GENERATION-X/V003/submission.asc
CANDIDATE_SOURCE_SHA256=c4d44c2ce4da85741d74f6148940242c9d3c56a8f59d22f660e0d3b14f1b076a

## RULE_REFRESH_RECEIPT

HEAD=d81121c3da417888ec6a36807be6715c79ab22ae
DIRECT_PARENT=d81121c3da417888ec6a36807be6715c79ab22ae
ROUTE_MECHANISM=ProcessWideLowPrecision scalar address generation
EXCLUDED_MECHANISMS=N01 register lifetime; N02 code layout/inline; N03 vector instruction scheduling; N04 GM/L2 access policy
SCOPE=Only the N05 worktree and branch; no other Route worktree or branch
EXECUTION_LIMIT=One source change; existing entry only; no new Runner; no device reset; no push

## Resource refresh

RESOURCE_COMMAND=ssh cann-server3 'npu-smi info'
RESOURCE_RC=0
RESOURCE_LOG=resource-snapshot-20261010T1430Z.log
RESOURCE_UPDATE=N01 finished; NPU2 released; NPU3 has no process
NPU2_3=No running processes in the V003 compile snapshots
NPU4_5_6_7=Existing other-user processes
NPU0_1=Existing processes
V002_CORRECTNESS=NONE
V002_LOCAL=NONE

## Single-factor address hypothesis

V003 carries the pass-2 output GM cursor. `col` advances by `tileWidth`, `outputBase` is computed once per batch, and `outputOffset` advances by `rowWidth` after each retained batch row. The existing output Store order and offsets are preserved.

The first-pass V002 input cursor remains unchanged. Vector arithmetic, DMA calls, double-buffer selection, event allocation, event waits, event release, and row/core ownership remain unchanged.

SOURCE_REVIEW=PASS
COMPILE=PASS; CONFIGURE_RC=0; BUILD_RC=0; FINAL_RC=0; SSH_RC=0
STANDALONE_RUNTIME_PROBE_BUILD=PASS; V002=PASS; V003=PASS
PAIRED_RUNNER_BUILD=PASS_AFTER_RETRY; FIRST_ATTEMPT=FAIL_<vector>_NOT_FOUND
RUNTIME_EXECUTION=NOT_RUN
CORRECTNESS=NONE; no runtime binary was executed
LOCAL_SCORE=NONE; no paired device samples were collected
RUNTIME_RESOURCE_LOG=runtime-preflight-npu3.log
RUNTIME_BUILD_LOGS=runtime-compile-v002-v003.log,runtime-compile-probe-entry.log
PUSH=NO
