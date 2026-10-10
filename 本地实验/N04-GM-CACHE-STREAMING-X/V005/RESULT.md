# N04 V005 Result

ROUTE=N04-GM-CACHE-STREAMING-X
REVISION=V005
PARENT=V003 / commit 8c65a7d33b497c7957b94bd565bfe34a3508e4c3
REJECTED_SIBLING=V004 / commit ef7a597f
PERFORMANCE_ANCHOR=R31B V011 / Official 45.16
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n04-gm-cache-streaming
BRANCH=route/n04-gm-cache-streaming-x

## Source Change

CANDIDATE=本地实验/N04-GM-CACHE-STREAMING-X/V005/Candidate.asc
CANDIDATE_SOURCE_SHA256=8c76aa021a1daff43b3686f48acc306bd3088329b5cbc2b4e3db8fff13c8c23c
PARENT_SOURCE=本地实验/N04-GM-CACHE-STREAMING-X/V003/Candidate.asc
PARENT_SOURCE_SHA256=18081b1b42c298029d3d3547f3611cff977aa99c778e46bd876c60f2d9f71f19
CHANGE=Only residualGm_ SetL2CacheHint mode changed from CACHE_MODE_DISABLE to CACHE_MODE_NORMAL in ProcessWideFp32FullCacheRows.
HINTS=xGm_ remains CACHE_MODE_DISABLE; residualGm_ uses CACHE_MODE_NORMAL.
UNCHANGED=FP32 producer/consumer structure; row-then-tile order; arithmetic; GM addresses; DataCopyPad; synchronization; events; ownership; shape selection; parameter residency; N01/N03/N05 mechanisms.
NEW_RUNNER=NO

## Compile

HOST=hwnput3
CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
SOC_VERSION=Ascend910B3
REMOTE_ROOT=/tmp/n04-gm-cache-streaming-v005.QIOLfk
BUILD_INPUT=existing R31B V016 CMakeLists.txt, device.cpp, adapter.cpp, link_one.sh, runtime_probe.cpp, and existing V002 provider patch
BUILD_COMMAND=source set_env.sh; export ASCEND_HOME_PATH, ASCEND_CANN_PACKAGE_PATH, SOC_VERSION; cmake -S REMOTE_ROOT -B REMOTE_ROOT/build -DCMAKE_CXX_FLAGS=-DL2_CACHE_HINT; cmake --build REMOTE_ROOT/build --target device submission full_link --verbose --parallel 1 -- -B
PATCH_DRY_RC=0
PATCH_RC=0
CONFIGURE_RC=0
BUILD_RC=0
COMPILE=PASS; device PASS; submission PASS; full_link PASS
UNRESOLVED_SYMBOLS=NONE; nm -u on device.cpp.o, adapter.cpp.o, device.alink, and submission.alink returned no output
COMPILE_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V005/compile-v005.log

## Generated Output Proof

GENERATED_OUTPUT=The V003-to-V005 device.cpp.o, adapter.cpp.o, device.alink, and submission.alink artifacts are all DIFFERENT.
GENERATED_OUTPUT_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V005/generated-v005.log

## No-Reset Probe And Correctness

NO_RESET_BUILD_INPUT=existing V016 CMakeLists.probe.txt adaptation, runtime_probe_fp32.asc, runtime_probe_v016.asc, runtime_probe_fp32.cpp no-reset source, existing CANN provider entry, and V005 Candidate.
CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward
NO_RESET_CONFIGURE_RC=0
NO_RESET_BUILD_RC=0
NO_RESET_BINARY=probe_fp32_v016 and probe_fp32_v011 produced
RESET_CALL=ABSENT_FROM_NO_RESET_SOURCE
CORRECTNESS_COMMAND=probe_fp32_v016 2 32768 3; probe_fp32_v011 2 32768 3
CORRECTNESS=FAIL_PROBE_SHAPE_MISMATCH_SHARED
CANDIDATE_RESULT=FAIL; failures=57358; max_abs=0.091647863; max_rel=95.187904; RC=1
PARENT_RESULT=FAIL; failures=57052; max_abs=0.09269774; max_rel=213.73425; RC=1
FAILURE_CLASS=PROBE_SHAPE_MISMATCH_SHARED; both binaries fail the same FP32 wide workset under the same no-reset probe; the result is not attributed to the cache hint.
CACHE_HINT_CONCLUSION=NONE
V005_STATUS=STOPPED; no further V005 source or device work will be added.
DEVICE_ID=3
DEVICE_PRE=HBM=96%; Aicore=68%; Aivector=35%; HBM bandwidth=0%
DEVICE_BETWEEN_RUNS=HBM=96%; Aicore=63%; Aivector=41%; HBM bandwidth=43%
DEVICE_POST=HBM=96%; Aicore=49%; Aivector=34%; HBM bandwidth=26%
DEVICE_RUN=ATTEMPTED; no process was stopped or changed
CORRECTNESS_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V005/correctness-v005-npu3.log

## Local

LOCAL_SCORE=NONE
LOCAL_REASON=No reliable timing sample was collected; the device load was high and this revision stopped after correctness evidence.

## Boundary And Retained Evidence

PRIOR_V003_CORRECTNESS=Not replayed; V003 PASS evidence remains at its existing paths.
PRIOR_V004_STATUS=Rejected no-op; not used as Candidate.
PRIOR_NEGATIVE_EVIDENCE=V001/V002/V003 records remain unchanged.
PUSH=NO
