# N01 V011 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V011
PARENT_REVISION: V010 (`33ded88d38088d696d3f208d906d92d871686669`)
SOURCE_COMMIT: `d3341d74208cb73a49c939fb28e960e3d1eca84d`
SOURCE_SHA256: `c0f3ed3ccb2f31f1618549e3f98d5ef2f01f92f16b11e5a9f94439fb5208eb26`
SOURCE_PATH: `本地实验/N01-REGISTER-LIVE-RANGE-X/V011/submission.asc`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

In `ProcessWideLowPrecision` Pass 2, V011 moves only the `valueFp32` LocalTensor view acquisition from the per-tile setup into the non-half per-row branch. The view now has one row's use interval. Vector operations, scalar address expressions, buffer allocation, DMA order, synchronization, tile schedule, and core allocation are unchanged.

## Build and Device Code

- CANN `8.5.0.alpha002`; configure and `paired_runner_v016` build both returned `0`.
- Build log: `results/build-v011.log`.
- Runner SHA256: `8c5f593492ad3baffea9c90d337f0aea153c901e18141ba2b02fe6c283a7a619`; binary retained at `results/paired_runner_v016`.
- The build used the V004 parent source at `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V004/e833b161/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc` and V011 Candidate at `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V011/d3341d74/candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc`. The remote Candidate SHA256 matches the V011 source SHA above; the parent input SHA256 is `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Extracted `.aicore_binary` SHA256: `515b0cfadca671e2604e80446895308b2885b7d0e535fbe65dffc870d4ddfc56`; it differs from V010 (`21704136130ca9df7ba7fec104afe36ef999d0f60d229423a10375da15f26d0e`). `.text` size is `0x14104`. Candidate function sizes are FP32 `12840`, FP16 `13788`, BF16 `14492` bytes. Register count is unavailable; no register-count improvement is claimed.
- `results/device-code-v011.log` records extraction, device function symbols, and sections.

## Correctness

- d5 Correctness: 4/4 passed; runner exit `0`.
- Raw output: `results/correctness-v011.raw.log`.
- Runner output labels Parent `V011`, Candidate `V001`; these are generic runner labels. The tested source inputs are identified by the exact paths and SHA256 values above and in `results/correctness-v011.command`.

## Paired Local

The existing `paired_runner_v016` completed 21 interleaved pairs per case (84 total) on d5. Parent is R31B V011; Candidate source is V011. Runner labels Parent `V011` and Candidate `V016` are generic slots, not Route revision names. Values are device-event microseconds; delta is Candidate minus Parent.

| Case | Parent median | Candidate median | Paired delta median | Delta range | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 6.220 | 5.940 | -0.100 | -151.980 to +78.380 | 21 |
| FP16 D=32768 | 10.340 | 10.280 | -0.080 | -8.960 to +88.980 | 21 |
| BF16 D=12288 | 6.660 | 6.620 | -0.140 | -101.440 to +7.580 | 21 |
| BF16 D=32768 | 13.880 | 13.320 | -0.560 | -121.740 to +139.040 | 21 |

All raw pairs and outliers are retained in `results/local-v011.raw.log`. Large timing spikes remain, especially for the D=32768 BF16 case. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`; no gain is claimed from the medians.

## Device State and Evidence

Before Correctness and Local, d5 reported HBM use `91%`, AICore `0%`, and the existing `VLLMWorker_TP` process (PID 3355594, 56354 MB). The same process and reported device state remained after Local; the V011 runner had exited. No other process was changed.

Remote run directory: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V011/d3341d74`

Evidence files: `results/compile-v011.command`, `results/build-v011.log`, `results/device-code-v011.log`, `results/aicore_binary_v011.bin`, `results/paired_runner_v016`, `results/correctness-v011.command`, `results/correctness-v011.raw.log`, `results/pre-correctness-device.command`, `results/pre-correctness-device.txt`, `results/pre-local-device.command`, `results/pre-local-device.txt`, `results/local-v011.command`, `results/local-v011.raw.log`, `results/post-local-device.command`, `results/post-local-device.txt`.

NEXT_ACTION: preserve and push V011 results, then continue with an independent wide-row temporary lifetime question based on the exact V011 source and retained device-code evidence. Do not repeat V007-V010 changes.
