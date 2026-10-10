# W5-R01 CROSSROW-EPI-PROLOGUE-X V003 Revision declaration

AGENT_ID: W5-R01/Zeno
ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V003
BRANCH: research/w5-r01-crossrow-epi-prologue
WORKTREE: /home/data4t2/lelinfeng/cann-w5-r01-crossrow

## Parent and hypothesis

DIRECT_PARENT_REVISION: V002 Candidate
DIRECT_PARENT_SOURCE_PATH: ../V002/Candidate.asc
DIRECT_PARENT_SOURCE_SHA256: 426489bfdc63441f2fd179fdff176875b24d815e6e0420343aeed0b0dab7933f
HYPOTHESIS_ID: H1_VALUE_FP32BUF_ROW_WIDTH_CAPACITY
HYPOTHESIS: the generic cached-row path reserves valueFp32Buf_ for kCacheElems even when rowWidth is smaller; sizing that existing buffer to the actual row width reduces UB footprint without changing the retained-y lifetime, input prologue placement, arithmetic, output stores, or event boundaries.

## Single-factor scope

EDIT_SCOPE: Candidate.asc only; AddRmsNormBiasKernel::Init(); the existing valueFp32Buf_ InitBuffer allocation at the V002 line-146 source location.
ONLY_CHANGE: replace the kCacheElems-sized valueFp32Buf_ allocation with rowWidth-sized FP32 storage.
UNCHANGED: V002 source, Parent snapshot, ABI, runner, shape, arithmetic, Load/Store helpers, and all Sync/event calls.
EXPECTED_SHAPE: rows=16,width=6144,blocks=8,dtype=fp16; localRows=2 and two generic tiles.
M1_CONTROL: rows=1,width=6144,blocks=1,dtype=fp16 correctness only; existing Local runner requires at least two rows per effective block.

## Dedup and gate

MECHANISM_OVERLAP: NO with successor-row MTE2 prologue placement, W3 CROSSROW pipeline/retention scheduling, Main-1 MTE3 drain, Store-Epilogue, or Sync/Fastpath changes.
PATCH_SIMILARITY: NOT_COMPUTED_BEFORE_SOURCE_EDIT; no new patch exists yet.
FULL_LOCAL_SCORE: NOT_COMPUTABLE; current evidence is shape-local only.
LOCAL_SCOPE: diagnostic only; SHAPE_LOCAL_ONLY=YES; never LOCAL_BEST or COMPLETE.

## Execution contract

BUILD_ENTRY: existing V001/support/CMakeLists.txt and V001/support/build
BUILD_COMMAND: source /usr/local/Ascend/ascend-toolkit/set_env.sh; set HCC C++ include variables as recorded in V002-SOURCE-IDENTITY-BUILD.md; cmake --build 本地实验/CROSSROW-EPI-PROLOGUE-X/V001/support/build --clean-first --parallel 1
CORRECTNESS_TARGET: existing crossrow_v001_paired_runner with rows=16,width=6144,blocks=8,dtype=fp16
CORRECTNESS_M1: existing crossrow_v001_paired_runner with rows=1,width=6144,blocks=1,dtype=fp16

STATUS: IN_PROGRESS after this declaration and source-snapshot commit; no Local until Build and Correctness pass.
