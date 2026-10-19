# N01 V015 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V015
SOURCE_PARENT_REVISION: V014 (`b25f528e8d165cabde0bdd14268674cc718e1ee1`)
SOURCE_COMMIT: `964b0f3b3302d0072bdf69178d90b04b2b33ae42`
SOURCE_SHA256: `43852ba9c9d122c39d15c47752a103275c225f05ca1042d2e6f39cc3968130ca`
SOURCE_PATH: `本地实验/N01-REGISTER-LIVE-RANGE-X/V015/submission.asc`
MEASUREMENT_PARENT: R31B V011
MEASUREMENT_PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

In the BF16 branch of `ProcessWideLowPrecision` Pass 2, V015 removes the simultaneous gamma and bias FP32 staging vectors. It converts gamma into `xFp32`, applies the existing normalization and gamma multiply, then overwrites that same tile with converted bias before the existing add. This tests shorter live storage for converted parameters. The arithmetic order, allocated buffers, DMA order, synchronization, tile schedule, and core allocation are unchanged. Conversion is repeated per row within a batch.

## Build and Device Code

- CANN `8.5.0.alpha002`; configure and `paired_runner_v016` build both returned `0`.
- Build log: `results/build-v015.log`.
- Runner SHA256: `2ebd32050445029ed1150f61640fd1770cc04cb78cab9c3b3f630405ec75ace5`; binary retained at `results/paired_runner_v016`.
- The exact Candidate input path is `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V015/964b0f3b/candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc`; its remote SHA matched the local source SHA. The fixed R31B V011 Parent input SHA is `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Extracted `.aicore_binary` SHA256: `cfb1914acf2210fc93b6e58524a2f9644f15aa3dfadc32471946ee8e86a271fb`; it differs from V014. `.text` size is `0x14120`; Candidate function sizes are FP32 `12840`, FP16 `13772`, BF16 `14536` bytes. BF16 function size decreased 56 bytes from V014; register count is unknown and no register reduction is asserted.
- Runner labels are generic slots: Correctness labels Candidate `V001`, while paired Local labels it `V016`. Both refer to the V015 compiled Candidate above, not Route revisions V001 or V016. Parent is R31B V011.

## Correctness

- d5 Correctness: 4/4 passed; Runner exit `0`.
- Raw output: `results/correctness-v015.raw.log`.

## Paired Local

The existing `paired_runner_v016` completed 21 interleaved pairs per case (84 total) on d5. Values are device-event microseconds; paired delta is Candidate minus Parent. Raw samples and all outliers remain in `results/local-v015.raw.log`.

| Case | Parent median (p10-p90; min-max) | Candidate median (p10-p90; min-max) | Paired delta median (p10-p90; min-max) |
|---|---|---|---|
| FP16 D=12288 | 6.160 ([5.660, 7.220]; [5.540, 7.240]) | 6.040 ([5.720, 7.360]; [5.680, 168.620]) | +0.060 ([-0.400, +0.440]; [-1.020, +161.400]) |
| FP16 D=32768 | 10.140 ([9.680, 11.780]; [9.620, 155.200]) | 10.100 ([9.720, 16.920]; [9.600, 114.600]) | +0.000 ([-0.380, +4.600]; [-40.600, +21.500]) |
| BF16 D=12288 | 7.300 ([6.760, 9.360]; [6.260, 74.180]) | 6.860 ([6.280, 178.020]; [6.200, 207.940]) | -0.300 ([-2.380, +171.080]; [-67.560, +199.220]) |
| BF16 D=32768 | 14.180 ([13.700, 18.960]; [13.580, 217.520]) | 13.560 ([13.220, 15.480]; [13.040, 103.080]) | -0.640 ([-5.500, +0.600]; [-147.840, +2.300]) |

Every case contains one or more large outliers; the BF16 D=12288 Candidate maximum is `207.940 us`, and the BF16 D=32768 Parent maximum is `217.520 us`. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`; no gain is inferred from the medians.

## Device State and Evidence

Before Correctness, d5 reported HBM use `91%`, AICore `0%`, and existing `VLLMWorker_TP` PID 3355594 at 56354 MB. Before Local it reported the same HBM/process state and AICore `0%`; the post-Local snapshot again showed the same state and no paired Runner process. No other process was changed.

Remote run directory: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V015/964b0f3b`

Evidence files: `results/compile-v015.command`, `results/build-v015.log`, `results/device-code-v015.log`, `results/aicore_binary_v015.bin`, `results/paired_runner_v016`, `results/correctness-v015.command`, `results/correctness-v015.raw.log`, `results/pre-correctness-device.command`, `results/pre-correctness-device.txt`, `results/pre-local-device.command`, `results/pre-local-device.txt`, `results/local-v015.command`, `results/local-v015.raw.log`, `results/post-local-device.command`, `results/post-local-device.txt`, `results/local-summary-query-attempt-01.command`.

The first remote post-run summary query returned `127` because `rg` is not installed on server3. It did not affect either device run; the summary was subsequently computed from the retained local raw log.

NEXT_ACTION: commit and push these V015 results, then test an independent single-factor wide-row FP32 temporary live-range hypothesis as V016 on d5.
