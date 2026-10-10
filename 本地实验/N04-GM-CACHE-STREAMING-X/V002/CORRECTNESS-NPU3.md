# N04 V002 NPU3 Correctness Evidence

ROUTE=N04-GM-CACHE-STREAMING-X
REVISION=V002
HOST=hwnput3
DEVICE_ID=3
SHAPE=dtype=fp32 rows=2 width=16384
CANDIDATE_SOURCE=本地实验/N04-GM-CACHE-STREAMING-X/V001/Candidate.asc
CANDIDATE_SHA256=337601ad3b5575b0f669234514af5500849e1d9f6d9df8923a53a0b0e54eb0ca
PARENT_SOURCE=/home/data4t2/lelinfeng/phase4-workspaces/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70a4b183d1eaf5610edf80c2879b15e3

## Entry

The existing V016 `runtime_probe_fp32` target was reused in a temporary directory. The temporary probe source had the CANN 8.5 provider in the build translation unit and had its final `aclrtResetDevice` call removed. `Candidate.asc` was copied unchanged; it contains no provider definition.

NO_RESET_SOURCE=YES
NO_RESET_BINARY=NOT_BUILT
PROVIDER_SOURCE=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/opp/built-in/op_impl/ai_core/tbe/impl/ascendc/3rd/template_linear_algebra/include/template_linear_algebra/catlass.hpp
PROVIDER_FORM=__gm__ struct OpSystemRunCfg g_opSystemRunCfg{Catlass::L2_OFFSET};
TEMP_DIR=/tmp/n04-v001-v016-no-reset.LfUsLE

## Commands And Return Codes

```text
source /usr/local/Ascend/ascend-toolkit/set_env.sh                                      RC=0
export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest
export ASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
export SOC_VERSION=Ascend910B3
cmake -S /tmp/n04-v001-v016-no-reset.LfUsLE -B /tmp/n04-v001-v016-no-reset.LfUsLE/build -DCMAKE_CXX_FLAGS=-DL2_CACHE_HINT
  attempt 1 CONFIGURE_RC=1: source directory had no CMakeLists.txt; BUILD_RC=1
  attempt 2 CONFIGURE_RC=1: existing CMakeLists required runtime_probe_v016.asc; no build
  attempt 3 CONFIGURE_RC=0
cmake --build /tmp/n04-v001-v016-no-reset.LfUsLE/build --target probe_fp32_v016 probe_fp32_v011 --verbose --parallel 1
  BUILD_RC=2: CANN ASC registration compilation failed with fatal error: 'vector' file not found

/tmp/n04-v001-v016-no-reset.LfUsLE/build/probe_fp32_v016 2 16384 3
  EXECUTED=NO; executable was not produced
/tmp/n04-v001-v016-no-reset.LfUsLE/build/probe_fp32_v011 2 16384 3
  EXECUTED=NO; executable was not produced
```

The three build outputs are preserved in `correctness-v001-npu3-attempt1.log`, `correctness-v001-npu3-attempt2-config-missing-runtime-v016.log`, and `correctness-v001-npu3-attempt3.log`.

## Device Snapshot

`npu3-closeout.log` records the final read-only snapshot at `2026-10-10T16:34:16+00:00` through `2026-10-10T16:34:18+00:00`: NPU3 HBM was `3428/65536 MB` (`5%`), AICore was `0%`, and `npu-smi info` reported no running process on NPU3. No device program started, so no device state was changed.

## Result

CORRECTNESS=NONE
CORRECTNESS_STATUS=NOT_RUN
CORRECTNESS_REASON=FP32 probe build failed before either Candidate or Parent command could start; final error was missing `vector` during ASC registration compilation.
LOCAL=NONE
LOCAL_REASON=No executable and no timing sample.
DEVICE_RUN=NONE
PARENT_COMMAND_NOT_RUN=/tmp/n04-v001-v016-no-reset.LfUsLE/build/probe_fp32_v011 2 16384 3
CANDIDATE_COMMAND_NOT_RUN=/tmp/n04-v001-v016-no-reset.LfUsLE/build/probe_fp32_v016 2 16384 3
