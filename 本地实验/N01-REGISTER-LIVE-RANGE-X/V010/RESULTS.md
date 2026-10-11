# N01 V010 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V010
PARENT_REVISION: V009 (`4298b511a1407d8bfff9f00054efedb3d7bef522`)
SOURCE_COMMIT: `33ded88d38088d696d3f208d906d92d871686669`
SOURCE_SHA256: `389e01bf37813293d4477d711c03c6419602297906e3be9fdaf59d2c80870f7b`
SOURCE_PATH: `本地实验/N01-REGISTER-LIVE-RANGE-X/V010/submission.asc`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

In `ProcessWideLowPrecision` Pass 2, V010 removes a repeated gamma/bias conversion into the same FP32 staging buffers, along with the barrier that followed that repeated conversion. The first conversion and its barrier remain before the row loop. Arithmetic results, buffer allocation, DMA order, synchronization needed by the remaining operations, tile schedule, and core allocation are unchanged. The change removes duplicate Vector writes to the temporary values; generated code is inspected below.

## Build and Device Code

- CANN `8.5.0.alpha002`; configure and `paired_runner_v016` build both returned `0`.
- Build log: `results/build-v010.log`.
- Runner SHA256: `10c1f3dca55ac458db25f781c3c7cc45dd551833f9cc39010c6ec863eda4c739`; binary retained at `results/paired_runner_v016`.
- The build used the V004 parent source at `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V004/e833b161/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc` and V010 Candidate at `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V010/33ded88d/candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc`. The remote Candidate SHA256 matches the V010 source SHA above; the parent input SHA256 is `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Extracted `.aicore_binary` SHA256: `21704136130ca9df7ba7fec104afe36ef999d0f60d229423a10375da15f26d0e`; it differs from V009 (`22d7aad2c4c88ba876887dc46765e78907ce83cef502398b53137584a9383e4a`). `.text` size is `0x1413c`, versus V009 `0x14158`. Candidate function sizes are FP32 `12840`, FP16 `13804`, BF16 `14532` bytes. Register count is unavailable; no register-count improvement is claimed.
- `results/device-code-v010.log` records extraction and object symbols/sections. `results/device-elf-v010.readelf.log` records the extracted device ELF symbols and sections.

## Correctness

- d5 Correctness: 4/4 passed; runner exit `0`.
- Raw output: `results/correctness-v010.raw.log`.
- Runner output labels Parent `V011`, Candidate `V001`; these are generic runner labels. The tested parent and Candidate are identified by the exact input paths and SHA256 values above and in `results/correctness-v010.command`.

## Paired Local

The existing `paired_runner_v016` completed 21 interleaved pairs per case (84 total) on d5. Parent is R31B V011; Candidate source is V010. Runner labels Parent `V011` and Candidate `V016` are generic slots, not route revision names. Values are device-event microseconds; delta is Candidate minus Parent.

| Case | Parent median | Candidate median | Paired delta median | Delta range | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 6.240 | 6.160 | +0.020 | -112.180 to +104.680 | 21 |
| FP16 D=32768 | 10.380 | 10.160 | -0.140 | -162.120 to +48.420 | 21 |
| BF16 D=12288 | 6.580 | 6.480 | -0.180 | -207.600 to +109.180 | 21 |
| BF16 D=32768 | 13.820 | 13.240 | -0.600 | -1.000 to +90.160 | 21 |

All 84 raw pairs and outliers are retained in `results/local-v010.raw.log`. Large timing spikes occur across cases. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`; no performance gain is claimed from these medians.

## Device State and Evidence

Before Correctness and Local, d5 reported HBM usage `91%`, AICore `0%`, and the existing `VLLMWorker_TP` process (PID 3355594, 56354 MB). The same process and reported device state remained after Local; the V010 runner had exited. No other process was changed. The pre-Correctness snapshot also includes its own snapshot-shell line in the route-scoped process query; a separate `paired_runner_v016 --correctness` process query was empty before launch.

Remote run directory: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V010/33ded88d`

Evidence files: `results/compile-v010.command`, `results/build-v010.log`, `results/device-code-v010.log`, `results/device-elf-v010.readelf.log`, `results/aicore_binary_v010.bin`, `results/paired_runner_v016`, `results/correctness-v010.command`, `results/correctness-v010.raw.log`, `results/pre-correctness-device.command`, `results/pre-correctness-device.txt`, `results/pre-local-device.command`, `results/pre-local-device.txt`, `results/local-v010.command`, `results/local-v010.raw.log`, `results/post-local-device.command`, `results/post-local-device.txt`.

NEXT_ACTION: after preserving and pushing V010, create V011 from this source with one independent Pass 2 lifetime experiment: move only the `valueFp32` LocalTensor view acquisition into the per-row non-half branch, shortening that view's scope without changing vector operations, scalar address generation, buffers, synchronization, or tile behavior. Build and inspect the generated device code before Correctness and paired Local on d5.
