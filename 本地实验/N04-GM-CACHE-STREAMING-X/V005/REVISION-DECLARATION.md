# N04 V005 Revision Declaration

ROUTE=N04-GM-CACHE-STREAMING-X
REVISION=V005
DIRECT_PARENT=V003 / commit 8c65a7d33b497c7957b94bd565bfe34a3508e4c3
REJECTED_SIBLING=V004 / commit ef7a597f
PERFORMANCE_ANCHOR=R31B V011 / Official 45.16
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n04-gm-cache-streaming
BRANCH=route/n04-gm-cache-streaming-x

## Candidate

CANDIDATE_SOURCE_PATH=本地实验/N04-GM-CACHE-STREAMING-X/V005/Candidate.asc
CANDIDATE_SOURCE_SHA256=8c76aa021a1daff43b3686f48acc306bd3088329b5cbc2b4e3db8fff13c8c23c
PARENT_SOURCE_PATH=本地实验/N04-GM-CACHE-STREAMING-X/V003/Candidate.asc
PARENT_SOURCE_SHA256=18081b1b42c298029d3d3547f3611cff977aa99c778e46bd876c60f2d9f71f19

## Single Factor

CHANGE=In ProcessWideFp32FullCacheRows only, change residualGm_ SetL2CacheHint from CACHE_MODE_DISABLE to CACHE_MODE_NORMAL.
HINTS=xGm_ remains CACHE_MODE_DISABLE; residualGm_ uses CACHE_MODE_NORMAL.
HINT_TARGET=The residual FP32 input GlobalTensor only.
HINT_API=CacheMode supports NORMAL and DISABLE in the installed Ascend C API reference; compilation is required to validate the active CANN entry.
HYPOTHESIS=Allow the residual stream to use normal L2 treatment while the one-read x stream remains bypassed, testing one input hint mode without changing the work schedule.

## Unchanged Boundary

UNCHANGED=FP32 producer/consumer helpers; row-then-tile producer order; arithmetic; GM addresses; DataCopyPad descriptors; V/S/MTE2/MTE3 events; synchronization; ownership; shape selection; parameter residency.
OTHER_ROUTES_UNCHANGED=N01 register lifetime; N02 layout; N03 Vector emission; N05 Scalar address generation.
V003_CORRECTNESS=Already PASS at V003; not rerun as a V003 replay. V005 will use the existing no-reset FP32 probe entry.
V004_CANDIDATE=Not used; V004 was byte-identical no-op.

## Build Entry

BUILD_INPUT=existing R31B V016 device/submission/full_link CMake input and existing no-reset FP32 probe input.
PROVIDER_IN_CANDIDATE=NO
NEW_RUNNER=NO
DEVICE_TARGET=NPU3 on server3 only
PUSH=NO

## RULE_REFRESH_RECEIPT

Current AGENTS.md, cann-route-executor SKILL.md, Ascend C cache-hint API reference, V003 commit 8c65a7d3, and V004 rejected-no-op records were read before this revision. V005 changes one real SetL2CacheHint mode in the confirmed FP32 wide path. V003 correctness is not replayed; V005 Compile and the existing no-reset probe are the next actions. No other Route worktree is read and no new Runner is created.

## Pre-Compile Proof

SOURCE_DIFF=V003-to-V005 is one executable line at ProcessWideFp32FullCacheRows: residualGm_ CACHE_MODE_DISABLE -> CACHE_MODE_NORMAL.
SOURCE_SHA_PROOF=V003 18081b1b42c298029d3d3547f3611cff977aa99c778e46bd876c60f2d9f71f19; V005 8c76aa021a1daff43b3686f48acc306bd3088329b5cbc2b4e3db8fff13c8c23c
COMPILE=PASS; configure RC=0; device PASS; submission PASS; full_link PASS; final build RC=0
COMPILE_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V005/compile-v005.log
NO_RESET_COMPILE=PASS; configure RC=0; probe_fp32_v016 PASS; probe_fp32_v011 PASS; build RC=0
NO_RESET_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V005/correctness-v005-npu3.log
GENERATED_OUTPUT_PROOF=V003 and V005 device/submission objects and device/submission alink artifacts are all DIFFERENT
GENERATED_OUTPUT_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V005/generated-v005.log
CORRECTNESS=FAIL_PROBE_SHAPE_MISMATCH_SHARED; both Candidate and Parent fail rows=2,width=32768 on NPU3; no OOM
CANDIDATE_RESULT=FAIL; failures=57358; max_abs=0.091647863; max_rel=95.187904; RC=1
PARENT_RESULT=FAIL; failures=57052; max_abs=0.09269774; max_rel=213.73425; RC=1
FAILURE_CLASS=PROBE_SHAPE_MISMATCH_SHARED; the common rows=2,width=32768 mismatch prevents attributing the result to the cache hint
CACHE_HINT_CONCLUSION=NONE
V005_STATUS=STOPPED_NO_CACHE_HINT_CONCLUSION; no further V005 source or device work
DEVICE_ID=3
DEVICE_PRE=HBM=96%; Aicore=68%; Aivector=35%; HBM bandwidth=0%
DEVICE_BETWEEN_RUNS=HBM=96%; Aicore=63%; Aivector=41%; HBM bandwidth=43%
DEVICE_POST=HBM=96%; Aicore=49%; Aivector=34%; HBM bandwidth=26%
DEVICE_RUN=ATTEMPTED; no process was stopped or changed
RESET_CALL=ABSENT_FROM_NO_RESET_SOURCE
LOCAL_SCORE=NONE
LOCAL_REASON=No reliable timing sample was collected; device load was high and this revision stopped after correctness evidence
EVIDENCE=compile-v005.log; correctness-v005-npu3.log; generated-v005.log
