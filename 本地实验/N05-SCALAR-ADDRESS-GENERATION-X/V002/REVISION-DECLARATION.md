# N05 V002

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V002
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n05-scalar-address-generation
BRANCH=route/n05-scalar-address-generation-x
DIRECT_PARENT=7e565236fd3e2b4f3d00422f992e6696ca415a93
HEAD_AT_START=7e565236fd3e2b4f3d00422f992e6696ca415a93
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16
PARENT_SOURCE=本地实验/N05-SCALAR-ADDRESS-GENERATION-X/V001/submission.asc
PARENT_SOURCE_SHA256=8183a42535b3130263da3dce4999747f555de053194c32a0087f3c1f53b54558
CANDIDATE_SOURCE=本地实验/N05-SCALAR-ADDRESS-GENERATION-X/V002/submission.asc
CANDIDATE_SOURCE_SHA256=82d2432143f514caedc652fe9ac57b17c3cf1b4aa0012300c2314e0c99d0a0d9

## RULE_REFRESH_RECEIPT

HEAD=7e565236fd3e2b4f3d00422f992e6696ca415a93
DIRECT_PARENT=7e565236fd3e2b4f3d00422f992e6696ca415a93
ROUTE_MECHANISM=ProcessWideLowPrecision scalar address generation
EXCLUDED_MECHANISMS=N01 register lifetime; N02 code layout/inline; N03 vector instruction scheduling; N04 GM/L2 access policy
SCOPE=Only the N05 worktree and branch; no other Route worktree or branch
EXECUTION_LIMIT=One source change; compile with existing CMake targets; no Runner; no device reset; no push

## Single change

V002 carries one flattened row-major GM input offset for the first pass. The step is one tile inside a row and the remaining row tail at a row boundary. Existing `batchRow`, `tile`, and `col` state remains for retained-y and reduction indexing.

Tile traversal, arithmetic, DMA calls, double-buffer selection, event allocation, event waits, event release, and row/core ownership remain unchanged. The second pass is unchanged.

SOURCE_REVIEW=PASS; diff is limited to ProcessWideLowPrecision pass-1 GM input address generation
COMPILE=PASS; ATTEMPT_01_CONFIGURE_RC=1; ATTEMPT_02_CONFIGURE_RC=0; ATTEMPT_02_BUILD_RC=0
CORRECTNESS=NONE; NPU 2/3 were reserved for N01/N02 and NPU 4/5/6/7 had other user processes
LOCAL_SCORE=NONE; no safe device was available and no timing program was started
PUSH=NO
