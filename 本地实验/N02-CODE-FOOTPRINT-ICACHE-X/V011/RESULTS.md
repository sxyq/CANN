# N02-CODE-FOOTPRINT-ICACHE-X V011

| Field | Value |
|---|---|
| ROUTE | N02-CODE-FOOTPRINT-ICACHE-X |
| REVISION | V011 |
| PARENT_REVISION | V010 |
| PARENT_SOURCE_COMMIT | 89b13447234890a5c1b1203885fd7ccbc49f5bc7 |
| PARENT_DEVICE_REFERENCE | R31B V011 / Official 45.16 |
| CANDIDATE_SOURCE_COMMIT | d8b1331eaf91697cc5bd928ff9ac55a9906b88e1 |
| SOURCE_SHA256 | 957083ebcaf098d6e9ffcd4531714c90d38a6d1e6bfb244f8eec4851eaced9d7 |
| SOURCE | `/Users/sunyiyang/Desktop/Project/cann/worktrees/n02-code-footprint-icache/本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V011/submission.asc` |
| REMOTE_HOST | cann-server3 |
| SOC | Ascend910B3 |
| DEVICE_ID | d6 / NPU 6 |
| TOOLCHAIN | CANN 8.5.0.alpha002 |
| REMOTE_RUN | `/home/data4t2/lelinfeng/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V011/957083ebcaf098d6e9ffcd4531714c90d38a6d1e6bfb244f8eec4851eaced9d7` |

## Change

The Candidate extracts the per-row compute and Store body from the wide low-precision pass-2 tile loop into `ApplyWideLowPrecisionOutput`. The surrounding tile and row traversal, parameter prefetch, addresses, and source ordering of conversions, arithmetic, PipeBarrier, MTE synchronization, and Store remain unchanged.

The compiler emitted `ApplyWideLowPrecisionOutput` as separate FP16 and BF16 device functions, so this is a real device code-layout change rather than a source-only split. The helper boundary also introduces a call path; any timing effect could include call overhead as well as instruction placement. Available `llvm-objdump` output labels device instruction words `<not available>`, so emitted instruction scheduling cannot be audited from that disassembly. Do not treat V011 as an isolated I-cache measurement.

## Compile And Device Code

Parent and Candidate configure/full-link, plus the existing V016 paired Runner configure/build, all returned 0.

| Artifact | Parent V011 | Candidate V011 | Difference |
|---|---:|---:|---:|
| `device.cpp.o` | 334040 B | 339432 B | +5392 B |
| `device.alink` | 183144 B | 187608 B | +4464 B |
| FP16 `ProcessWideLowPrecision` | 0x1234 B | 0x1818 B | +0x5e4 B |
| BF16 `ProcessWideLowPrecision` | 0x1248 B | 0x161c B | +0x3d4 B |
| FP16 `ApplyWideLowPrecisionOutput` | absent | 0x160 B | emitted |
| BF16 `ApplyWideLowPrecisionOutput` | absent | 0x138 B | emitted |

Candidate object and linked artifact contents differ from V006, V009, and V010. Raw objects, linked files, symbol tables, and disassembly text are saved under `results/`.

## Correctness

The existing V016 paired Runner returned 0 on d6. Parent V011 and Candidate V011 passed all four FP16/BF16 cases at widths 12288 and 32768.

- CORRECTNESS: PASS 4/4
- CORRECTNESS_RC: 0

## Local

The existing paired Runner used 45 warmups and 21 interleaved pairs per case on d6. Raw samples and summaries are in `results/local/paired-local.log`; status is in `results/local/paired-local-status.txt`.

| Case | Parent median us | Candidate median us | Paired delta median us | Delta p10..p90 us | Delta min..max us | Delta MAD us |
|---|---:|---:|---:|---:|---:|---:|
| FP16 tail D=12288 | 6.640 | 6.180 | -0.120 | -9.160..54.980 | -258.960..166.060 | 0.600 |
| FP16 wide D=32768 | 10.260 | 9.880 | -0.040 | -57.880..0.460 | -132.540..71.260 | 0.380 |
| BF16 tail D=12288 | 6.840 | 6.340 | -0.340 | -0.640..0.020 | -9.760..56.180 | 0.220 |
| BF16 wide D=32768 | 13.860 | 13.240 | -0.800 | -0.960..54.460 | -65.820..114.980 | 0.280 |

Before and after sampling, d6 used about 59876/65536 MB HBM (91%, 5660 MB free) with AICore at 0%. `VLLMWorker_TP` PID 3357697 held 56355 MB throughout. Large and mixed-sign outliers make the median differences descriptive only.

- LOAD_QUALITY: LOW
- LOCAL_SCORE: NONE
- LOCAL_DELTA: NONE
- INTERPRETATION: Noise and active HBM occupancy do not support a performance conclusion.

## Evidence

- Compile logs, object files, linked files, symbols, and disassembly: `results/compile/` and `results/artifacts/`.
- Correctness raw output and status: `results/correctness/`.
- Local raw samples and status: `results/local/`.
- d6 load records: `results/snapshots/`.

V001-V005 Official negative results remain unchanged. V008 remains a negative code-generation result because its Candidate object and linked artifact matched V006.
