# N01 V009 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V009
PARENT_REVISION: V008 (`f03947035feab6674a1a558becc42475cb3cf788`)
SOURCE_COMMIT: `4298b511a1407d8bfff9f00054efedb3d7bef522`
SOURCE_SHA256: `75fd82bbd9dd9b97bb6f47233c600f871bffe1338632638db2035e4ab662c450`
SOURCE_PATH: `本地实验/N01-REGISTER-LIVE-RANGE-X/V009/submission.asc`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

V009 moves the `reduceLocal` FP32 LocalTensor view from the row-batch scope into the Pass 1 block of `ProcessWideLowPrecision`. The view is used only while writing Pass 1 reduction partials; Pass 1 arithmetic, reductions, DMA order, synchronization, buffer allocation, tiling, and Pass 2 are unchanged. The hypothesis was that ending this view before normalization and Pass 2 might reduce overlap in the generated code.

## Build and Device Code

- CANN `8.5.0.alpha002`; configure and `paired_runner_v016` build both returned `0`.
- Build log: `results/build-v009.log`.
- Runner SHA256: `1dde0235b347b59f6ef62d6937051b0f7191a618fe37a0a9e32249bcc6d30811`; executable retained at `results/paired_runner_v016`.
- Extracted `.aicore_binary` SHA256: `22d7aad2c4c88ba876887dc46765e78907ce83cef502398b53137584a9383e4a`, byte-identical to V008. `.text` remains `0x14158` bytes.
- Candidate function sizes remain FP32 `12840`, FP16 `13804`, BF16 `14560` bytes. The scope change produced no device-binary difference; register count is unavailable.
- Exact V009 and reused Runner source identities, extraction command, symbols, and sections are retained in `results/compile-v009.command` and `results/device-code-v009.log`.

## Correctness

- d5 Correctness: 4/4 passed; Runner exit `0`.
- Raw output: `results/correctness-v009.raw.log`.
- Runner labels are generic: Parent `V011`, Candidate `V001`. Candidate identity is this V009 source commit, SHA256, and remote path in `results/correctness-v009.command`.

## Paired Local

The existing `paired_runner_v016` completed 21 interleaved pairs for each of four cases (84 total) on d5. Parent is R31B V011; Candidate is V009. Runner labels Parent `V011` and Candidate `V016` are generic slots, not revision identifiers. Values are device-event microseconds; delta is Candidate minus Parent.

| Case | Parent median | Candidate median | Paired delta median | Delta range | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 6.720 | 5.880 | -0.340 | -187.280 to +49.600 | 21 |
| FP16 D=32768 | 10.100 | 10.140 | +0.020 | -0.740 to +23.620 | 21 |
| BF16 D=12288 | 7.180 | 7.020 | -0.140 | -113.340 to +61.160 | 21 |
| BF16 D=32768 | 14.120 | 14.000 | +0.020 | -40.620 to +68.860 | 21 |

All raw pairs and outliers are retained in `results/local-v009.raw.log`. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`; no gain is claimed from the noisy medians.

## Device State and Evidence

Before Correctness and Local, d5 reported HBM 91%, AICore 0%, and the existing `VLLMWorker_TP` process at 56354 MB. After Local the same device/process state remained; the V009 Runner had exited. No other process was changed.

Remote run directory: `~/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V009/4298b511`

Evidence files: `results/compile-v009.command`, `results/build-v009.log`, `results/device-code-v009.log`, `results/aicore_binary_v009.bin`, `results/paired_runner_v016`, `results/correctness-v009.command`, `results/correctness-v009.raw.log`, `results/pre-correctness-device.command`, `results/pre-correctness-device.txt`, `results/pre-local-device.command`, `results/pre-local-device.txt`, `results/local-v009.command`, `results/local-v009.raw.log`, `results/post-local-device.command`, `results/post-local-device.txt`.

NEXT_ACTION: continue with V010 on d5 using one distinct wide-row FP32 temporary lifetime hypothesis. Do not repeat V008 or V009 scope changes.
