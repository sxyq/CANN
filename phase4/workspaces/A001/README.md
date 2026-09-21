# A001 Row-Resident Fused Streaming

This workspace contains the A001 `AddRmsNormBias` Ascend C kernel for
Ascend910B3 / DAV_2201. Each Vector Core owns a contiguous range of complete
rows. The D dimension is never split between cores.

## Build

The compile was run on `cann-server3` with CANN `8.5.0.alpha002`:

```sh
source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh
export ASCEND_CANN_PACKAGE_PATH="$ASCEND_HOME_PATH"
export CPLUS_INCLUDE_PATH="$ASCEND_HOME_PATH/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:$ASCEND_HOME_PATH/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:$ASCEND_HOME_PATH/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward"
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_PREFIX_PATH="$ASCEND_HOME_PATH/aarch64-linux/tikcpp/ascendc_kernel_cmake" \
  -DCMAKE_MODULE_PATH="$ASCEND_HOME_PATH/aarch64-linux/tikcpp/ascendc_kernel_cmake/ASC_CMake"
cmake --build build -j2
```

The captured logs are `build/configure.log` and `build/compile.log`.

## Direct-invocation submission

`submission.asc` is the A001-only direct-invocation adapter. It derives the
flattened row count and last dimension from `TensorGroupInfo`, selects a
participating-core count, and launches the row-resident device kernel with
scalar launch metadata. The base implementation remains in
`op_kernel/add_rms_norm_bias_kernel.asc`; the adapter does not alter it.

The adapter was compiled on `cann-server3` with the same toolchain. The
captured records are `build/configure-adapter.log` and
`build/compile-adapter.log`.

## V003 focused update

The V002 result recorded 124949.87 us for testcase 14 and 12187.19 us for
testcase 15; testcase 14 was the dominant long-D latency case. `submission_v003.asc`
keeps `DataCopyPad` for partial or unaligned transfers, while complete transfers
whose GM byte offset and length are both 32B aligned use `DataCopy`. This avoids
the padding path on the majority of full chunks without changing row ownership,
dtype conversion, or the reduction algorithm.

The V003 target was compiled on `cann-server3` with CANN `8.5.0.alpha002` and
`dav-2201`. Logs are `build/configure-v003.log` and `build/compile-v003.log`.

## V004 focused update

V003 measured `118631.29 us` for testcase 14 and `11909.16 us` for testcase
15; testcase 14 represented `89.17%` of the measured 15-case total. V004
removes the separate FP32 `xFloat_` buffer: `x` is converted directly into the
already-required `u` work buffer before adding the residual. The freed UB is
spent on larger streamed chunks (`3584` half/bfloat16 elements and `2560`
FP32 elements), reducing chunk-loop and transfer setup work on long rows.

The V004 target was compiled on `cann-server3` with CANN `8.5.0.alpha002` and
`dav-2201`. Logs are `build/configure-v004.log` and `build/compile-v004.log`.
