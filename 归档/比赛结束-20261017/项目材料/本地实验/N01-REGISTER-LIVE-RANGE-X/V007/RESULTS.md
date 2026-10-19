# N01 V007 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V007
PARENT_REVISION: V006 (`6cc244b025049552159903387f9f77dc668cdce9`)
SOURCE_COMMIT: `a22a9fb37de40f64f3087106ceda534888897242`
SOURCE_SHA256: `e99237a06ab559d12d07fdf5f3831ae16e4bfdfa0c324f9eb0be1dc028469def`
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

In `ProcessWideLowPrecision`, V007 narrows the lifetime of the Pass 1 `valueFp32` LocalTensor view from the full tile-stream loop to the active FP16 or BF16 branch. The backing buffer, calculations, transfers, synchronization, tile schedule, and core allocation are unchanged.

## Build and Correctness

- CANN `8.5.0.alpha002`, target `paired_runner_v016`: configure and final build passed, exit codes `0`.
- Build attempt 01 returned `2`: the compiler plugin lacked `ASCEND_HOME_PATH` and crashed with exit `139`. The raw failure is retained as `results/compile-v007-attempt-01.log`.
- The successful build loaded `/usr/local/Ascend/ascend-toolkit/set_env.sh` in the same shell. Runner binary SHA256: `4b0f0d94e3889943d35f97e509c31496c49cbea08fc6cd47733c84dcaf881841`.
- Correctness on d5: 4/4 passed; Runner exit code `0`. Cases: FP16/BF16 at D=12288 and D=32768.
- Correctness output labels Parent as `V011` and Candidate as `V001`. These are generic Runner labels. The tested Candidate is this V007 source commit and SHA above.

## Device Code

The `.aicore_binary` section was extracted from the V007 ASC object; extraction returned `0`. Device ELF SHA256: `0b6d16e3352e05aa20c09397e55035fe02dfcd193724fe6688ab2da2eb5ccfd8`. The `.text` section is `0x14148` bytes. Candidate function sizes from the symbol table are FP32 `12840`, FP16 `13804`, and BF16 `14544` bytes. Register count is unknown; no register-count improvement is claimed.

## Paired Local

The existing paired Runner completed 21 interleaved pairs for each of four cases, 84 pairs total, on d5. Parent is R31B V011. Candidate is source commit `a22a9fb3` with Source SHA above. The Local label `V016` identifies the generic Runner's Candidate slot, not the N01 revision. Values are device-event microseconds; delta is Candidate minus Parent.

| Case | Parent median | Candidate median | Paired delta median | Delta range | Pairs |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 6.200 | 5.980 | +0.120 | -216.240 to +125.680 | 21 |
| FP16 D=32768 | 10.060 | 10.160 | +0.160 | -0.340 to +103.480 | 21 |
| BF16 D=12288 | 6.820 | 6.880 | +0.020 | -2.240 to +42.260 | 21 |
| BF16 D=32768 | 13.980 | 14.100 | 0.000 | -160.080 to +0.700 | 21 |

`LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`. All 84 raw pairs are retained. The opposing outliers are not filtered and no performance gain is claimed.

## Device State and Evidence

Before Local, d5 showed HBM `91%`, AICore `0%`, and the existing `VLLMWorker_TP` process (PID 3355594, 56354 MB). The same process remained after Local and was left unchanged. Pre/post snapshots and build, Correctness, Local, and device-code evidence are in `results/`.

Remote run: `~/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V007/a22a9fb3`

NEXT_ACTION: continue on d5 with V008, testing a distinct Pass 1 `xFp32` view-lifetime change.
