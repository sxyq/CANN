# N01 V012 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V012
PARENT_REVISION: V011 (`d3341d74208cb73a49c939fb28e960e3d1eca84d`)
SOURCE_COMMIT: `beae4dcca02b5bf90550d3d290188e8f98cba518`
SOURCE_SHA256: `4e6e4b1f13f00bffbff9102c65f9ab0e8b788adcbd6a653331faeac8c0da5a17`
SOURCE_PATH: `本地实验/N01-REGISTER-LIVE-RANGE-X/V012/submission.asc`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

In `ProcessWideLowPrecision` Pass 2, V012 moves only the `xFp32` LocalTensor view acquisition into the non-half parameter-conversion block and into the per-row application branches. The underlying FP32 buffer contents, Vector operation order, address expressions, DMA order, synchronization, tile schedule, and core allocation are unchanged.

## Build and Device Code

- CANN `8.5.0.alpha002`; configure and `paired_runner_v016` build both returned `0`.
- Build log: `results/build-v012.log`.
- Runner SHA256: `9d67ececaefcb3c51c4e6d0e1384ebbb762b60e961a20e5eba18ba6c7f9074dc`; binary retained at `results/paired_runner_v016`.
- The build used the V004 parent source at `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V004/e833b161/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc` and V012 Candidate at `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V012/beae4dcc/candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc`. The remote Candidate SHA256 matches the V012 source SHA above; Parent SHA256 is `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Extracted `.aicore_binary` SHA256: `32a6969657ee119dcc83f21d2459987b1669b968b584b14fd84c285602defbe5`; it differs from V011 (`515b0cfadca671e2604e80446895308b2885b7d0e535fbe65dffc870d4ddfc56`). `.text` size is `0x14148`. Candidate function sizes are FP32 `12840`, FP16 `13800`, BF16 `14548` bytes. Register count is unavailable; no register-count improvement is claimed.
- `results/device-code-v012.log` records the extracted device segment identity, symbols, and sections.

## Correctness

- d5 Correctness: 4/4 passed; runner exit `0`.
- Raw output: `results/correctness-v012.raw.log`.
- Runner labels Parent `V011` and Candidate `V001` are generic. The tested inputs are identified by exact source paths and SHA256 values above and in `results/correctness-v012.command`.

## Paired Local

The existing `paired_runner_v016` completed 21 interleaved pairs per case (84 total) on d5. Parent is R31B V011; Candidate source is V012. Runner labels Parent `V011` and Candidate `V016` are generic slots, not Route revision names. Values are device-event microseconds; delta is Candidate minus Parent.

| Case | Parent median; p10-p90; min-max | Candidate median; p10-p90; min-max | Paired delta median; p10-p90; min-max |
|---|---|---|---|
| FP16 D=12288 | 6.280; 5.640..77.240; 5.340..116.240 | 6.220; 5.800..73.700; 5.540..146.360 | +0.020; -60.100..+34.840; -71.520..+140.200 |
| FP16 D=32768 | 9.880; 9.700..11.740; 9.600..19.420 | 10.180; 9.760..12.020; 9.600..15.880 | +0.060; -0.240..+0.480; -9.780..+6.160 |
| BF16 D=12288 | 6.900; 6.540..17.220; 6.300..150.280 | 6.500; 6.240..7.220; 6.200..156.120 | -0.240; -0.540..+0.180; -143.720..+138.900 |
| BF16 D=32768 | 14.080; 13.580..77.480; 13.400..168.940 | 13.440; 12.960..18.800; 12.860..128.760 | -0.640; -64.540..+4.620; -155.420..+115.360 |

All 84 raw pairs and outliers are retained in `results/local-v012.raw.log`. Timing distributions remain noisy, with wide tails and large opposing spikes. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`; no gain is claimed from the medians.

## Device State and Evidence

Before Correctness and Local, d5 reported HBM use `91%`, AICore `0%`, and the existing `VLLMWorker_TP` process (PID 3355594, 56354 MB). The same device/process state remained after Local; the V012 runner had exited. No other process was changed.

Remote run directory: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V012/beae4dcc`

Evidence files: `results/compile-v012.command`, `results/build-v012.log`, `results/device-code-v012.log`, `results/aicore_binary_v012.bin`, `results/paired_runner_v016`, `results/correctness-v012.command`, `results/correctness-v012.raw.log`, `results/pre-correctness-device.command`, `results/pre-correctness-device.txt`, `results/pre-local-device.command`, `results/pre-local-device.txt`, `results/local-v012.command`, `results/local-v012.raw.log`, `results/post-local-device.command`, `results/post-local-device.txt`.

NEXT_ACTION: commit and push V012 results, then continue with one distinct wide-row temporary lifetime change on d5. Do not repeat the changes from V007-V012.
