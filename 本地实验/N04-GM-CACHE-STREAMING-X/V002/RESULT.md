# N04 V002 Result

ROUTE=N04-GM-CACHE-STREAMING-X
REVISION=V002
PARENT=V001, commit 8f03481980d3832a0734587a534b3dfac8122d99
PERFORMANCE_ANCHOR=R31B V011 / Official 45.16
CHANGE=Build-entry-only wiring of the installed CANN 8.5 Catlass provider.
CANDIDATE=V001 Candidate.asc reused unchanged; Candidate contains no g_opSystemRunCfg definition.

## Provider

CANN 8.5 sample source:
`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/opp/built-in/op_impl/ai_core/tbe/impl/ascendc/3rd/template_linear_algebra/include/template_linear_algebra/gemm/kernel/grouped_matmul_slice_m.hpp`

The sample defines `__gm__ struct OpSystemRunCfg g_opSystemRunCfg{Catlass::L2_OFFSET};`. Its `Catlass::L2_OFFSET` value is `0` in the adjacent CANN 8.5 `catlass.hpp`. V002 applies that existing provider to the two translation units in the reused R31B V016 compile entry. The Candidate source is not edited.

## Attempts

The first V002 attempt recorded two failed patch hunks at line 27. The script continued to configure and build the unpatched copy: environment load RC=0, configure RC=0, device and submission compilation passed, then `full_link` failed with undefined `g_opSystemRunCfg`; overall build RC=2. The failed attempt remains in `compile-v002.log`.

The corrected patch dry run returned 0. Applying it returned 0; both hunks applied at line 27 with fuzz 2. Remote source output showed the provider include and definition at lines 30-31 in both `device.cpp` and `adapter.cpp`.

## Successful Compile

Host=`hwnput3`; CANN=`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`; SoC=`Ascend910B3`.

Commands and results:

```sh
source /usr/local/Ascend/ascend-toolkit/set_env.sh
# RC=0
export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest
export ASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
export SOC_VERSION=Ascend910B3
cmake -S /tmp/n04-gm-cache-streaming-v002.TBu8zR -B /tmp/n04-gm-cache-streaming-v002.TBu8zR/build -DCMAKE_CXX_FLAGS=-DL2_CACHE_HINT
# RC=0
cmake --build /tmp/n04-gm-cache-streaming-v002.TBu8zR/build --target device submission full_link --verbose --parallel 1 -- -B
# RC=0; device PASS, submission PASS, full_link PASS
nm -u /tmp/n04-gm-cache-streaming-v002.TBu8zR/build/CMakeFiles/device.dir/device.cpp.o
nm -u /tmp/n04-gm-cache-streaming-v002.TBu8zR/build/device.alink
nm -u /tmp/n04-gm-cache-streaming-v002.TBu8zR/build/submission.alink
# All three commands produced no unresolved-symbol output.
```

Compile evidence: `本地实验/N04-GM-CACHE-STREAMING-X/V002/compile-v002.log`.

## Device And Local Results

DEVICE_RUN=NONE
CORRECTNESS=NONE
CORRECTNESS_STATUS=NOT_RUN
CORRECTNESS_REASON=The existing V016 FP32 probe build failed before either Candidate or Parent could start. Attempt 3 returned BUILD_RC=2 with fatal error: 'vector' file not found.
LOCAL_SCORE=NONE
LOCAL_REASON=No executable and no timing sample.
DEVICE_ID=3
DEVICE_REASON=No device program was started; NPU3 was sampled read-only before and after the failed build.
DEVICE_PRE=HBM 3428/65536 MB (5%); AICore 0%; no running process.
DEVICE_POST=HBM 3428/65536 MB (5%); AICore 0%; no running process.
NO_RESET_BINARY=NOT_BUILT
EVIDENCE=CORRECTNESS-NPU3.md; correctness-v001-npu3-attempt1.log; correctness-v001-npu3-attempt2-config-missing-runtime-v016.log; correctness-v001-npu3-attempt3.log; npu3-closeout.log

## Next

Preserve the build failure. Do not add a Runner or alter Candidate.asc.
