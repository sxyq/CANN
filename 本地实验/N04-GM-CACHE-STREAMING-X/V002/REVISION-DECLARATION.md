# N04 V002 Revision Declaration

ROUTE=N04-GM-CACHE-STREAMING-X
REVISION=V002
DIRECT_PARENT=V001 / commit 8f03481980d3832a0734587a534b3dfac8122d99
PERFORMANCE_ANCHOR=R31B V011 / Official 45.16
WORKTREE=/Users/sunyiyang/Desktop/Project/cann/worktrees/n04-gm-cache-streaming
BRANCH=route/n04-gm-cache-streaming-x

## Candidate

CANDIDATE_SOURCE_PATH=本地实验/N04-GM-CACHE-STREAMING-X/V001/Candidate.asc
CANDIDATE_SOURCE_SHA256=337601ad3b5575b0f669234514af5500849e1d9f6d9df8923a53a0b0e54eb0ca
CANDIDATE_CHANGE=NONE; V001 remains the only kernel source change.
CANDIDATE_PROVIDER_SYMBOL=ABSENT; no g_opSystemRunCfg is present in Candidate.asc.

## One Change

CHANGE=Use the existing CANN 8.5 Catlass provider in the reused V016 build translation units.
BUILD_ENTRY_PATCH=本地实验/N04-GM-CACHE-STREAMING-X/V002/build-entry.patch
PROVIDER_SOURCE=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/opp/built-in/op_impl/ai_core/tbe/impl/ascendc/3rd/template_linear_algebra/include/template_linear_algebra/gemm/kernel/grouped_matmul_slice_m.hpp
PROVIDER_FORM=__gm__ struct OpSystemRunCfg g_opSystemRunCfg{Catlass::L2_OFFSET};
PROVIDER_CONSTANT_SOURCE=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/opp/built-in/op_impl/ai_core/tbe/impl/ascendc/3rd/template_linear_algebra/include/template_linear_algebra/catlass.hpp
PROVIDER_CONSTANT=Catlass::L2_OFFSET=0
UNCHANGED=arithmetic; tile sizes; DMA descriptors; cross-row access order; synchronization; parameter residency; output cache policy; FP16 3x20480 shape excluded.

## Existing V001 Failure Retained

V001_FULL_LINK=FAIL; undefined g_opSystemRunCfg; RC=2.
V001_EVIDENCE=本地实验/N04-GM-CACHE-STREAMING-X/V001/compile-v001-existing-entry.log and link-symbol-investigation.md.
V002_INITIAL_PATCH=FAIL; both build-entry.patch hunks missed in the first remote attempt; individual patch RC was not captured by that command.
V002_INITIAL_BUILD=FAIL; existing unpatched entry reproduced undefined g_opSystemRunCfg; BUILD_RC=2.

## Compile

HOST=hwnput3
CANN=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
SOC_VERSION=Ascend910B3
BUILD_ROOT=/tmp/n04-gm-cache-streaming-v002.TBu8zR
ENV_COMMAND=source /usr/local/Ascend/ascend-toolkit/set_env.sh; export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest ASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002 SOC_VERSION=Ascend910B3
ENV_RC=0
PATCH_DRY_COMMAND=patch --dry-run -p0 < build-entry.patch
PATCH_DRY_RC=0
PATCH_COMMAND=patch -p0 < build-entry.patch
PATCH_RC=0
CONFIGURE_COMMAND=cmake -S /tmp/n04-gm-cache-streaming-v002.TBu8zR -B /tmp/n04-gm-cache-streaming-v002.TBu8zR/build -DCMAKE_CXX_FLAGS=-DL2_CACHE_HINT
CONFIGURE_RC=0
BUILD_COMMAND=cmake --build /tmp/n04-gm-cache-streaming-v002.TBu8zR/build --target device submission full_link --verbose --parallel 1 -- -B
BUILD_RC=0
COMPILE=PASS; device PASS; submission PASS; full_link PASS.
UNRESOLVED_SYMBOL_PROBE=nm -u on device.cpp.o, device.alink and submission.alink returned no output; command RC=0.

## Device And Local Results

DEVICE_ID=3
DEVICE_RUN=NONE
CORRECTNESS=NONE
CORRECTNESS_STATUS=NOT_RUN
CORRECTNESS_REASON=The no-reset FP32 probe targets were not produced. Attempt 3 configured with RC=0 but target build returned RC=2 because ASC registration compilation could not find `vector`.
LOCAL_SCORE=NONE
LOCAL_REASON=No executable and no timing sample.
DEVICE_REASON=No device program was started; NPU3 was sampled read-only before and after the failed build.
DEVICE_PRE=HBM 3428/65536 MB (5%); AICore 0%; no running process.
DEVICE_POST=HBM 3428/65536 MB (5%); AICore 0%; no running process.
NO_RESET_SOURCE=YES
NO_RESET_BINARY=NOT_BUILT
EVIDENCE_LOG=本地实验/N04-GM-CACHE-STREAMING-X/V002/CORRECTNESS-NPU3.md
RAW_LOGS=本地实验/N04-GM-CACHE-STREAMING-X/V002/correctness-v001-npu3-attempt1.log; 本地实验/N04-GM-CACHE-STREAMING-X/V002/correctness-v001-npu3-attempt2-config-missing-runtime-v016.log; 本地实验/N04-GM-CACHE-STREAMING-X/V002/correctness-v001-npu3-attempt3.log; 本地实验/N04-GM-CACHE-STREAMING-X/V002/npu3-closeout.log

## Next

Preserve the build failure. Do not add a Runner or alter Candidate.asc.
