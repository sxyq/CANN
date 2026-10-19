# N01 V008 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V008
PARENT_REVISION: V007 (`a22a9fb37de40f64f3087106ceda534888897242`)
SOURCE_COMMIT: `f03947035feab6674a1a558becc42475cb3cf788`
SOURCE_SHA256: `51bc4acda94997c76fdf5926371b120e5dd1efc0e955a95b6441f0a2089a3463`
SOURCE_PATH: `本地实验/N01-REGISTER-LIVE-RANGE-X/V008/submission.asc`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

In the Pass 1 loop of `ProcessWideLowPrecision`, V008 moves creation of the FP32 `xFp32` LocalTensor view from outside the tile loop into the active FP16 or BF16 branch. The view is recreated for the active tile; arithmetic, buffers, DMA order, synchronization, and tiling are unchanged. The isolated question is whether a shorter C++ view lifetime changes generated code or measured execution.

## Build and Device Code

- CANN `8.5.0.alpha002`; configure and `paired_runner_v016` build both returned `0`.
- Build log: `results/build-v008.log`.
- Runner SHA256: `1dde0235b347b59f6ef62d6937051b0f7191a618fe37a0a9e32249bcc6d30811`; the binary is retained at `results/paired_runner_v016`.
- Extracted `.aicore_binary` SHA256: `22d7aad2c4c88ba876887dc46765e78907ce83cef502398b53137584a9383e4a`; `.text` size `0x14158`.
- Candidate function sizes: FP32 `12840`, FP16 `13804`, BF16 `14560` bytes. Relative to V007, FP32 and FP16 sizes are unchanged and BF16 is 16 bytes larger. Register count is unavailable; no register reduction is claimed.
- `device-code-v008.log` retains the first summary-command error (`rg` is unavailable on server3) and the successful remote ELF inspection. The ELF extraction itself succeeded.

## Correctness

- d5 Correctness: 4/4 passed; Runner exit `0`.
- Raw output: `results/correctness-v008.raw.log`.
- Runner labels are generic: Parent `V011`, Candidate `V001`. The Candidate is identified by this V008 source commit, SHA256, and remote input path recorded in `results/correctness-v008.command`.

## Paired Local

The existing `paired_runner_v016` ran 21 interleaved pairs per case (84 total) on d5. Parent is R31B V011; Candidate source is V008. Runner labels Parent `V011` and Candidate `V016` are generic slot labels, not source revision identifiers. Values are device-event microseconds; paired delta is Candidate minus Parent.

| Case | Parent median | Candidate median | Paired delta median | Delta range | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 6.240 | 6.260 | +0.040 | -11.200 to +205.160 | 21 |
| FP16 D=32768 | 11.420 | 10.180 | -0.080 | -100.140 to +84.140 | 21 |
| BF16 D=12288 | 6.900 | 6.820 | 0.000 | -152.620 to +164.580 | 21 |
| BF16 D=32768 | 13.880 | 13.920 | +0.100 | -9.460 to +126.120 | 21 |

All raw pairs are retained in `results/local-v008.raw.log`. Large outliers occur in each case. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`; no performance gain is claimed or selected from the samples.

## Device State and Evidence

Before Correctness, d5 reported HBM 91%, AICore 0%, and the existing `VLLMWorker_TP` process at 56354 MB. The pre-Local snapshot reported the same device/process state. The route-scoped post-Local snapshot also reported HBM 91%, AICore 0%, and that same process; the V008 Runner had exited. Other processes were not changed.

Remote run directory: `~/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V008/f0394703`

Evidence files: `results/compile-v008.command`, `results/build-v008.log`, `results/device-code-v008.log`, `results/aicore_binary_v008.bin`, `results/paired_runner_v016`, `results/correctness-v008.command`, `results/correctness-v008.raw.log`, `results/pre-correctness-device.txt`, `results/pre-local-device.command`, `results/pre-local-device.txt`, `results/local-v008.command`, `results/local-v008.raw.log`, `results/post-local-device.command`, `results/post-local-device.txt`.

NEXT_ACTION: create V009 with a distinct single-factor FP32 temporary lifetime hypothesis; keep the N01 device assignment on d5.
