# N04 V006 Revision Declaration

ROUTE=N04-GM-CACHE-STREAMING-X
REVISION=V006
DIRECT_PARENT=V005 / commit 86ca996bcf8dc30d4d654e3700bccc145c35625b
PERFORMANCE_ANCHOR=R31B V011 / Official 45.16
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n04-gm-cache-streaming
BRANCH=route/n04-gm-cache-streaming-x

## Candidate

CANDIDATE_SOURCE_PATH=本地实验/N04-GM-CACHE-STREAMING-X/V006/Candidate.asc
CANDIDATE_SOURCE_SHA256=ac67a949fa3ec42e7688a9e709278a4b1286f7ef1a8f7f1623b579ed01cf3960
PARENT_SOURCE_PATH=本地实验/N04-GM-CACHE-STREAMING-X/V005/Candidate.asc
PARENT_SOURCE_SHA256=8c76aa021a1daff43b3686f48acc306bd3088329b5cbc2b4e3db8fff13c8c23c
CORRECTNESS_PARENT_SOURCE=R31B V011 / 线上结果/R31B/V011/submission.asc

## Single Factor

CHANGE=In ProcessWideFp32FullCacheRows only, change xGm_ SetL2CacheHint from CACHE_MODE_DISABLE to CACHE_MODE_NORMAL.
HINTS=xGm_ uses CACHE_MODE_NORMAL; residualGm_ remains CACHE_MODE_NORMAL from V005.
HINT_TARGET=The x FP32 input GlobalTensor only.
HYPOTHESIS=Test normal L2 treatment for both one-read FP32 input streams at the already verified small wide shape, without changing the kernel schedule.

## Retained V005 Failure

V005_SHARED_FAILURE=At rows=2,width=32768, Candidate and Parent both failed under the same no-reset probe.
V005_CANDIDATE_FAILURES=57358; RC=1
V005_PARENT_FAILURES=57052; RC=1
V005_INTERPRETATION=Probe/shape shared mismatch; no cache-hint conclusion is carried forward.
V005_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V005/correctness-v005-npu3.log

## Unchanged Boundary

SHAPE=Use V003 verified FP32 rows=2,width=8192 only; V005 width=32768 is not replayed.
UNCHANGED=parameter residency; DMA descriptors; cross-row prefetch; Vector emission; Scalar address generation; synchronization; ownership; arithmetic; producer/consumer structure; row-then-tile order.
OTHER_ROUTES_UNCHANGED=N01 register lifetime; N02 layout; N03 Vector ILP; N05 Scalar address generation.
V003_CORRECTNESS=Already PASS at rows=2,width=8192; V006 rechecks Candidate against the R31B V011 Parent through the existing no-reset entry.
NEW_RUNNER=NO

## Build Entry

BUILD_INPUT=existing R31B V016 device/submission/full_link CMake input and existing no-reset FP32 probe input.
PROVIDER_IN_CANDIDATE=NO
DEVICE_TARGET=NPU3 on server3 only
LOCAL_POLICY=Run paired Local only if NPU3 load is reliable; otherwise record NONE.
PUSH=NO

## RULE_REFRESH_RECEIPT

Current AGENTS.md, cann-route-executor SKILL.md, V003 PASS evidence, V005 commit 86ca996b and its shared rows=2,width=32768 probe/shape mismatch evidence were read before this revision. V006 changes one real SetL2CacheHint mode relative to V005, uses the V003 verified FP32 shape, and does not replay the V005 failing shape. No other Route worktree is read and no new Runner is created.

## Pre-Compile Proof

SOURCE_DIFF=V005-to-V006 is one executable line at ProcessWideFp32FullCacheRows: xGm_ CACHE_MODE_DISABLE -> CACHE_MODE_NORMAL.
SOURCE_SHA_PROOF=V005 8c76aa021a1daff43b3686f48acc306bd3088329b5cbc2b4e3db8fff13c8c23c; V006 ac67a949fa3ec42e7688a9e709278a4b1286f7ef1a8f7f1623b579ed01cf3960
COMPILE=PASS; configure RC=0; device PASS; submission PASS; full_link PASS; final build RC=0
COMPILE_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V006/compile-v006.log
NO_RESET_COMPILE=PASS; configure RC=0; probe_fp32_v016 PASS; probe_fp32_v011 PASS; build RC=0
NO_RESET_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V006/correctness-v006-npu3.log
GENERATED_OUTPUT_PROOF=V005 and V006 device/submission objects and device/submission alink artifacts are all DIFFERENT
GENERATED_OUTPUT_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V006/generated-v006.log
CORRECTNESS=PASS; Candidate and R31B V011 Parent both pass rows=2,width=8192 on NPU3
CANDIDATE_RESULT=probe_fp32_v016 PASS; failures=0; max_abs=3.5762787e-07; max_rel=6.0680854e-06; RC=0
PARENT_RESULT=probe_fp32_v011 / R31B V011 PASS; failures=0; max_abs=3.5762787e-07; max_rel=6.0680854e-06; RC=0
V005_SHARED_FAILURE_RETAINED=rows=2,width=32768 Candidate/Parent both FAIL; retained as PROBE_SHAPE_MISMATCH_SHARED and not replayed
DEVICE_ID=3
DEVICE_PRE=HBM=96%; Aicore=68%; Aivector=30%; HBM bandwidth=39%
DEVICE_BETWEEN_RUNS=HBM=96%; Aicore=54%; Aivector=56%; HBM bandwidth=42%
DEVICE_POST=HBM=96%; Aicore=66%; Aivector=35%; HBM bandwidth=39%
DEVICE_RUN=PASS; no process was stopped or changed
RESET_CALL=ABSENT_FROM_NO_RESET_SOURCE
LOCAL_SCORE=NONE
LOCAL_REASON=No reliable paired Local: NPU3 remained at HBM=96% with active Aicore/Aivector load; no new Runner was created
EVIDENCE=compile-v006.log; correctness-v006-npu3.log; generated-v006.log
