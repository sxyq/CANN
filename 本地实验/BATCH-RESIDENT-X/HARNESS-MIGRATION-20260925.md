# BATCH-RESIDENT-X timing harness migration — unified device-event protocol (2026-09-25)

Measurement-layer migration only. Candidate kernel `submission_v001.asc` and all `.cpp`
compute sources are untouched; source SHA-256
`ad961c5837971bf989c9989b822cc2719a6a285c60969d35e1530463b7c8721d` verified unchanged
before and after this migration (re-verified locally and on server3). No new Revision.

Authority: `phase4/control/local-timing-protocol.md` (unified measurement protocol);
`phase4/control/execution-contract.md` sections F (local-first flow) and R
(long-horizon parallel exploration — Track-A keeps the Candidate unchanged).

## What changed

Legacy harness: `phase4/workspaces/BATCH-RESIDENT-X/timing_probe.asc` — CPU
steady_clock wall-clock around launch + `aclrtSynchronizeStream`, warmup fixed at 5,
21 samples, fixed 6-arg CLI, only a median row written (no per-sample dump), no
device events. Protocol lists this as wall-clock with no per-sample dump. All numbers
produced by it are now tagged LEGACY_TIMING_METHOD (see the tags below; data retained).

New harness, ported from the SCHED reference implementation
(`cann-next6/SCHED-ROWGROUP-X/phase4/workspaces/SCHED-ROWGROUP-X/support/runner_ref.inc`
+ `run_ref_validation.sh`) into BATCH's own workspace:

```text
phase4/workspaces/BATCH-RESIDENT-X/support/
  runner_ref.inc        measurement layer (port of SCHED runner_ref.inc)
  runner_ref_batch.asc  #define SRX_SUBMISSION "submission_v001.asc" + include
  local_types.h         TensorInfo / TensorGroupInfo structs
  CMakeLists.txt        brx_ref_probe target (bisheng, Ascend910B3, dav-2201)
  run_ref_smoke.sh      one-shot functional smoke wrapper
```

The legacy `timing_probe.asc` is left in place, unmodified, as historical reference.

Port differences vs the SCHED reference (deliberate, route-specific):

- Golden check after the timed loop uses the same input generation and golden model
  as BATCH's own `npu_correctness.cpp` (AddRMSNormBias semantics, same tolerances),
  so the post-timing D2H result agrees with the route's correctness probe.
- `stats.txt` additionally records `route BATCH-RESIDENT-X`.
- Everything else follows the reference: same lifecycle, event placement, stats set,
  raw dump format, and argc contract.

## Mapping to protocol clauses

| Protocol clause | Where implemented |
|---|---|
| Warmup default ≥10, once per process | `runner_ref.inc` warmup loop; `warmup` CLI arg; smoke uses 10 |
| ≥11 in-process samples (prefer 21); blocks in one process | `samples`/`blocks` CLI args; smoke uses 21 samples × 1 block; no `aclFinalize` between blocks |
| Device events primary, per single launch, sync before reading | `aclrtRecordEvent(evStart)` → 1..batch_n launches → `aclrtRecordEvent(evStop)` → `aclrtSynchronizeEvent(evStop)` → `aclrtEventElapsedTime` → `device_us` |
| Wall-clock recorded as secondary, judge on device_us | `host_wall_us` steady_clock column alongside; `method DEVICE_EVENT_PRIMARY+HOST_WALL_SECONDARY` in stats |
| Warmup sync after each launch; no sample read with launches in flight | `aclrtSynchronizeStream` per warmup; per-sample event wait + stream sync before writing the row |
| Alloc once per process, outside timed loop | five `aclrtMalloc` calls before warmup; none inside the sample loop |
| H2D before warmup; D2H only after timed loop | H2D block before warmup; single D2H after all blocks, then golden compare |
| One device per run, no concurrent timing on leased devices | device is CLI arg; smoke ran on d5 without claiming any lease; d4 untouched (SCHED lease active) |
| Per-sample dump + jitter stats | `*-raw.tsv` (block/rep/device_us/host_wall_us per sample); `*-stats.txt` median/mean/stdev/MAD/p10/p90/min/max/abs_spread/CV/max_min per block and combined |
| Repeat-batch optional, default N=1 | optional 11th arg `batch_n`, default 1 (protocol: REPEAT_BATCH not adopted) |
| argc contract: 10 args, or 11 with batch_n | `argc != 10 && argc != 11` → usage + exit 2 |
| Correctness never interleaved with timing | D2H + golden compare run once after all blocks |

## Exact command lines

Build (on server3; local machine has no NPU toolchain):

```bash
# sources synced to /home/data4t2/lelinfeng/BATCH-RESIDENT-X_runs/ref-harness/
cd /home/data4t2/lelinfeng/BATCH-RESIDENT-X_runs/ref-harness/support
mkdir -p build && cd build
export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward
cmake ..
cmake --build . -j1        # produces brx_ref_probe
```

Note: server3 is aarch64, so `CPLUS_INCLUDE_PATH` uses the
`/usr/include/aarch64-linux-gnu/c++/11` equivalent of the x86_64 path in the task
brief. `ASCEND_HOME_PATH` must be the toolkit root (the bisheng plugin appends
`aarch64-linux/ascc/lib64` itself); with it unset the plugin cannot load and
`acl/acl.h` is not found.

Runner CLI (identical to the reference contract):

```text
brx_ref_probe device rows width dtype result_prefix warmup samples blocks gap_sec [batch_n]
```

Functional smoke (what was run; plumbing only, no lease, no performance recording):

```bash
cd /home/data4t2/lelinfeng/BATCH-RESIDENT-X_runs/ref-harness/support
export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002
ASCEND_DEVICE_ID=5 bash run_ref_smoke.sh
# equivalent to:
# ./build/brx_ref_probe 5 8 256 0 results-smoke/d5/smoke-d5-r8x256-dt0 10 21 1 0
```

## Smoke result (2026-09-25, server3 d5)

- Exit code 0; runner printed `REF_HARNESS route=BATCH-RESIDENT-X ... warmup=10
  samples=21 blocks=1 batch_n=1 bad=0`.
- `*-raw.tsv`: header `block rep device_us host_wall_us` + 21 sample rows; all 21
  `device_us` values present and positive (device-event timestamps flowing).
- `*-stats.txt`: `method DEVICE_EVENT_PRIMARY+HOST_WALL_SECONDARY`, `warmup 10`,
  `samples_per_block 21`, `batch_n 1`, `bad 0` (post-timing golden compare passed).
- Evidence copied to `phase4/local/BATCH-RESIDENT-X/harness/smoke-d5/`;
  build logs in `phase4/local/BATCH-RESIDENT-X/harness/build-logs/`.
- This was a plumbing run only: no timing lease claimed, no performance numbers
  recorded or judged, no P/C, no qualification. d4 was not touched.

## LEGACY_TIMING_METHOD tags

Prior BATCH wall-clock numbers are tagged, not deleted:

- `phase4/local/BATCH-RESIDENT-X/V001/LEGACY-TIMING-METHOD.md` (root tag, also
  marks `local-result.json`)
- `phase4/local/BATCH-RESIDENT-X/V001/support/results-window-qual/LEGACY-TIMING-METHOD.md`
- `phase4/local/BATCH-RESIDENT-X/V001/support/results-round2/LEGACY-TIMING-METHOD.md`
- summary JSONs (`local-result.json`, `gates-summary.json`, `round2-summary.json`)
  gained an additive `timing_method: LEGACY_TIMING_METHOD` key.

Legacy and device-event samples must never be merged in one median.
