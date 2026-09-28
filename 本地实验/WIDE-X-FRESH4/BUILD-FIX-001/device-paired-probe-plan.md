# WIDE-X-FRESH4 Device Paired Probe Plan

Status: PREPARED_NOT_RUN. Wait for Main to grant an uncontested device window.

## Scope

- Route/revision: WIDE-X-FRESH4 / BUILD-FIX-001
- Context: FRESH_BLIND
- Candidate source SHA: 5d0ee01165e46a281cbb7d1605feba3605ad59883845400a063e97704f27f2be
- Declared direct parent: CURRENT
- Parent source SHA: bc4608dcb1028b61de50fc8a74d4a72661945c02c3caccb8b60dd543bdf6677c
- No candidate, CMake, existing runner, scheduler, or online submission changes are in scope.

## Execution Preconditions

Do not start device execution before Main grants the device and window. At that point, collect five load snapshots at 10-second intervals. The selected device must show no unrelated compute process and no sustained AICore activity in all five snapshots. Record HBM use and every listed process; do not stop or alter another process.

The current route runner selects device 4, but the probe device must be the one Main grants. Keep that device fixed for both artifacts and all paired cases. If the window becomes occupied or the scheduler owner changes, stop without timing and report the corresponding status.

## Artifacts And Comparability

The candidate has recorded build/link PASS and NPU correctness PASS for FP32, FP16, and BF16 at widths 2048, 16384, and 32768. The declared CURRENT parent has no recorded NPU build, link, or correctness result and no ready control executable in the route evidence. Its source still contains the device `sqrtf` call and `size_t` DataCopyPad lengths that BUILD-FIX-001 addressed. Its FP32 `CAST_RINT` path is also known to fail correctness.

Before any timing, require a runnable direct-parent control and correctness PASS for each timed case using the same executable input and API path. Do not use a compatibility-transformed parent as though it were the unmodified CURRENT source. FP32 parent cases remain excluded unless the declared control passes correctness without changing the candidate, CMake, or existing runner. If a comparable parent executable cannot be prepared within those boundaries, stop and report CONTROL_NOT_READY; do not fall back to candidate-only timing.

## Cases And Pair Order

Use the existing correctness runner's rows=2, dtype, width, deterministic input generation, epsilon, and availableCoreNum values. Eligible cases are the existing grid:

| Dtype | Widths |
|---|---|
| FP32 | 2048, 16384, 32768; time only if CURRENT correctness passes |
| FP16 | 2048, 16384, 32768 |
| BF16 | 2048, 16384, 32768 |

For every eligible case, use identical input bytes and one fixed stream. Run three warmups per artifact. Collect 12 paired blocks, alternating order CURRENT->BUILD-FIX-001 and BUILD-FIX-001->CURRENT. Each sample should time a batch of repeated kernel launches with device events and divide elapsed device time by the launch count. Keep allocation, host reference work, and output comparison outside the timed event interval.

Correctness must pass independently on both artifacts before recording their timing for that case. Save each raw sample and the exact launch count. Report each side's median, p10, p90, and median absolute deviation; report paired latency ratios and their median absolute deviation as the jitter summary. Do not infer an improvement from contaminated samples.

## Load Record

Capture before and after each case, and between paired blocks at least every three pairs:

- Timestamp and selected device ID.
- Full `npu-smi info` output.
- `npu-smi info -t usages -i <device>` with HBM use, HBM bandwidth, AICore, and AIVector utilization.
- `ps -eo pid,ppid,user,pcpu,pmem,etime,args` filtered for VLLM/python and the NPU process table from `npu-smi`.
- Window owner/grant and whether any unrelated process appeared during the block.

Mark a block LOAD_CONTAMINATED if another process appears, AICore activity is sustained, or load changes materially across its paired samples. Preserve the raw timing and load data but make no performance claim for that block.

## Current State

No device snapshots or timing were collected for this plan. Existing host-shim timing is not device performance. Decision remains NEEDS_ONE_MORE_LOCAL. Online submission is prohibited.
