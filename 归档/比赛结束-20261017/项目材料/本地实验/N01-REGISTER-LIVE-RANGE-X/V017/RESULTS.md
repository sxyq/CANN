# N01 V017 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V017
SOURCE_PARENT_REVISION: V016 (`b292bf75f6566eb927666b774022855a72ef6d1c`)
SOURCE_COMMIT: `b0801b847d1d523fddb4c8e37221b8ab59040e2c`
SOURCE_SHA256: `24918945466c2f1d848f96c610582b45ba8c2011ea72be7f65321f91dc4cdcad`
SOURCE_PATH: `本地实验/N01-REGISTER-LIVE-RANGE-X/V017/submission.asc`
MEASUREMENT_PARENT: R31B V011
MEASUREMENT_PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

In the BF16 branch of `ProcessWideLowPrecision` Pass 1, V017 converts x into the retained y tile and residual into `residualFp32`, then adds residual to y. Once that add completes, the residual values are dead; V017 overwrites the same `residualFp32` tile with y squared and passes it as the ReduceSum source, using `xFp32` as the separate work tile. Arithmetic order, buffer allocation, DMA order, synchronization, tile schedule, and core allocation are unchanged. This is distinct from V016, which wrote the square to `xFp32` and used `residualFp32` as ReduceSum work storage.

## Build and Device Code

- CANN `8.5.0.alpha002`; configure and `paired_runner_v016` build both returned `0`.
- Build log: `results/build-v017.log`.
- Runner SHA256: `f5cbe9fd900cdf8d583df1b47983ad04cc8ab662547303acdaf4ca22bcb6273f`; binary retained at `results/paired_runner_v016`.
- Exact Candidate input is the V017 source at `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V017/b0801b84/candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc`; its remote SHA matched the local source SHA. Fixed R31B V011 Parent input SHA is `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Extracted `.aicore_binary` SHA256: `99665b5ab812e6f9a2f61787ae5c51dac5cbe2a50e0452cd52cbe13ab0cd41da`; it differs from V016. `.text` size is `0x14108`; Candidate function sizes are FP32 `12840`, FP16 `13772`, BF16 `14512` bytes. Register count is unknown; no register reduction is claimed.
- Runner labels are generic: Correctness Candidate `V001` and Local Candidate `V016` both refer to this compiled V017 input. Parent is R31B V011.

## Correctness

- d5 Correctness: 4/4 passed; Runner exit `0`.
- Raw output: `results/correctness-v017.raw.log`.

## Paired Local

The existing Runner completed 21 interleaved pairs for each case (84 total) on d5. Values are device-event microseconds; paired delta is Candidate minus Parent. All samples and outliers remain in `results/local-v017.raw.log`.

| Case | Parent median (p10-p90; min-max) | Candidate median (p10-p90; min-max) | Paired delta median (p10-p90; min-max) |
|---|---|---|---|
| FP16 D=12288 | 6.160 ([5.600, 7.560]; [5.480, 123.740]) | 6.020 ([5.700, 7.400]; [5.660, 10.460]) | +0.140 ([-0.480, +0.480]; [-118.060, +4.320]) |
| FP16 D=32768 | 11.360 ([9.880, 73.200]; [9.540, 148.860]) | 11.620 ([9.820, 107.940]; [9.620, 217.500]) | +0.000 ([-62.900, +97.780]; [-116.500, +207.620]) |
| BF16 D=12288 | 7.020 ([6.460, 58.640]; [6.340, 199.720]) | 7.540 ([6.440, 106.360]; [6.120, 208.580]) | +0.120 ([-17.080, +15.120]; [-48.200, +191.740]) |
| BF16 D=32768 | 13.880 ([13.600, 104.440]; [13.520, 183.760]) | 13.300 ([12.860, 33.840]; [12.760, 169.040]) | -0.660 ([-90.440, +20.060]; [-171.000, +154.960]) |

Large outliers appear in every case; FP16 D=32768 Candidate reaches `217.500 us`, BF16 D=12288 Candidate reaches `208.580 us`, and BF16 D=32768 has paired deltas from `-171.000` to `+154.960 us`. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`; no median-only gain is claimed.

## Device State and Evidence

Before Correctness and Local, d5 reported HBM use `91%`, AICore `0%`, and existing `VLLMWorker_TP` PID 3355594 at 56354 MB. The same state remained after Local, with the Candidate Runner exited. No other process was changed.

Remote run directory: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V017/b0801b84`

Evidence files: `results/compile-v017.command`, `results/build-v017.log`, `results/device-code-v017.log`, `results/aicore_binary_v017.bin`, `results/paired_runner_v016`, `results/correctness-v017.command`, `results/correctness-v017.raw.log`, `results/pre-correctness-device.command`, `results/pre-correctness-device.txt`, `results/pre-local-device.command`, `results/pre-local-device.txt`, `results/local-v017.command`, `results/local-v017.raw.log`, `results/post-local-device.command`, `results/post-local-device.txt`.

NEXT_ACTION: commit and push V017 results, then test a different one-factor wide-row FP32 temporary lifetime question as V018 on d5.
