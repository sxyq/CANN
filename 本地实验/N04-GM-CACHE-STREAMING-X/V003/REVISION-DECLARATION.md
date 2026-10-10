# N04 V003 Revision Declaration

ROUTE=N04-GM-CACHE-STREAMING-X
REVISION=V003
DIRECT_PARENT=V002 / commit a64b3e812774b6b232bf3f259e3de1c359fe4273
PERFORMANCE_ANCHOR=R31B V011 / Official 45.16
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n04-gm-cache-streaming
BRANCH=route/n04-gm-cache-streaming-x

## Candidate

CANDIDATE_SOURCE_PATH=本地实验/N04-GM-CACHE-STREAMING-X/V003/Candidate.asc
CANDIDATE_SOURCE_SHA256=18081b1b42c298029d3d3547f3611cff977aa99c778e46bd876c60f2d9f71f19
PARENT_SOURCE_PATH=线上结果/R31B/V011/submission.asc
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3

## Single Factor

CHANGE=Restructure the existing wide FP32 full-y path into executable producer and consumer tile helpers.
PRODUCER=ProduceWideFp32TileBatch loads x/residual with the existing valid-byte DataCopyPad path, forms y, squares y, and writes per-tile reduction partials.
CONSUMER=ConsumeWideFp32TileBatch loads gamma/bias, consumes the resident FP32 y tiles, applies the existing normalization and affine sequence, and stores output with the existing MTE3 event pair.
L2_HINT=xGm_ and residualGm_ retain SetL2CacheHint(CACHE_MODE_DISABLE) in ProcessWideFp32FullCacheRows.

## Structure Audit

The initial tile-then-row producer ordering would have changed the cross-row GM access order. That would exceed the inherited N04 boundary, so V003 uses the existing row-then-tile producer order inside a real helper. The consumer keeps the existing tile-then-row parameter/output order. Addresses, valid element counts, arithmetic order within each row, DMA helpers, event IDs, and shape selection remain unchanged.

The change is limited to the N04 FP32 wide path. It does not alter FP16/BF16 paths, Init shape selection, N01 register lifetime, N02 code-layout axis, N03 vector-chain axis, or N05 scalar address generation.

## Build Entry

BUILD_INPUT=existing R31B V016 CMakeLists.probe.txt, runtime_probe_fp32.asc, runtime_probe_fp32.cpp with the existing no-reset adaptation; existing CANN 8.5 provider entry from V002 is used in the build translation units.
PROVIDER_IN_CANDIDATE=NO
NEW_RUNNER=NO
PUSH=NO

## RULE_REFRESH_RECEIPT

Current AGENTS.md, cann-route-executor SKILL.md, server experiment rules, Local measurement rules, Git workflow, and N04 V001/V002 records were read before this revision. V001 L2 hint evidence and V002 provider/full-link and no-reset probe build failures are retained. This revision stays on N04 GM/L2 streaming and does not read other Route worktrees. Source and declaration were created before any V003 build command.

## Initial Status Before Retry

COMPILE=PASS; configure RC=0; device PASS; submission PASS; full_link PASS; final build RC=0
COMPILE_FIRST_ATTEMPT=FAIL; configure RC=1 because the existing CMakeLists.txt input runtime_probe.cpp was not copied; build RC=2; this raw attempt is retained in compile-v003.log
UNRESOLVED_SYMBOLS=NONE; nm -u returned RC=0 with no output for device.cpp.o, adapter.cpp.o, device.alink, and submission.alink
INITIAL_NO_RESET_COMPILE=FAIL; configure RC=0; probe build RC=2; fatal error: 'vector' file not found during ASC registration compilation
INITIAL_CORRECTNESS=NOT_RUN; no probe executable was produced
INITIAL_DEVICE_RUN=NONE; only read-only NPU3 state was sampled
INITIAL_LOCAL_SCORE=NONE
INITIAL_EVIDENCE=compile-v003.log; correctness-v003-npu3.log

## Include Retry And Minimal Correctness

RETRY_ENV=CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward
RETRY_BUILD_INPUT=existing V016 CMakeLists.probe.txt copied as CMakeLists.txt; existing runtime_probe_fp32.asc/.cpp and runtime_probe_v016.asc; no-reset source unchanged
RETRY_CONFIGURE_RC=0
RETRY_TARGETS=probe_fp32_v016; probe_fp32_v011
RETRY_BUILD_RC=0; both FP32 no-reset probe executables produced
RETRY_EVIDENCE=correctness-v003-npu3-retry-cplus-include.log
CORRECTNESS=PASS; minimal rows=2,width=8192 on NPU3
CANDIDATE_RESULT=probe_fp32_v016 PASS; failures=0; max_abs=3.5762787e-07; max_rel=6.0680854e-06; RC=0
PARENT_RESULT=probe_fp32_v011 PASS; failures=0; max_abs=3.5762787e-07; max_rel=6.0680854e-06; RC=0
DEVICE_ID=3
DEVICE_PRE=HBM=96%; Aicore=53%; Aivector=33%; HBM bandwidth=26%
DEVICE_BETWEEN_RUNS=HBM=96%; Aicore=0%; Aivector=0%; HBM bandwidth=23%
DEVICE_POST=HBM=96%; Aicore=71%; Aivector=37%; HBM bandwidth=40%
DEVICE_RUN=PASS; no process was stopped or changed
RESET_CALL=ABSENT_FROM_NO_RESET_PROBE
LOCAL_SCORE=NONE
LOCAL_REASON=No timing sample was collected; only minimal correctness was requested in this retry
EVIDENCE=compile-v003.log; correctness-v003-npu3.log; correctness-v003-npu3-retry-cplus-include.log; correctness-v003-npu3-minimal.log
SOURCE_COMMIT=63341f2b
