# N01 V013 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V013
PARENT_REVISION: V012 (`beae4dcca02b5bf90550d3d290188e8f98cba518`)
SOURCE_COMMIT: `c3d557a0fa2564f1db3561f28d4ccf3f219e4bfc`
SOURCE_SHA256: `3499280f91cd2a3069d72b08fde4cc865a99bf84238c73168e34d6847076ca32`
SOURCE_PATH: `本地实验/N01-REGISTER-LIVE-RANGE-X/V013/submission.asc`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

In `ProcessWideLowPrecision` Pass 2, V013 shortens only the `residualFp32` LocalTensor view scope. One view is acquired for bias conversion; a new view of the same buffer is acquired in each non-half row application. Buffer data, Vector operations, arithmetic order, scalar address expressions, DMA order, synchronization, tile schedule, and core allocation are unchanged.

## Build and Device Code

- CANN `8.5.0.alpha002`; configure and `paired_runner_v016` build both returned `0`.
- Build log: `results/build-v013.log`.
- Runner SHA256: `a9cd44e0209e13ab28e5ce7c679dc4c607acad6d778c02ca83fa934ddf8a7ed3`; binary retained at `results/paired_runner_v016`.
- The build used Parent at `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V004/e833b161/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc` and Candidate at `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V013/c3d557a0/candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc`. Remote Candidate SHA256 matches the V013 source SHA above; Parent SHA256 is `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Extracted `.aicore_binary` SHA256: `e518fd5fae2e0b94d23a9e847798aa42a3be896f71fe331871241749bf4da754`; it differs from V012 (`32a6969657ee119dcc83f21d2459987b1669b968b584b14fd84c285602defbe5`). `.text` size is `0x1410c`. Candidate function sizes: FP32 `12840`, FP16 `13768`, BF16 `14520` bytes. Register count is unavailable; no register-count improvement is claimed.
- `results/device-code-v013.log` records extraction, symbols, and sections.

## Correctness

- d5 Correctness: 4/4 passed; runner exit `0`.
- Raw output: `results/correctness-v013.raw.log`.
- Runner labels Parent `V011` and Candidate `V001` are generic slots. Exact tested source inputs and SHA256 values are recorded above and in `results/correctness-v013.command`.

## Paired Local

The existing `paired_runner_v016` completed 21 interleaved pairs per case (84 total) on d5. Parent is R31B V011; Candidate source is V013. Runner labels Parent `V011` and Candidate `V016` are generic slots, not Route revision names. Values are device-event microseconds; paired delta is Candidate minus Parent.

| Case | Parent median; p10-p90; min-max | Candidate median; p10-p90; min-max | Paired delta median; p10-p90; min-max |
|---|---|---|---|
| FP16 D=12288 | 5.960; 5.620..7.220; 5.560..81.980 | 5.980; 5.760..7.080; 5.600..77.740 | -0.040; -0.440..+0.460; -75.860..+71.380 |
| FP16 D=32768 | 10.160; 9.800..11.960; 9.600..35.920 | 10.260; 9.680..98.040; 9.380..217.800 | +0.180; -0.400..+86.740; -26.080..+187.780 |
| BF16 D=12288 | 6.760; 6.420..7.300; 6.240..7.680 | 6.620; 6.220..7.000; 6.020..7.240 | -0.240; -0.540..+0.060; -0.940..+0.220 |
| BF16 D=32768 | 14.060; 13.540..14.520; 13.360..163.640 | 13.380; 12.900..13.740; 12.860..14.040 | -0.740; -1.360..-0.060; -150.400..+0.580 |

All 84 raw pairs and outliers are retained in `results/local-v013.raw.log`. FP16 D=32768 has Candidate samples up to 217.800 us; BF16 D=32768 has a Parent sample at 163.640 us. The other two cases have tighter device-event ranges. Because outliers materially affect some distributions, `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`; no gain is claimed by selecting only central samples.

## Device State and Evidence

Before Correctness and Local, d5 reported HBM use `91%`, AICore `0%`, and the existing `VLLMWorker_TP` process (PID 3355594, 56354 MB). The same reported state remained after Local; the V013 runner had exited. No other process was changed.

Remote run directory: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V013/c3d557a0`

Evidence files: `results/compile-v013.command`, `results/build-v013.log`, `results/device-code-v013.log`, `results/aicore_binary_v013.bin`, `results/paired_runner_v016`, `results/correctness-v013.command`, `results/correctness-v013.raw.log`, `results/pre-correctness-device.command`, `results/pre-correctness-device.txt`, `results/pre-local-device.command`, `results/pre-local-device.txt`, `results/local-v013.command`, `results/local-v013.raw.log`, `results/post-local-device.command`, `results/post-local-device.txt`.

NEXT_ACTION: commit and push V013 results, then test one distinct wide-row FP32 temporary view lifetime factor on d5. Do not repeat V007-V013 changes.
