# N04 V003 Result

ROUTE=N04-GM-CACHE-STREAMING-X
REVISION=V003
PARENT=V002 / commit a64b3e812774b6b232bf3f259e3de1c359fe4273
PERFORMANCE_ANCHOR=R31B V011 / Official 45.16
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n04-gm-cache-streaming
BRANCH=route/n04-gm-cache-streaming-x

## Source Change

CANDIDATE=本地实验/N04-GM-CACHE-STREAMING-X/V003/Candidate.asc
CANDIDATE_SOURCE_SHA256=18081b1b42c298029d3d3547f3611cff977aa99c778e46bd876c60f2d9f71f19
CHANGE=The FP32 wide full-y path now calls two executable helpers: ProduceWideFp32TileBatch and ConsumeWideFp32TileBatch.
PRODUCER_ORDER=row-then-tile, matching the prior x/residual GM access order. A tile-then-row trial was rejected because it changed the inherited cross-row access order.
CONSUMER_ORDER=tile-then-batch-row, matching the existing parameter/output pass.
L2_HINT=SetL2CacheHint(CACHE_MODE_DISABLE) remains on xGm_ and residualGm_ in ProcessWideFp32FullCacheRows.
UNCHANGED=Init shape selection; ChooseWideFullYRows; x/residual/gamma/bias/output addresses; DataCopyPad valid-byte counts; Add/Mul/ReduceSum/normalization/affine arithmetic; V/S/MTE2/MTE3 synchronization semantics; FP16/BF16 paths.
PROVIDER_IN_CANDIDATE=NO
NEW_RUNNER=NO

## Compile

HOST=hwnput3
CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
SOC_VERSION=Ascend910B3
REMOTE_ROOT=/tmp/n04-gm-cache-streaming-v003.SlFBn3
BUILD_INPUT=existing R31B V016 CMakeLists.txt, device.cpp, adapter.cpp, link_one.sh, and existing V002 provider patch
BUILD_COMMAND=source set_env.sh; export ASCEND_HOME_PATH, ASCEND_CANN_PACKAGE_PATH, SOC_VERSION; cmake -S REMOTE_ROOT -B REMOTE_ROOT/build -DCMAKE_CXX_FLAGS=-DL2_CACHE_HINT; cmake --build REMOTE_ROOT/build --target device submission full_link --verbose --parallel 1 -- -B
FIRST_ATTEMPT=CONFIGURE_RC=1; existing CMakeLists.txt required runtime_probe.cpp, which was not copied into the temporary build directory; BUILD_RC=2
RETRY=CONFIGURE_RC=0; device PASS; submission PASS; full_link PASS; BUILD_RC=0
UNRESOLVED_SYMBOLS=NONE; nm -u on the two object files and two .alink files returned no output, each command RC=0
COMPILE_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V003/compile-v003.log

## No-Reset Probe And Device State

NO_RESET_BUILD_INPUT=existing R31B V016 CMakeLists.probe.txt, runtime_probe_fp32.asc, runtime_probe_v016.asc, and a temporary no-reset copy of runtime_probe_fp32.cpp with the existing CANN provider entry
NO_RESET_CONFIGURE_RC=0
INITIAL_NO_RESET_BUILD_RC=2
INITIAL_NO_RESET_FAILURE=fatal error: 'vector' file not found during ASC registration compilation
INITIAL_NO_RESET_BINARY=NOT_BUILT
INITIAL_CORRECTNESS=NOT_RUN; no candidate or parent probe command started
INITIAL_DEVICE_RUN=NONE
INITIAL_LOCAL_SCORE=NONE
INITIAL_DEVICE_EVIDENCE=NPU3 was sampled read-only before and after the initial failed build; no process was stopped, reset, or changed by this route
INITIAL_NO_RESET_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V003/correctness-v003-npu3.log

## Include Retry And Minimal Correctness

RETRY_ENV=CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward
RETRY_BUILD_INPUT=existing V016 CMakeLists.probe.txt copied as CMakeLists.txt; existing runtime_probe_fp32.asc/.cpp and runtime_probe_v016.asc; no-reset source unchanged
RETRY_CONFIGURE_RC=0
RETRY_TARGETS=probe_fp32_v016; probe_fp32_v011
RETRY_BUILD_RC=0
RETRY_BINARY=probe_fp32_v016 and probe_fp32_v011 produced
RETRY_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V003/correctness-v003-npu3-retry-cplus-include.log

CORRECTNESS=PASS
CORRECTNESS_COMMAND=probe_fp32_v016 2 8192 3; probe_fp32_v011 2 8192 3
CANDIDATE_RESULT=PASS; failures=0; max_abs=3.5762787e-07; max_rel=6.0680854e-06; RC=0
PARENT_RESULT=PASS; failures=0; max_abs=3.5762787e-07; max_rel=6.0680854e-06; RC=0
DEVICE_ID=3
DEVICE_PRE=HBM=96%; Aicore=53%; Aivector=33%; HBM bandwidth=26%
DEVICE_BETWEEN_RUNS=HBM=96%; Aicore=0%; Aivector=0%; HBM bandwidth=23%
DEVICE_POST=HBM=96%; Aicore=71%; Aivector=37%; HBM bandwidth=40%
DEVICE_RUN=PASS; no process was stopped or changed
RESET_CALL=ABSENT_FROM_NO_RESET_PROBE
LOCAL_SCORE=NONE
LOCAL_REASON=No timing sample was collected; this retry performed minimal correctness only
NO_RESET_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V003/correctness-v003-npu3.log; 本地实验/N04-GM-CACHE-STREAMING-X/V003/correctness-v003-npu3-retry-cplus-include.log; 本地实验/N04-GM-CACHE-STREAMING-X/V003/correctness-v003-npu3-minimal.log
PRIOR_EVIDENCE_RETAINED=V001/V002 compile, provider, and no-reset failure records were not overwritten; existing V016 local records remain unchanged

## Boundary

V003 is limited to N04 GM/L2 streaming. N01 register lifetime, N02 code-layout, N03 vector-chain, and N05 scalar-address mechanisms were not changed. The full source snapshot and both successful and failed build evidence are retained. PUSH=NO.
