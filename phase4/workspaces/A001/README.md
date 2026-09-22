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

## V005 focused update

V005 used the V003 streamed layout and moved the RMS reciprocal calculation
outside the output chunk loop. The online result remained 15/15 at `25.97`,
with testcase 14 at `124365 us` and testcase 15 at `11955 us`; this did not
show a confirmed latency improvement over the prior control.

The V005 target was compiled on `cann-server3` with CANN `8.5.0.alpha002` and
`dav-2201`. Logs are `build/configure-v005.log` and `build/compile-v005.log`.

## V006 focused update

V006 targets the long-D streamed path after the V005 result. The five queues
are single-slot because every transfer is consumed immediately by a matching
`DeQue`, so there is no cross-tile overlap in this implementation. The freed
workspace is spent on larger streamed chunks: `4096` half/bfloat16 elements
and `3072` FP32 elements. Row ownership, the two-pass FP32 reduction, aligned
`DataCopy`/tail `DataCopyPad` selection, and the direct-invocation ABI remain
unchanged.

The V006 target was compiled on `cann-server3` with CANN `8.5.0.alpha002` and
`dav-2201`. Logs are `build/configure-v006.log` and `build/compile-v006.log`.

V006 online validation remained 15/15 with an official score of `27.30`.
Testcase 14 measured `125075.65 us` and testcase 15 measured `10677.60 us`.

## V007 focused update

V007 keeps the V006 single-slot queues and streamed chunk sizes. It removes
the temporary FP32 `xFloat_` buffer by converting `x` directly into the
existing `u` buffer before adding the residual. This reduces one intermediate
buffer and one local write/read pair per streamed chunk without changing row
ownership, chunk scheduling, arithmetic precision, or the submission ABI.

The V007 target was compiled on `cann-server3` with CANN `8.5.0.alpha002` and
`dav-2201`. Logs are `build/configure-v007.log` and `build/compile-v007.log`.

V007 online validation remained 15/15 with an official score of `27.24`.
Testcase 14 measured `124964.64 us` and testcase 15 measured `10653.88 us`.

## V008 focused update

V008 keeps the V007 single-slot queues and removes the separate FP32
`residualFloat_` allocation. `LoadU` temporarily reuses `squareFloat_` for
residual conversion; the square buffer is consumed only after `LoadU` returns
in the reduction stage. The recovered workspace increases streamed chunks to
`5120` half/bfloat16 elements or `4096` FP32 elements, reducing long-row tile
iterations without changing the row mapping, arithmetic order, or ABI.

The V008 target was compiled on `cann-server3` with CANN `8.5.0.alpha002` and
`dav-2201`. Logs are `build/configure-v008.log` and `build/compile-v008.log`.

V008 online validation remained 15/15 with an official score of `28.10`.
Testcase 14 measured `118183.70 us` and testcase 15 measured `11402.05 us`.

## V009 focused update

V009 adds an architecture branch for wide FP32 rows (`D >= 16384`). The first
pass retains the exact FP32 `u = x + residual` tiles in the output GM buffer;
the second pass reads those retained tiles before applying RMS normalization,
gamma, and bias. This avoids rereading `x` and `residual` and repeating their
conversion/addition for the wide-D case. Half, bfloat16, and narrower FP32
rows retain the V008 path, so their precision behavior is unchanged.

The V009 target was compiled on `cann-server3` with CANN `8.5.0.alpha002` and
`dav-2201`. Logs are `build/configure-v009.log` and `build/compile-v009.log`.

## V010 focused update

V010 is an architecture-level rewrite from V008 (V009 is not the base). It
removes the second GM read of `x`/`residual` by keeping the FP32 `u` slice in
UB across reduction and epilogue, caches `gamma`/`bias` in UB when they fit,
and adds 2D row/column decomposition with an output-GM partial reduce when
`R < core_count` so wide-D few-row shapes can use the full Vector Core count.

The V010 target was compiled on `cann-server3` with CANN `8.5.0.alpha002` and
`dav-2201`. Logs are `build/configure-v010.log` and `build/compile-v010.log`.

## V011 focused update

V011 keeps the V010 architecture and adds a `FastKernel` for small totals
(`D <= 8192 && R * D <= 262144`) so tiny shapes skip residency setup, and
tightens `ResidentKernel` to one `GetValue` per row with double-buffered
x/res loads. Target: recover T01/T02/T05 and cut T14/T08 per-row overhead.

The V011 target was compiled on `cann-server3` with CANN `8.5.0.alpha002` and
`dav-2201`. Logs are `build/configure-v011.log` and `build/compile-v011.log`.

## V012 focused update

V012 repairs the V011 FastKernel (queue-synced gamma/bias, UB size check that
falls back when `D` is too wide for one-shot buffers) and uses the V010
ResidentKernel as the safe path for every other shape. Goal: 15/15 at or above
the V010 score while keeping the small-case speed that T02/T03/T04 showed.

The V012 target was compiled on `cann-server3` with CANN `8.5.0.alpha002` and
`dav-2201`. Logs are `build/configure-v012.log` and `build/compile-v012.log`.

## V013 focused update

V013 keeps the V012 split and fixes the T04 param-reload regression: FastKernel
loads gamma/bias once per core through the param queue and reuses them, and the
element gate is widened to 2M with the UB footprint still enforced. ResidentKernel
adds one GetValue per row and double-buffered x/res loads for T14/T08.

The V013 target was compiled on `cann-server3` with CANN `8.5.0.alpha002` and
`dav-2201`. Logs are `build/configure-v013.log` and `build/compile-v013.log`.
