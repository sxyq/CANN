# TINY V001 Local Timing Matrix

STATUS: `PRE-REGISTERED; NOT RUN`
REVISION: `V001`
PARENT: `R31B-V011`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
CANDIDATE_SOURCE_SHA256: `4e8abff76486532cab4be6bc6098ea80c048802d285837ad0afb193ff9712024`
LATEST_CORRECTNESS_IDENTITY: `A=40; M=20; B=19; shape=[20,256]; dtype=FP32`
LATEST_CORRECTNESS_EXECUTABLE_SHA256: `08fc6c673b56ea677ed23bcceb73fa8ed1f7b8e39475f49bcb768f447a06e49e`

## Fixed Inputs and Dispatch

Both probes use the same FP32 `[20,256]` x, residual, gamma, and bias values as the V001 Correctness runner. Read the live vector-core count before timing and stop if it differs from the recorded `A=40`; derive `M=max(2,floor(A/2))` and `B=M-1` again. The harness logs A/M/B and the reached ownership condition on each invocation.

| Probe | `availableCoreNum` | `blockCount` | Ownership branch | Per-block FP32 path |
|---|---:|---:|---|---|
| Primary equal-fastform | 40 | 20 | `rowCount==blockCount`, H2 fastform | 20 blocks use `ProcessNarrowMidOverlap` |
| Fallback control | 19 | 19 | `rowCount!=blockCount`, Parent formula | block 0 uses `ProcessSmallFp32ContiguousBatched`; blocks 1-18 use `ProcessNarrowMidOverlap` |

The cap-19 control is analyzed independently and never pooled with cap 40. Only cap 40 bears on H2. These remain research proxies and do not assert any Official workload shape or dispatch.

## Registered Run Order

Run each cap independently; never combine cap 40 and cap 19 samples:

1. Parent `same-parent`: one process, 45 warmups, then two consecutive 21-sample blocks. Stop for this cap if it does not pass.
2. `PRECHECK-A`: six independent Parent-only process runs, each 45 synchronized warmups and 21 single-launch device-event samples.
3. Wait 10 seconds without resetting the device.
4. `PRECHECK-B`: six independent Parent-only process runs with the same command and sample count.
5. Evaluate A and B separately for this cap. Only if both qualify, run `paired`: one process, 45 Parent and 45 Candidate synchronized warmups in alternating P/C order, then four adjacent 21-pair blocks ordered `P/C, C/P, P/C, C/P`.

Each timing sample wraps exactly one launch with `aclrtRecordEvent` start/stop, waits for the stop event, reads `aclrtEventElapsedTime`, and records `steady_clock` wall time secondarily. Allocations and H2D happen once before warmup. There is no allocation, H2D, D2H, or golden comparison in a timed sample. P and C share the same input device allocations and output buffer. After the complete sample matrix, run Parent and Candidate once each, D2H, and compare against the independent FP32 golden calculation at `atol=rtol=1e-4`.

Do not start any timing command without Main's explicit device lease and a fresh device/process/resource read. No lease has been created for this preparation.

Every session receives one unique Run ID. Create its result directory with `mkdir` without `-p`; stop if creation returns nonzero. The runner reserves both output files with exclusive-create semantics and exits before ACL initialization if either path already exists or cannot be created. Never reuse a Run ID or result directory.

## Qualification and Decision

- Run Parent same-binary before PRECHECK-A/B for each cap. A failed same-binary result stops that cap before window qualification or P/C.
- PRECHECK-A and PRECHECK-B are evaluated separately over the six process-level event medians in each block. Each must have `CV<=0.15` and `max/min<=1.30`; otherwise do not run P/C for that cap in this attempt.
- Parent same-binary uses its 42 event samples in one process. It passes only when full-sample `MAD/median<=0.10` and the relative difference between its two block medians is `<=0.10`.
- Primary statistics are per-block `DEVICE_EVENT_US` median, pooled full-sample `MAD/median`, p10/p90, and the four paired block-median deltas `Candidate/Parent-1`. Retain every raw sample; do not trim, winsorize, or remove outliers. Wall time, CV, min/max, and max/min are secondary diagnostics.
- Evaluate cap 40 and cap 19 separately. For cap 40, call a direction stable only if all four paired block deltas have the same nonzero sign. `LOCAL_ACCEPTED` requires four negative deltas and the absolute median paired delta to exceed the measured Parent same-binary block drift. `LOCAL_REJECTED` requires four positive deltas and the absolute median paired delta to exceed that same drift. Mixed direction, zero direction, or a smaller effect is `NEEDS_ONE_MORE_LOCAL`. A failed qualification is retained as such and does not count as Candidate regression.
- Cap 19 is a control only; its result cannot accept/reject H2 and cannot be combined numerically with cap 40.

## Non-timing Build and Identity

Remote run root:

```text
/home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-prep-20261003
```

The already-staged exact sources may be compiled before a timing lease. During CMake configure, two temporary files in `build/` are generated from the byte-identified Parent and Candidate sources with only the class, device-kernel, and host-wrapper identifiers renamed so ASC registers both kernel entries in one executable. The original source files are not rewritten; record the generated-file SHAs alongside the original source SHAs. Build/link does not run the harness:

```bash
cd /home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-prep-20261003
source /usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh
export CPATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/aarch64-target-linux-gnu:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/toolkit/toolchain/hcc/aarch64-target-linux-gnu/include/c++/7.3.0/backward
cmake -S source/support -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --target tiny_v001_timing_harness --parallel 1 --verbose
sha256sum source/submission.asc source/support/parent_submission.asc build/parent_timing_submission.asc build/candidate_timing_submission.asc build/tiny_v001_timing_harness
readelf -h build/tiny_v001_timing_harness
```

## Timing Commands (Lease Required)

After Main grants the timing lease and a fresh resource read passes, create one new result directory for the Run ID. The `mkdir` for the Run ID must succeed; if it fails, stop. Run cap 40 and cap 19 in separate subdirectories and evaluate each cap independently:

```bash
set -e
TIMING_ROOT=/home/data4t2/lelinfeng/cann/server_runs/TINY-FIXED-OVERHEAD-CHAMPION-X/V001/timing-runs
RUN_ID="M1-TINY-V001-D5-$(date -u +%Y%m%dT%H%M%SZ)-01"
RESULT_DIR="$TIMING_ROOT/$RUN_ID"
mkdir "$TIMING_ROOT" 2>/dev/null || test -d "$TIMING_ROOT"
mkdir "$RESULT_DIR" || exit 1
mkdir "$RESULT_DIR/cap40" || exit 1
mkdir "$RESULT_DIR/cap19" || exit 1
build/tiny_v001_timing_harness 5 40 same-parent "$RESULT_DIR/cap40/same-parent"
```

Run the next block in the same shell only after `cap40/same-parent-stats.tsv` reports `same_binary_pass=PASS`:

```bash
for rep in 01 02 03 04 05 06; do build/tiny_v001_timing_harness 5 40 precheck "$RESULT_DIR/cap40/precheck-A-${rep}"; done
sleep 10
for rep in 01 02 03 04 05 06; do build/tiny_v001_timing_harness 5 40 precheck "$RESULT_DIR/cap40/precheck-B-${rep}"; done
```

Run P/C only after separately confirming that both cap-40 PRECHECK blocks meet the registered thresholds:

```bash
build/tiny_v001_timing_harness 5 40 paired "$RESULT_DIR/cap40/paired"
```

Cap 19 has the same three-stage order, independently of cap 40. First Parent same-binary:

```bash
build/tiny_v001_timing_harness 5 19 same-parent "$RESULT_DIR/cap19/same-parent"
```

Only after `cap19/same-parent-stats.tsv` reports `same_binary_pass=PASS`, collect its PRECHECK-A/B:

```bash
for rep in 01 02 03 04 05 06; do build/tiny_v001_timing_harness 5 19 precheck "$RESULT_DIR/cap19/precheck-A-${rep}"; done
sleep 10
for rep in 01 02 03 04 05 06; do build/tiny_v001_timing_harness 5 19 precheck "$RESULT_DIR/cap19/precheck-B-${rep}"; done
```

Run cap-19 P/C only if its A and B blocks separately qualify:

```bash
build/tiny_v001_timing_harness 5 19 paired "$RESULT_DIR/cap19/paired"
```

Capture the required before/after device, HBM, AICore, process, disk, and lease state around the eventual measurement session. This registration itself collected no timing samples and created no lease.

The executable previously built at `timing-prep-20261003/build/tiny_v001_timing_harness` (SHA256 `c85ec6ec3a248b565cbc758b56cd98a616faa0fb78cda3b04272cd2c29027131`) contains the removed device-reset call and must not be run. Reset-free Build/Link evidence for the current support source is in `timing-harness-build-20261003T132344Z.md`; its executable has not been run.
