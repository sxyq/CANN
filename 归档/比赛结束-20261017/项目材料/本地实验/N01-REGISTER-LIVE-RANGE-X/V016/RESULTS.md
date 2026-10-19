# N01 V016 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V016
SOURCE_PARENT_REVISION: V015 (`964b0f3b3302d0072bdf69178d90b04b2b33ae42`)
SOURCE_COMMIT: `b292bf75f6566eb927666b774022855a72ef6d1c`
SOURCE_SHA256: `f53b21a9f5ed77c82d26b469d71f4385374fefa47b6f13378ed5f555143a7f21`
SOURCE_PATH: `本地实验/N01-REGISTER-LIVE-RANGE-X/V016/submission.asc`
MEASUREMENT_PARENT: R31B V011
MEASUREMENT_PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

In the BF16 branch of `ProcessWideLowPrecision` Pass 1, V016 converts `x` directly into the retained `valueTile`, converts residual into its existing FP32 temporary, and adds residual into `valueTile` in place. It acquires `xFp32` afterward for the square operation. The hypothesis is that the widened add no longer needs a separate live FP32 tile for `x`. Arithmetic order, buffer allocation, DMA order, synchronization, tile schedule, and core allocation remain unchanged; FP16 and other paths are unchanged.

## Build and Device Code

- CANN `8.5.0.alpha002`; configure and `paired_runner_v016` build both returned `0`.
- Build log: `results/build-v016.log`.
- Runner SHA256: `b4d1c1c15db53caf0d9d9cf081a1bd38027d9265067327191648fa911db1c1eb`; binary retained at `results/paired_runner_v016`.
- Exact Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V016/b292bf75/candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc`; remote SHA matched the local source SHA. The fixed R31B V011 Parent input SHA is `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Extracted `.aicore_binary` SHA256: `c7e7850a0f65aba143115279891a6dc7656de3296614c30835c2449f43c30376`; it differs from V015. `.text` size is `0x14110`; Candidate function sizes are FP32 `12840`, FP16 `13772`, BF16 `14520` bytes. Register count is unknown; no register reduction is claimed.
- Runner labels are generic slots: Correctness uses Candidate `V001`; paired Local uses Candidate `V016`. Both identify this compiled V016 Candidate, not Route revisions V001/V016. Parent is R31B V011.

## Correctness

- d5 Correctness: 4/4 passed; Runner exit `0`.
- Raw output: `results/correctness-v016.raw.log`.

## Paired Local

The existing `paired_runner_v016` completed 21 interleaved pairs per case (84 total) on d5. Values are device-event microseconds; paired delta is Candidate minus Parent. Raw samples and all outliers remain in `results/local-v016.raw.log`.

| Case | Parent median (p10-p90; min-max) | Candidate median (p10-p90; min-max) | Paired delta median (p10-p90; min-max) |
|---|---|---|---|
| FP16 D=12288 | 6.200 ([5.620, 79.740]; [5.580, 88.540]) | 6.060 ([5.760, 90.140]; [5.520, 161.720]) | +0.160 ([-31.620, +82.840]; [-77.220, +154.920]) |
| FP16 D=32768 | 9.980 ([9.700, 11.740]; [9.560, 16.000]) | 10.140 ([9.660, 23.800]; [9.600, 170.180]) | +0.160 ([-0.200, +12.240]; [-1.000, +158.440]) |
| BF16 D=12288 | 6.880 ([6.380, 15.340]; [6.320, 84.320]) | 6.700 ([6.280, 17.680]; [6.220, 155.240]) | +0.020 ([-0.600, +2.340]; [-77.840, +148.040]) |
| BF16 D=32768 | 13.660 ([13.540, 88.180]; [13.540, 174.380]) | 13.060 ([12.820, 13.660]; [12.760, 108.560]) | -0.600 ([-74.600, -0.160]; [-161.580, +94.300]) |

The distributions have substantial tails and large outliers across cases; FP16 D=32768 Candidate reaches `170.180 us`, while BF16 D=32768 Parent reaches `174.380 us`. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`; no gain is claimed from central medians.

## Device State and Evidence

Before Correctness and Local, d5 reported HBM use `91%`, AICore `0%`, and existing `VLLMWorker_TP` PID 3355594 using 56354 MB. After Local, the same HBM/AICore/process state remained and the V016 Runner had exited. No other process was changed.

Remote run directory: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V016/b292bf75`

Evidence files: `results/compile-v016.command`, `results/build-v016.log`, `results/device-code-v016.log`, `results/aicore_binary_v016.bin`, `results/paired_runner_v016`, `results/correctness-v016.command`, `results/correctness-v016.raw.log`, `results/pre-correctness-device.command`, `results/pre-correctness-device.txt`, `results/pre-local-device.command`, `results/pre-local-device.txt`, `results/local-v016.command`, `results/local-v016.raw.log`, `results/post-local-device.command`, `results/post-local-device.txt`.

NEXT_ACTION: commit and push these V016 results, then test an independent one-factor wide-row FP32 temporary lifetime hypothesis as V017 on d5.
