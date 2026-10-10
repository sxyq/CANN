# N04 V006 Result

ROUTE=N04-GM-CACHE-STREAMING-X
REVISION=V006
PARENT=V005 / commit 86ca996bcf8dc30d4d654e3700bccc145c35625b
PERFORMANCE_ANCHOR=R31B V011 / Official 45.16
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n04-gm-cache-streaming
BRANCH=route/n04-gm-cache-streaming-x

## Source Change

CANDIDATE=本地实验/N04-GM-CACHE-STREAMING-X/V006/Candidate.asc
CANDIDATE_SOURCE_SHA256=ac67a949fa3ec42e7688a9e709278a4b1286f7ef1a8f7f1623b579ed01cf3960
SOURCE_PARENT=本地实验/N04-GM-CACHE-STREAMING-X/V005/Candidate.asc
SOURCE_PARENT_SHA256=8c76aa021a1daff43b3686f48acc306bd3088329b5cbc2b4e3db8fff13c8c23c
CORRECTNESS_PARENT=R31B V011 / 线上结果/R31B/V011/submission.asc
CHANGE=Only xGm_ SetL2CacheHint mode changed from CACHE_MODE_DISABLE to CACHE_MODE_NORMAL in ProcessWideFp32FullCacheRows.
HINTS=xGm_ uses CACHE_MODE_NORMAL; residualGm_ remains CACHE_MODE_NORMAL.
UNCHANGED=parameter residency; DMA descriptors; cross-row prefetch; Vector emission; Scalar address generation; synchronization; ownership; arithmetic; producer/consumer structure; row-then-tile order.
NEW_RUNNER=NO

## Retained V005 Evidence

V005_SHARED_FAILURE=At rows=2,width=32768, Candidate and Parent both failed under the same no-reset probe.
V005_FAILURE_CLASS=PROBE_SHAPE_MISMATCH_SHARED
V005_CANDIDATE_FAILURES=57358; RC=1
V005_PARENT_FAILURES=57052; RC=1
V005_CACHE_HINT_CONCLUSION=NONE
V005_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V005/correctness-v005-npu3.log

## Compile

HOST=hwnput3
CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
SOC_VERSION=Ascend910B3
REMOTE_ROOT=/tmp/n04-gm-cache-streaming-v006.zibqDR
BUILD_INPUT=existing R31B V016 CMakeLists.txt, device.cpp, adapter.cpp, link_one.sh, runtime_probe.cpp, and existing V002 provider patch
BUILD_COMMAND=source set_env.sh; export ASCEND_HOME_PATH, ASCEND_CANN_PACKAGE_PATH, SOC_VERSION; cmake -S REMOTE_ROOT -B REMOTE_ROOT/build -DCMAKE_CXX_FLAGS=-DL2_CACHE_HINT; cmake --build REMOTE_ROOT/build --target device submission full_link --verbose --parallel 1 -- -B
PATCH_DRY_RC=0
PATCH_RC=0
CONFIGURE_RC=0
BUILD_RC=0
COMPILE=PASS; device PASS; submission PASS; full_link PASS
UNRESOLVED_SYMBOLS=NONE; nm -u on device.cpp.o, adapter.cpp.o, device.alink, and submission.alink returned no output
COMPILE_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V006/compile-v006.log

## Generated Output Proof

GENERATED_OUTPUT=V005-to-V006 device.cpp.o, adapter.cpp.o, device.alink, and submission.alink artifacts are all DIFFERENT.
GENERATED_OUTPUT_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V006/generated-v006.log

## No-Reset Correctness

NO_RESET_BUILD_INPUT=existing V016 no-reset CMake input, runtime_probe_fp32.asc, runtime_probe_v016.asc, runtime_probe_fp32.cpp no-reset source, existing CANN provider entry, and V006 Candidate.
CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward
NO_RESET_CONFIGURE_RC=0
NO_RESET_BUILD_RC=0
NO_RESET_BINARY=probe_fp32_v016 and probe_fp32_v011 produced
RESET_CALL=ABSENT_FROM_NO_RESET_SOURCE
CORRECTNESS_COMMAND=probe_fp32_v016 2 8192 3; probe_fp32_v011 2 8192 3
CORRECTNESS=PASS
CANDIDATE_RESULT=probe_fp32_v016 PASS; failures=0; max_abs=3.5762787e-07; max_rel=6.0680854e-06; RC=0
PARENT_RESULT=probe_fp32_v011 / R31B V011 PASS; failures=0; max_abs=3.5762787e-07; max_rel=6.0680854e-06; RC=0
DEVICE_ID=3
DEVICE_PRE=HBM=96%; Aicore=68%; Aivector=30%; HBM bandwidth=39%
DEVICE_BETWEEN_RUNS=HBM=96%; Aicore=54%; Aivector=56%; HBM bandwidth=42%
DEVICE_POST=HBM=96%; Aicore=66%; Aivector=35%; HBM bandwidth=39%
DEVICE_RUN=PASS; no process was stopped or changed
CORRECTNESS_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V006/correctness-v006-npu3.log

## Local

LOCAL_SCORE=NONE
LOCAL_REASON=NPU3 remained at HBM=96% with active Aicore/Aivector load; no reliable paired Local was recorded and no new Runner was created.

## Boundary

SHAPE=V003 verified rows=2,width=8192; V005 rows=2,width=32768 shared mismatch was retained and not replayed.
PUSH=NO
