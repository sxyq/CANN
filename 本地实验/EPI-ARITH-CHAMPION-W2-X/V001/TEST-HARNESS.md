# EPI-ARITH-CHAMPION-W2-X V001 Build and Correctness Harness

## Scope and Preconditions

- ROUTE / REVISION: `EPI-ARITH-CHAMPION-W2-X` / `V001`.
- Candidate SHA-256: `9a28f5cd8703dc4ff5c46fba09f59a537132594e5a879a50a17349086bfe4d59`.
- Direct Parent: R31B V011, SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- CANN environment assumes toolkit `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002` and `--npu-arch=dav-2201` for Ascend 910B3, as encoded by the build script.
- Before running, Main must assign device 7 and synchronize this committed Route tree plus `线上结果/R31B/V011/submission.asc` to `/home/data4t2/lelinfeng/cann`. The scripts reject either source if its SHA differs. This harness does not fetch branches, allocate a device lease, or submit Online.
- Use one fresh UTC `EPI_RUN_ID` for both commands. The build script refuses an existing run directory; the correctness script refuses an existing correctness output directory.
- Both commands record pre/post `npu-smi info`, process lists, `df -h`, and `du -sh` in their main logs. They stop before building or running correctness when available disk is below 15 GiB. The correctness runner finalizes its ACL context and never resets device 7.

## Exact Commands

Run from a terminal that can use the configured `cann-server3` SSH alias. These commands are documented only; this preparation did not connect to server3 or run them.
The per-run output directory is `/home/data4t2/lelinfeng/cann/server_runs/EPI-ARITH-CHAMPION-W2-X/V001/${RUN_ID}/`.

```bash
RUN_ID="$(date -u +%Y%m%dT%H%M%SZ)"
ssh cann-server3 "cd /home/data4t2/lelinfeng/cann && EPI_RUN_ID=${RUN_ID} bash 本地实验/EPI-ARITH-CHAMPION-W2-X/V001/support/build_server3.sh"
```

Build log:
`/home/data4t2/lelinfeng/cann/server_runs/EPI-ARITH-CHAMPION-W2-X/V001/${RUN_ID}/logs/build.log`

After build completes successfully, run correctness separately:

```bash
ssh cann-server3 "cd /home/data4t2/lelinfeng/cann && EPI_RUN_ID=${RUN_ID} DEVICE_ID=7 bash 本地实验/EPI-ARITH-CHAMPION-W2-X/V001/support/correctness_server3.sh"
```

Correctness orchestration log:
`/home/data4t2/lelinfeng/cann/server_runs/EPI-ARITH-CHAMPION-W2-X/V001/${RUN_ID}/logs/correctness-matrix.log`

Each runner invocation has a separate log, for example:
`/home/data4t2/lelinfeng/cann/server_runs/EPI-ARITH-CHAMPION-W2-X/V001/${RUN_ID}/logs/correctness-parent-r2-d12288-fp32.log`

Each invocation writes one result TSV, for example:
`/home/data4t2/lelinfeng/cann/server_runs/EPI-ARITH-CHAMPION-W2-X/V001/${RUN_ID}/correctness/candidate-r2-d12288-fp32.tsv`

The matrix summary is:
`/home/data4t2/lelinfeng/cann/server_runs/EPI-ARITH-CHAMPION-W2-X/V001/${RUN_ID}/correctness/summary.tsv`

The matrix runs Parent and Candidate once per shape, for 14 runner invocations and 14 individual command logs. The orchestration log contains pre/post resource snapshots and the command matrix. It does not create events or take timing samples.

## Coverage

All cases use `rows=2`, `dtype=FP32`, `blockCount=1` (`availableCoreNum=1` passed to both exact-source entry points), device 7, epsilon `1e-5`, and deterministic input values in approximately `[-5,5]`.

| Width D | Purpose | V011 path / source-derived batch |
|---:|---|---|
| 8192 | Exact threshold control; Candidate arithmetic edit is unreachable | Existing non-wide FP32 path; `batchRows=NA` |
| 8193 | First width above threshold; tail and three tiles | Wide FP32 full-cache, effective `batchRows=2` |
| 12288 | Selected H3 target | Wide FP32 full-cache, effective `batchRows=2` |
| 16384 | Active two-row batch at four tiles | Wide FP32 full-cache, effective `batchRows=2` |
| 18416 | Two-row UB budget boundary | Wide FP32 full-cache, effective `batchRows=2` |
| 18417 | Immediately beyond two-row UB budget | Wide FP32 full-cache, effective `batchRows=1` per batch |
| 32768 | Wide single-row-batch control | Wide FP32 full-cache, effective `batchRows=1` per batch |

`source_derived_batch_rows` is calculated from the pinned source's `ChooseWideFullYRows`, UB budget, tile width, `rows=2`, and `blockCount=1`; it records the effective rows in each processing batch. The harness cannot read the kernel-local variable without changing Candidate, so this value is source-derived rather than runtime instrumentation. The 14 invocations run both Parent and Candidate for every width.
The result TSV stores `0` for D=8192 because that shape uses the non-wide path; its table entry is `NA`.

Only FP32 is included because the selected edit is in `ProcessWideFp32FullCacheRows`, called only for `T=float`. FP16/BF16 use separate low-precision paths and are outside this Revision's change surface.

## Precision Rule

The runner compares each side against a deterministic CPU reference using FP32 mixed tolerance: `atol=2^-16`, `rtol=2^-10`, required matched ratio `>=0.99`, and maximum absolute error `<=1e-2`. CPU reference accumulation for the RMS statistic uses double precision; the host target keeps elementwise FP32 multiply and add operations separate. Each invocation exits nonzero on ACL failure, non-finite output, or a failed threshold.

## Route-Local Files

- `support/CMakeLists.txt`: exact-source Parent/Candidate ASC shared libraries and ACL correctness executable.
- `support/runner_parent.asc`, `support/runner_candidate.asc`, `support/local_abi_shim.h`: local direct-invocation ABI wrappers; the tracked Candidate source remains unchanged.
- `support/runner_correctness.cpp`: device-7 correctness-only runner; no timing API or device reset.
- `support/build_server3.sh`, `support/correctness_server3.sh`: separate commands, per-run server output paths, disk-space cutoff, and pre/post resource snapshots.
