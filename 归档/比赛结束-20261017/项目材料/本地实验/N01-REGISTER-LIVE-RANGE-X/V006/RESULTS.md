# N01 V006 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V006
PARENT_REVISION: V005 (`9e74552a`)
SOURCE_COMMIT: `6cc244b025049552159903387f9f77dc668cdce9`
SOURCE_SHA256: `5116d0e69f1fdc4dcb200a56f1882d259ca81b0a77ec6cbb818c1e13118f3211`
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

In `ProcessWideLowPrecision`, V006 moves the `invRmsValues` declaration and zero initialization from before Pass 1 to after Pass 1 and before scalar normalization. This shortens the array's overlap with Pass 1 Vector work. Arithmetic, memory transfers, synchronization, tiling, and core allocation are unchanged.

## Build and Correctness

- CANN `8.5.0.alpha002`, target `paired_runner_v016`: configure and build passed, both exit codes `0`.
- Runner binary SHA256: `239a8cd96dd3ac40b00e8657fc0e0d11ca0b19793ff8719d1155f9752338111a`.
- Correctness on d5: 4/4 passed, Runner exit code `0`; FP16/BF16 at D=12288 and D=32768.
- Attempt 01 exited `127` because `libgraph.so` was unavailable before loading the CANN runtime environment. Its raw error is retained. Attempt 02 loaded the environment and passed.

## Paired Local

The existing paired Runner completed 21 interleaved pairs per case, 84 pairs total, on d5. Parent is R31B V011; Candidate is the V006 source above. Values are device-event microseconds; delta is Candidate minus Parent.

| Case | Parent median | Candidate median | Paired delta median | Pairs |
|---|---:|---:|---:|---:|
| FP16 D=12288 | 6.180 | 5.920 | -0.040 | 21 |
| FP16 D=32768 | 10.220 | 10.000 | -0.180 | 21 |
| BF16 D=12288 | 6.840 | 6.960 | +0.120 | 21 |
| BF16 D=32768 | 13.900 | 13.980 | -0.080 | 21 |

`LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`. Large outliers occur in both signs, including paired deltas from -177.280 to +206.000 us. No samples were removed and no performance gain is claimed.

## Device State and Evidence

Before Correctness and Local, d5 reported HBM use at 91%, AICore use at 0%, and the existing `VLLMWorker_TP` process (PID 3355594, 56354 MB). That process was left unchanged. Pre/post snapshots, raw outputs, command records, configure/build logs, Runner binary, and ASC object are retained in `results/`.

Remote run: `~/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V006/6cc244b0`

NEXT_ACTION: continue with V007 using a distinct register-lifetime hypothesis; V006 Local does not establish a performance gain.
