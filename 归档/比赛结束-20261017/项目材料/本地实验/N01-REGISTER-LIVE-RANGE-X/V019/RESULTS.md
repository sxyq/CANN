# N01 V019 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X  
REVISION: V019  
SOURCE_PARENT_REVISION: V018 (`07f0d3b603f966ebca9b4efc8d9a66087bdee458`)  
SOURCE_COMMIT: `405a56772fd2eddb32f416b99805048d7780d87d`  
SOURCE_SHA256: `be78afa95cf955825282e458c02d137fffbdc2ff36ccfb21a20d5c9357244893`  
SOURCE_PATH: `本地实验/N01-REGISTER-LIVE-RANGE-X/V019/submission.asc`  
MEASUREMENT_PARENT: R31B V011  
MEASUREMENT_PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`  
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

In FP16 `ProcessWideLowPrecision` Pass 2, the normalization conversion and multiply now use `valueFp32` rather than `xFp32` as their FP32 work tile. The values are converted from retained `yTile` and rounded to the same FP16 `outputLocal` before gamma and bias. Only the work-tile assignment changes; arithmetic order, barriers, DMA, tile schedule, and core assignment are unchanged. The source diff from V018 is limited to this three-use substitution.

## Build And Device Code

- CANN `8.5.0.alpha002`; environment setup, CMake configure, compile, and `.aicore_binary` extraction returned `0`.
- Exact Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V019/405a5677/candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc`; remote SHA256 matched the local source SHA256.
- CMake cache bound Candidate to that V019 directory and Parent to `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V004/e833b161/parent`.
- Existing generic Runner target: `paired_runner_v016`; Runner binary SHA256 `0edc705fc973e50b9207aabada9e3828dfe411a26f9574c3c7960547b03d5183`.
- Extracted AICore ELF SHA256 `dbdd82539137bbce4ab2b62c1d45b83d6a0599e3560dee8ec85dd8a31dd3e37e`; it differs from V018. `.text` changed from `0x14108` to `0x14104`; FP16 Candidate function size changed from 13772 to 13768 bytes. Register count is unavailable; no register reduction is claimed.
- Runner slot labels do not identify Route revisions: Correctness Candidate `V001` and Local Candidate `V016` both execute the compiled V019 source above. Local Parent `V011` is the fixed R31B V011 input.
- Build output: `results/build-v019.log`; device ELF and symbols: `results/aicore_binary_v019.bin`, `results/device-code-v019.log`, and `results/device-code-compare-v018-v019.log`.

## Correctness

- d5: PASS, 4/4 cases; Runner exit `0`.
- Raw output: `results/correctness-v019.raw.log`.
- Exact command and input identities: `results/correctness-v019.command`.

## Paired Local

The existing Runner completed 21 interleaved pairs per case (84 total) on d5. Times and paired deltas are device-event microseconds; delta is Candidate minus Parent. All raw samples and outliers remain in `results/local-v019.raw.log`.

| Case | Parent median (p10-p90; min-max) | Candidate median (p10-p90; min-max) | Paired delta median (p10-p90; min-max) |
|---|---|---|---|
| FP16 D=12288 | 6.160 (5.740-24.900; 5.600-111.160) | 7.160 (5.640-187.400; 5.580-210.000) | +0.020 (-19.040-+181.440; -103.880-+204.260) |
| FP16 D=32768 | 10.160 (9.680-72.360; 9.580-202.900) | 10.080 (9.760-65.400; 9.680-101.040) | -0.260 (-45.940-+0.380; -101.860-+64.880) |
| BF16 D=12288 | 6.880 (6.540-15.040; 6.420-102.860) | 6.840 (6.260-20.880; 6.080-107.820) | -0.140 (-0.660-+5.400; -70.240-+101.380) |
| BF16 D=32768 | 14.040 (13.600-14.400; 13.400-170.120) | 13.240 (12.980-13.660; 12.900-121.280) | -0.680 (-1.300--0.380; -156.680-+107.440) |

All cases contain substantial event-time outliers and/or broad paired-delta tails. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`; no median-only gain is claimed.

## Device State And Evidence

Before Correctness and Local, d5 reported HBM use `91%` and AICore use `0%`. Existing `VLLMWorker_TP` PID `3355594` used `56354 MB` before and after Local; the V019 Runner had exited after sampling. No other process was changed.

Remote run directory: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V019/405a5677`.

Evidence includes `results/compile-v019.command`, `results/build-v019.log`, `results/device-code-v019.command`, `results/device-code-v019.log`, `results/device-code-compare-v018-v019.log`, `results/aicore_binary_v019.bin`, `results/paired_runner_v016`, `results/correctness-v019.command`, `results/correctness-v019.raw.log`, `results/pre-build-device.command`, `results/pre-build-device.txt`, `results/pre-correctness-device.command`, `results/pre-correctness-device.txt`, `results/pre-local-device.command`, `results/pre-local-device.txt`, `results/local-v019.command`, `results/local-v019.raw.log`, `results/post-local-device.command`, and `results/post-local-device.txt`.

LOCAL_SCORE: NONE  
NEXT_ACTION: commit and push V019 source and result evidence, then continue with a distinct one-factor FP32 temporary lifetime hypothesis as V020 on d5. Do not repeat V019.
