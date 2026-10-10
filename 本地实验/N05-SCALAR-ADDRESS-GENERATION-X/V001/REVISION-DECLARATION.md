# N05 V001

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V001
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n05-scalar-address-generation
BRANCH=route/n05-scalar-address-generation-x
DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
HEAD_AT_START=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
HEAD_COMMIT_PARENT=c9510be4e4a9da0084433e45442468da1eb75c72
PERFORMANCE_REFERENCE=R31B V011 / Official 45.16
PARENT_SOURCE=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE=本地实验/N05-SCALAR-ADDRESS-GENERATION-X/V001/submission.asc
CANDIDATE_SOURCE_SHA256=8183a42535b3130263da3dce4999747f555de053194c32a0087f3c1f53b54558

## RULE_REFRESH_RECEIPT

HEAD=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
DIRECT_PARENT=1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f
ROUTE_MECHANISM=ProcessWideLowPrecision scalar address generation
EXCLUDED_MECHANISMS=N01 register lifetime; N02 code layout/inline; N03 vector instruction scheduling; N04 GM/L2 access policy
NEXT_ACTION=Upload V001 to the N05 server3 temporary directory and compile on NPU 7

## Single change

Only `ProcessWideLowPrecision` pass 1 changed. The `(batchRow, tile, column, row offset)` values now advance by tile increments and row carry. The current and next tile no longer derive their row/tile indexes through integer division and multiplication.

The flattened visit order, row/core ownership, arithmetic, DMA operations, double-buffer selection, synchronization events, and pass 2 remain unchanged.

## Performance assumption

For wide low-precision rows with multiple tiles, carrying scalar indexes may remove repeated integer division and address multiplication from the pass-1 unit loop. The expected effect is limited to scalar address work; no performance result is assumed before device measurements.

COMPILE=PASS; CONFIGURE_RC=0; BUILD_RC=0; TARGETS=device,submission,full_link
CORRECTNESS=NOT_RUN; RUNTIME_PROBE_NOT_RUN_TO_PRESERVE_EXISTING_NPU_7_PROCESSES
LOCAL_SCORE=NONE; SAMPLE_COUNT=0; DEVICE_LOAD_PRESENT
NEXT_ACTION=Record V001 results and create a local commit; PUSH=NO
