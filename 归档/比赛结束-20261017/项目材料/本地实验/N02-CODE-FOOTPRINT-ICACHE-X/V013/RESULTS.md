# N02 CODE-FOOTPRINT-ICACHE-X V013

| Field | Value |
|---|---|
| ROUTE | N02 CODE-FOOTPRINT-ICACHE-X |
| REVISION | V013 |
| PARENT_REVISION | V012 |
| PARENT_SOURCE_COMMIT | `3fba7ee209e56b68f4e813ac0817120b4515e366` |
| CANDIDATE_SOURCE_COMMIT | `3b0593944` |
| RUNNER_LABEL_COMMIT | `88e74351` |
| SOURCE_SHA256 | `e596da15bb9f093bf9003a06753daafede6d50c77fa5376a16542c6c6f23d000` |
| SOURCE | `/Users/sunyiyang/Desktop/Project/cann/worktrees/n02-code-footprint-icache/本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V013/submission.asc` |
| REMOTE_HOST | cann-server3 |
| SOC / DEVICE | Ascend910B3 / d6 (NPU 6) |
| TOOLCHAIN | CANN 8.5.0.alpha002 |
| REMOTE_RUN | `/home/data4t2/lelinfeng/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V013/e596da15bb9f093bf9003a06753daafede6d50c77fa5376a16542c6c6f23d000` |

## Change

V013 explicitly places two adjacent steady-state tiles in one loop body. Each tile retains the prior wait, successor parameter loads, conversion, apply, and release order. The original one-tile steady loop and terminal path handle the final one or two tiles. The experiment changes generated control-flow layout and code volume; it does not alter row indexing or parameter addresses. The two-tile body duplicates code, so any timing effect also includes a larger hot function and is not an isolated instruction-cache result.

## Compile And Device Code

Candidate V013 configure/full-link and the existing V016 paired Runner build returned 0 under CANN 8.5.0.alpha002. The paired Runner was rebuilt once after its Candidate display label was changed from V012 to V013. Parent R31B V011 artifacts were reused from the already compiled V012 record; the Parent was not recompiled.

| Artifact | Paired Parent R31B V011 | Candidate V013 | Difference |
|---|---:|---:|---:|
| `device.cpp.o` | 334040 B | 351416 B | +17376 B |
| `device.alink` | 183144 B | 199896 B | +16752 B |
| FP16 `ProcessWideLowPrecision` | 0x1234 B | 0x2a10 B | larger |
| BF16 `ProcessWideLowPrecision` | 0x1248 B | 0x2a70 B | larger |

The candidate object and linked device artifact both compare different from the Parent. Objects, `.alink` files, symbol tables, disassembly text, and byte comparisons are saved in `results/`. The available disassembler does not decode device instruction words; the evidence confirms generated code changed, not an instruction-cache improvement.

## Correctness

The existing V016 Runner passed all four Parent R31B V011 / Candidate V013 cases on d6.

- CORRECTNESS: PASS 4/4
- CORRECTNESS_RC: 0
- RESULT: `results/correctness/paired-correctness.log`

The raw log prints the stale Candidate display name `N02-V012`; this is a Runner label error, not a V012 input. The first successful paired Runner build was recorded at 00:31 UTC. Its ASC translation unit includes `../../../../N02-CODE-FOOTPRINT-ICACHE-X/V013/submission.asc`, and the compiler dependency file records that same V013 path. The remote V013 source SHA in `build-identity.txt` matches `SOURCE_SHA256` above. The Correctness pre-run device snapshot is timestamped 00:34 UTC. The Runner source timestamp shows its display strings were changed at 00:39 UTC, and the label-only rebuild completed afterward at 00:39 UTC. The change from V012 to V013 touched only printed labels, as recorded in Route commit `88e74351`; no kernel source changed. Therefore all four PASS cases validate Candidate V013. The original log is preserved unchanged, and no Correctness rerun was made.

## Local

The existing V016 Runner used 45 warmups and 21 interleaved pairs per case on d6. Device-event time is primary; wall time is diagnostic. All 84 raw pairs are retained in `results/local/paired-local.log`.

| Case | Parent median us | Candidate median us | Paired delta median us | Delta p10..p90 us | Delta min..max us | Delta MAD us |
|---|---:|---:|---:|---:|---:|---:|
| FP16 tail D=12288 | 6.040 | 6.200 | -0.080 | -0.480..0.380 | -65.440..7.940 | 0.260 |
| FP16 wide D=32768 | 10.140 | 10.140 | -0.340 | -62.820..0.380 | -72.660..86.920 | 0.420 |
| BF16 tail D=12288 | 6.700 | 6.660 | -0.200 | -0.480..7.940 | -0.760..8.700 | 0.200 |
| BF16 wide D=32768 | 14.080 | 13.080 | -0.960 | -64.280..-0.440 | -175.960..-0.280 | 0.340 |

d6 reported 91% HBM use and 0% AICore use before Correctness and Local. `VLLMWorker_TP` PID 3357697 held 56355 MB before and after Local. Raw deltas contain large spikes in three cases, including a 175.960 us minimum in BF16 wide. The source of each spike is not known; no samples were removed or selected.

- LOAD_QUALITY: LOW
- LOCAL_SCORE: NONE
- LOCAL_DELTA: NONE
- LOCAL_RC: 0
- INTERPRETATION: Sustained HBM occupancy and extreme paired outliers prevent a performance conclusion.

## Evidence

- Compile logs, status, artifact sizes, symbols, disassembly, and comparisons: `results/compile/` and `results/artifacts/`.
- Parent artifact reuse record: `results/compile/parent-reuse.txt`.
- Correctness raw output and status: `results/correctness/`.
- All paired samples and status: `results/local/`.
- d6 snapshots before correctness, before Local, and after Local: `results/snapshots/`.
