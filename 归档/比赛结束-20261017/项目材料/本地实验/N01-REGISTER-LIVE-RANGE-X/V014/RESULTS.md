# N01 V014 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V014
SOURCE_PARENT_REVISION: V013 (`c3d557a0fa2564f1db3561f28d4ccf3f219e4bfc`)
SOURCE_COMMIT: `b25f528e8d165cabde0bdd14268674cc718e1ee1`
SOURCE_SHA256: `4efece873de93237048d54c0658eac05891a9c88ec3b2a097aa6f9ab40cc54d4`
SOURCE_PATH: `本地实验/N01-REGISTER-LIVE-RANGE-X/V014/submission.asc`
MEASUREMENT_PARENT: R31B V011
MEASUREMENT_PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

In `ProcessWideLowPrecision` Pass 1, V014 acquires the non-half `residualFp32` LocalTensor view inside the active tile branch instead of outside the tile loop. The underlying buffers, vector values, arithmetic, DMA order, synchronization, tile schedule, and core allocation are unchanged. This is a view-lifetime experiment; it does not establish a register-count reduction.

## Build and Device Code

- CANN `8.5.0.alpha002`; configure and `paired_runner_v016` build both returned `0`.
- Build log: `results/build-v014.log`.
- Runner SHA256: `8d6565461b579e71391264b572b983e28f63c8aa608b500d90e0728b19f47e6f`; executable retained at `results/paired_runner_v016`.
- Extracted `.aicore_binary` SHA256: `ff9884222ea0651284e295811a887b79b743f8afd1a50af13f35056d832ca158`; it differs from V013. `.text` size is `0x14158`; FP32/FP16/BF16 function sizes are `12840/13772/14592` bytes. Register count is unavailable.
- The direct source parent is V013; V014 changes only that source. `results/compile-v014.command` identifies the exact Candidate input path, source commit, and source SHA. The remote Candidate SHA matched the local source SHA. The fixed build/measurement Parent was the V004 R31B V011 input with SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Runner labels are generic slots: Correctness uses `V001` for its Candidate slot; paired Local uses `V016`. Both refer to the V014 compiled Candidate above, not Route revisions V001 or V016. Parent is R31B V011.

## Correctness

- d5 Correctness: 4/4 passed; runner exit code `0`.
- Raw output: `results/correctness-v014.raw.log`.

## Paired Local

The existing `paired_runner_v016` completed 21 interleaved pairs per case (84 total) on d5. Values are device-event microseconds; paired delta is Candidate minus Parent. Full samples, summary statistics, and outliers remain in `results/local-v014.raw.log`.

| Case | Parent median (p10-p90; min-max) | Candidate median (p10-p90; min-max) | Paired delta median (p10-p90; min-max) |
|---|---|---|---|
| FP16 D=12288 | 6.120 ([5.660, 17.400]; [5.560, 88.120]) | 6.300 ([5.680, 90.640]; [5.600, 165.800]) | +0.060 ([-0.260, +84.700]; [-80.560, +159.660]) |
| FP16 D=32768 | 10.500 ([9.660, 12.100]; [9.520, 107.860]) | 10.120 ([9.760, 32.420]; [9.640, 115.360]) | +0.040 ([-0.920, +7.500]; [-83.120, +92.460]) |
| BF16 D=12288 | 6.980 ([6.320, 60.580]; [6.060, 203.200]) | 7.400 ([6.340, 77.820]; [5.840, 195.220]) | +0.160 ([-53.320, +71.420]; [-196.940, +189.160]) |
| BF16 D=32768 | 13.900 ([13.360, 14.520]; [13.200, 172.200]) | 13.480 ([13.080, 14.120]; [12.720, 109.600]) | -0.440 ([-0.860, +0.760]; [-158.520, +95.080]) |

The distributions contain substantial outliers, including device-event samples above 100 us. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`; no gain is claimed from the central medians.

## Device State and Evidence

Before Local, d5 reported HBM use `91%`, AICore `0%`, and the existing `VLLMWorker_TP` process (PID 3355594, 56354 MB). The same HBM, AICore, and process state was present after Local; the V014 runner had exited. No other process was changed.

Remote run directory: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V014/b25f528e`

The post-Local device snapshot is retained, but no matching `post-local-device.command` file is present in the current worktree.

Evidence files: `results/compile-v014.command`, `results/build-v014.log`, `results/device-code-v014.log`, `results/aicore_binary_v014.bin`, `results/paired_runner_v016`, `results/correctness-v014.command`, `results/correctness-v014.raw.log`, `results/pre-correctness-device.command`, `results/pre-correctness-device.txt`, `results/pre-local-device.command`, `results/pre-local-device.txt`, `results/local-v014.command`, `results/local-v014.raw.log`, `results/post-local-device.txt`.

NEXT_ACTION: preserve and push these V014 results, then test a distinct BF16 Pass 2 FP32 temporary-reuse hypothesis on d5. Do not repeat V001 dtype or V007-V014 view-lifetime changes.
