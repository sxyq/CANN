# N02-CODE-FOOTPRINT-ICACHE-X V010

| Field | Value |
|---|---|
| ROUTE | N02-CODE-FOOTPRINT-ICACHE-X |
| REVISION | V010 |
| PARENT_REVISION | V009 |
| PARENT_SOURCE_COMMIT | 389db691e67c0941989761e80934168498d831fe |
| PARENT_DEVICE_REFERENCE | R31B V011 / Official 45.16 |
| CANDIDATE_SOURCE_COMMIT | 89b13447234890a5c1b1203885fd7ccbc49f5bc7 |
| SOURCE_SHA256 | 22cb1a23baa96a4864913f8c8189cd3ab8ae2df0fd361d456c3e65d17185845a |
| SOURCE | `/Users/sunyiyang/Desktop/Project/cann/worktrees/n02-code-footprint-icache/本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V010/submission.asc` |
| REMOTE_HOST | cann-server3 |
| SOC | Ascend910B3 |
| DEVICE_ID | d6 / NPU 6 |
| TOOLCHAIN | CANN 8.5.0.alpha002 |
| REMOTE_RUN | `/home/data4t2/lelinfeng/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V010/22cb1a23baa96a4864913f8c8189cd3ab8ae2df0fd361d456c3e65d17185845a` |

## Change

The pass-1 traversal now has an explicit first load, a steady loop for intermediate units, and a separate final unit. Each unit retains its flattened index calculation, row/tile address expressions, DataCopy calls, and per-unit event order. This changes the control-flow layout and the emitted device code. No host-only layout change was made. The source-level transfer order is retained; available disassembly did not decode instruction words well enough to verify compiler scheduling of MTE and Vector instructions. Treat this as a control-flow/code-layout experiment with scheduling interaction unverified, not as a clean I-cache attribution.

## Compile And Device Code

Parent and Candidate configure/full-link returned 0. The existing V016 paired Runner configure returned 0. Its first build attempt returned 2 because the compiler could not locate `<vector>` in generated registration source; no device command ran. The second build used the existing GCC 11 `CPATH` setting and returned 0. Both attempts are retained in the compile logs and status files.

| Artifact | Parent V011 | Candidate V010 | Difference |
|---|---:|---:|---:|
| `device.cpp.o` | 334040 B | 337880 B | +3840 B |
| `device.alink` | 183144 B | 187240 B | +4096 B |
| FP16 `ProcessWideLowPrecision` | 0x1234 B | 0x19d8 B | +0x7a4 B |
| BF16 `ProcessWideLowPrecision` | 0x1248 B | 0x16c0 B | +0x478 B |

Candidate `.o` and `.alink` contents differ from both V009 and V006 Candidate artifacts. The saved symbol tables and full-link outputs therefore confirm a new device implementation, rather than a source-only or host-only change.

## Correctness

The existing V016 paired Runner returned 0 on d6. Parent V011 and Candidate V010 passed all four FP16/BF16 cases at widths 12288 and 32768.

- CORRECTNESS: PASS 4/4
- CORRECTNESS_RC: 0

## Local

The existing paired Runner used 45 warmups and 21 interleaved pairs per case on d6. Raw samples and summaries are in `results/local/paired-local.log`; return status is in `results/local/paired-local-status.txt`.

| Case | Parent median us | Candidate median us | Paired delta median us | Delta p10..p90 us | Delta min..max us | Delta MAD us |
|---|---:|---:|---:|---:|---:|---:|
| FP16 tail D=12288 | 6.260 | 6.100 | -0.100 | -0.640..9.000 | -1.760..122.540 | 0.340 |
| FP16 wide D=32768 | 10.120 | 9.840 | -0.100 | -0.480..0.420 | -0.760..132.460 | 0.320 |
| BF16 tail D=12288 | 6.860 | 6.400 | -0.260 | -0.660..0.560 | -12.100..62.240 | 0.180 |
| BF16 wide D=32768 | 13.900 | 12.960 | -0.920 | -1.380..1.240 | -5.520..9.560 | 0.220 |

Before and after sampling, d6 used about 59876/65536 MB HBM (91%) and AICore usage was 0%. `VLLMWorker_TP` PID 3357697 held 56355 MB throughout. Large positive outliers and mixed paired ranges make the medians descriptive only.

- LOAD_QUALITY: LOW
- LOCAL_SCORE: NONE
- LOCAL_DELTA: NONE
- INTERPRETATION: Noise and occupied HBM prevent a performance conclusion.

## Evidence

- Compile logs, object files, linked files, symbols, and disassembly: `results/compile/` and `results/artifacts/`.
- Correctness raw output and status: `results/correctness/`.
- Local raw samples and status: `results/local/`.
- d6 load records: `results/snapshots/`.

V001-V005 Official negative results remain unchanged. V008 remains a negative code-generation result because its Candidate object and linked artifact matched V006.
