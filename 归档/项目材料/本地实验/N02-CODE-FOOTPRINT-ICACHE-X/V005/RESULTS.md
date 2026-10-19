# N02-CODE-FOOTPRINT-ICACHE-X V005

ROUTE: N02-CODE-FOOTPRINT-ICACHE-X  
REVISION: V005  
PARENT: R31B V011 / Official 45.16  
PARENT_SOURCE_COMMIT: 7132c86d32e8fa1d57e2ae8edb7846e3b4775b2a  
CANDIDATE_SOURCE_COMMIT: 2f0316b103f92978b9be07cce42c45f0566b4e30  
PAIRED_ENTRY_BIND_COMMIT: 87eaba22cd8efe1fd0d74e507207e0f7f627e26b  
BRANCH: route/n02-code-footprint-icache-x-20261011  
REMOTE_HOST: hwnput3  
SOC: Ascend910B3  
DEVICE: d6 / NPU 6  
TOOLCHAIN: CANN 8.5.0.alpha002  
REMOTE_RUN: `/home/data4t2/lelinfeng/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V005/2f0316b103f92978b9be07cce42c45f0566b4e30`

## Change

Pass-2 parameter prefetch changed from two-deep buffering to serial per-tile loads. This changes device control flow and instruction layout, and also reduces overlap between parameter loading and compute. Arithmetic, tile order, output semantics, shapes, dtypes, and launch ABI remain unchanged. The scheduling change is part of this experiment and must be considered when interpreting performance.

## Compile And Device Artifacts

- Parent and Candidate configure plus `full_link`: all return codes 0.
- Existing `paired_runner_v016` configure/build: return codes 0.
- `llvm-objdump` for both device objects: return codes 0; `diff_rc=1` records differing disassembly.

| Artifact | Parent | Candidate | Difference |
|---|---:|---:|---:|
| `device.cpp.o` | 334040 B | 330792 B | -3248 B |
| `device.alink` | 183144 B | 183144 B | 0 B |
| FP16 `ProcessWideLowPrecision` symbol | 0x1234 B | 0x0dd4 B | -0x460 B |
| BF16 `ProcessWideLowPrecision` symbol | 0x1248 B | 0x0de8 B | -0x460 B |

The parent and Candidate object files and linked device files differ in content. Object sizes, symbol tables, sections, disassembly, disassembly diff, artifact identities, and the four device files are retained under `results/compile/` and `results/artifacts/`.

## Correctness

The existing paired runner passed all four FP16/BF16 cases on d6 for widths 12288 and 32768. Parent V011 and N02-V005 each passed every case with zero failures. Runner return code: 0. Raw output and status are in `results/correctness/`.

## Local

The existing paired runner used 45 warmups and 21 interleaved pairs per case on d6. It compared Parent V011 with Candidate V005. Runner return code: 0; all four cases completed.

| Case | Parent median (us) | Candidate median (us) | Paired delta median (us) | Delta p10 / p90 (us) |
|---|---:|---:|---:|---:|
| FP16, width 12288 | 6.220 | 6.080 | -0.160 | -12.260 / +10.580 |
| FP16, width 32768 | 10.120 | 10.120 | +0.080 | -0.240 / +0.420 |
| BF16, width 12288 | 6.780 | 6.780 | -0.060 | -0.500 / +0.780 |
| BF16, width 32768 | 13.880 | 14.040 | +0.120 | -0.460 / +0.300 |

Several cases contain large outliers: paired deltas range from -92.440 to +135.400 us for FP16 width 12288, -189.280 to +140.620 us for BF16 width 12288, and -52.920 to +0.920 us for BF16 width 32768. Before Local, d6 used 59875/65536 MB HBM (5661 MB available), AICore was 0%, and an existing `VLLMWorker_TP` process used 56355 MB. The same process remained after Local. These conditions and outliers make the medians diagnostic only.

LOCAL_SCORE=NONE  
LOCAL_DELTA=NONE  
LOAD_QUALITY=LOW

The raw paired samples and complete summaries are in `results/local/paired-local.log`; device and process snapshots are in `results/snapshots/`.

## Follow-On Launch Attempt

After the completed 4x21 run, one additional invocation failed before the runner entered device code: the dynamic loader could not locate `libgraph.so`, return code 127. This failure is separately retained as `results/local/paired-local-rerun-01.log` and `paired-local-rerun-01-status.txt`; it is excluded from the Local sample summaries. The accompanying snapshots are named `*-rerun-01-*`.
