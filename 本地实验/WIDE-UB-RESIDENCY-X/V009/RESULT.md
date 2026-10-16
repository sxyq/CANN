# WIDE-UB-RESIDENCY-X V009

RESEARCH_TRACK: N02 CODE-FOOTPRINT-ICACHE-X
REVISION: V009
PARENT: `本地实验/WIDE-UB-RESIDENCY-X/V007/submission.asc`
PARENT_SOURCE_COMMIT: `bd2e5f7f`
BRANCH: `route/new-n02-wide-ub-residency-x-20261016`

## HYPOTHESIS

`Process()` has one call site for `ProcessWideLowPrecision()`. That helper is
declared `__aicore__ inline` and contains both full tiled passes, row reduction,
parameter staging, and output stores. Keeping this large body inline may enlarge
the generic dispatch function and increase instruction-cache competition with
other shape paths. Marking only `ProcessWideLowPrecision()` `noinline` may keep
the dispatch body smaller while leaving the helper's arithmetic and data flow
unchanged. The tradeoff is one outlined call on the wide low-precision path.

This is a code-layout hypothesis. It does not predict fewer vector operations,
GM copies, synchronization events, UB bytes, or blocks.

## ONE CHANGE

Add `__attribute__((noinline))` to the existing `ProcessWideLowPrecision()`
declaration. No body, dispatch condition, arithmetic, buffer, synchronization,
tile, or block-count change is included.

V006 changes the Pass2 V-to-MTE2 synchronization path. V007 reuses the BF16
gamma staging slot for output and changes the BF16 tile budget. V008 changes
the block assignment for two-row D=12288 cases and remains uncompiled and
unrun. V009 is copied from V007, so the unvalidated V008 block change is not
included.

## SOURCE EVIDENCE

- `Process()` calls `ProcessWideLowPrecision()` once in its wide low-precision
  branch; the other shape paths remain in the same dispatch method.
- `ProcessWideLowPrecision()` is a large helper containing Pass1 and Pass2 tile
  loops, row reductions, parameter loads, and output stores.
- The other existing member helpers also use `__aicore__ inline`; this change
  tests one large helper rather than changing all helper annotations.
- The only V009 source difference from V007 is the function attribute.
- V006 and V007 passed correctness, while every paired Local P10–P90 interval
  crossed zero; the function-layout effect remains a separate unmeasured
  hypothesis.

## COMPILE / DEVICE

SERVER: `cann-server3` (`hwnput3`).
CANN: 8.5.0.alpha002.
RUNNER: existing `paired_runner_v016`; the candidate include and CMake source
directory both point to V009.
REMOTE_BUILD_DIR: reused `/tmp/wide-ub-residency-x-v006-build.bh1s3Ux7/build`.
CONFIGURE_COMMAND: `cmake -S /tmp/wide-ub-residency-x-v006-build.bh1s3Ux7/本地实验/R31B/V016/support/paired -B /tmp/wide-ub-residency-x-v006-build.bh1s3Ux7/build -DCMAKE_BUILD_TYPE=Release -DR31B_V016_SOURCE_DIR=/tmp/wide-ub-residency-x-v006-build.bh1s3Ux7/本地实验/WIDE-UB-RESIDENCY-X/V009`; RC=0.
BUILD_COMMAND: `cmake --build /tmp/wide-ub-residency-x-v006-build.bh1s3Ux7/build --target paired_runner_v016 --parallel 1 -- -B`;
RC=0; `[100%] Built target paired_runner_v016`.
COMPILE: PASS.
RUNTIME_LINK: `ldd` resolves system `libstdc++.so.6` and CANN libraries;
evidence is in `ldd.log`. CMake emitted its existing RPATH ordering warning.
EVIDENCE: `configure.log`, `compile.log`, and `ldd.log`.

RUNNER_STARTED: NO.
CORRECTNESS: NOT_RUN BY REQUEST.
LOCAL: NOT_RUN BY REQUEST.
DEVICE: NOT_REQUESTED.
V008: NOT_COMPILED; NOT_RUN.
PUSH: NO.

The existing `paired_runner_v016` CMake target remains the compile entry. After
compilation, the device entry remains `paired_runner_v016 --paired --device N`
with the existing CANN environment; this task does not start the Runner.
