# WIDE-UB-RESIDENCY-X V006

ROUTE: WIDE-UB-RESIDENCY-X
REVISION: V006
PARENT: 本地实验/WIDE-UB-RESIDENCY-X/V005/submission.asc
PARENT_SOURCE_COMMIT: `42650fc80a08bdbc61dda7ae280c903c615644c2`
BRANCH: route/new-n02-wide-ub-residency-x-20261016

## HYPOTHESIS

In `ProcessWideLowPrecision` Pass2, every output row/tile executes
`SyncVToMTE2()` immediately before its output store. The same tile's
gamma/bias values occupy one of two staging slots backed by `xBuf_` and
`residualBuf_`. A slot's `V_MTE2` release event is signaled after all rows in
the tile have consumed those values, and the slot is waited before a later
parameter load reuses it. This slot-specific event may make the per-row
global V-to-MTE2 handshake unnecessary. The separate `SyncVToMTE3()` before
the output store remains in place.

Primary target: FP16, rows=2, D=32768. The existing UB planner selects a
4096-element tile for FP16 at this width, yielding eight tiles per row and 16
per-row/tile `SyncVToMTE2()` calls over the two-row launch. Each helper expands
to one `SetFlag` plus one `WaitFlag`, so the change removes 32 event operations.
GM payload and UB allocation are unchanged. No tail-specific branch is added.

## ONE CHANGE

Remove only the per-row/tile `SyncVToMTE2()` from Pass2 in
`submission.asc`. Keep Pass1 release waits, Pass2 `V_MTE2` slot events,
`SyncVToMTE3()`, `Store()`, and `SyncMTE3ToV()` unchanged. V005's BF16
conversion change is retained without alteration.

## ACCESS EVIDENCE

- Pass1 loads `x` and `residual` from GM into the two staging buffers, then
  waits for both outstanding `V_MTE2` releases before Pass2 uses those same
  buffers for `gamma` and `bias`.
- Pass2 loads the next parameter tile into the alternate slot while applying
  the current tile. Before either slot is reused, the code waits on its
  corresponding `prel0` or `prel1` event. After all batch rows consume a tile,
  it signals that slot's `V_MTE2` release event.
- The removed helper expands to `SetFlag<V_MTE2>(0)` and
  `WaitFlag<V_MTE2>(0)`. Output transfer keeps its distinct `V_MTE3` event
  before `Store`, followed by `MTE3_V` before output-buffer reuse.
- V005 removes a repeated BF16 `ToFloat` pair. V006 changes only the
  synchronization path; it does not alter conversion work, logical GM bytes,
  addresses, tile selection, or UB capacity.

## COMPILE / DEVICE

SERVER: `cann-server3` (`hwnput3`)
CANN: 8.5.0.alpha002; sourced
`/usr/local/Ascend/ascend-toolkit/set_env.sh` and used the existing paired
CMake project and system/toolkit C++ include paths.
CONFIGURE: `cmake -S .../本地实验/R31B/V016/support/paired -B .../build`; RC=0.
BUILD: `cmake --build .../build --target paired_runner_v016 -j2`; RC=0;
`[100%] Built target paired_runner_v016`.
COMPILE: PASS.
RUNTIME_LINK: `ldd` resolved `libgraph.so` and all listed shared libraries.
EVIDENCE: `configure.log`, `compile.log`, and `ldd.log`.
REMOTE_BUILD_DIR: `/tmp/wide-ub-residency-x-v006-build.bh1s3Ux7` (retained for
the future allocated run).

CORRECTNESS: NOT_RUN.
LOCAL_SCORE: NOT_MEASURED.
DEVICE: NOT_RUN; request Main allocation before the V006 paired run.
DEVICE_RESET: NO.
PUSH: NO.
