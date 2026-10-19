# N02-CODE-FOOTPRINT-ICACHE-X V004

ROUTE: N02-CODE-FOOTPRINT-ICACHE-X  
REVISION: V004  
PARENT_SOURCE_COMMIT: 7132c86d32e8fa1d57e2ae8edb7846e3b4775b2a  
CANDIDATE_COMMIT: 2746c556ae46b9a67fd9b326701a3e48c2934306  
SUPPORT_BIND_COMMIT: 54d920ed00a8d6a6bdf748ec6df4424362c1d2c6  
D6_RUNNER_COMMIT: 92acba5c697133269e8cf4e05001b46d95fdb2d2  
BRANCH: route/n02-code-footprint-icache-x-20261011  
REMOTE_HOST: hwnput3  
SOC: Ascend910B3  
DEVICE: d6 / NPU 6  
TOOLCHAIN: CANN 8.5.0.alpha002  
REMOTE_RUN: `/home/data4t2/lelinfeng/cann/server_runs/N02-CODE-FOOTPRINT-ICACHE-X-RESET-20261011/V004/2746c556ae46b9a67fd9b326701a3e48c2934306`

## Change

In `ProcessWideLowPrecision` pass 1, the first x/residual tile load and its `MTE2_V` ready event now sit before the double-buffer loop. The loop no longer contains the `u == 0` load branch. Tile order, arithmetic, event pairing, output semantics, shapes, dtypes, and launch ABI are unchanged. This is a device-side loop-entry layout change; no host dispatch was added.

## Compile

- Parent and Candidate `full_link`: PASS, return code 0 for each.
- Existing `paired_runner_v016` configure: PASS, return code 0.
- First paired build: return code 2 because the C++ plugin could not find `<vector>` without the system C++ include path. The failed output is retained in `results/compile/paired-build.log`.
- Retry with the existing GCC 11 include paths in `CPATH`: PASS, return code 0. Rebuilt after pinning the existing runner to device 6: PASS, return code 0.
- `llvm-objdump` completed for both device objects. `OBJDUMP_DIFF_RC=1` indicates differing disassembly output, not an objdump failure.

## Device Artifacts

| Artifact | Parent | Candidate | Difference |
|---|---:|---:|---:|
| `device.cpp.o` | 334040 B | 334000 B | -40 B |
| `device.alink` | 183144 B | 183144 B | 0 B |
| FP16 `ProcessWideLowPrecision` symbol | 0x1234 B | 0x1260 B | +44 B |
| BF16 `ProcessWideLowPrecision` symbol | 0x1248 B | 0x1240 B | -8 B |

The two hot symbols together grew by 36 B. Both object and linked device artifact contents differ. Artifact identities are in `results/compile/artifact-sha.txt`; Parent/Candidate objects and `.alink` files are retained under `results/artifacts/`. Raw disassembly, section tables, symbol tables, and the diff are retained under `results/compile/`.

## Correctness

The existing paired runner ran on d6 with rows=2 for FP16/BF16 at widths 12288 and 32768. Parent and V004 passed all four cases with zero failures. Runner return code: 0. Raw output and status are in `results/correctness/`.

## Local

The existing paired runner ran on d6 with 45 warmups and 21 interleaved pairs for each of four cases. Runner return code: 0. Raw samples and summaries are in `results/local/paired-local.log`.

| Case | Parent median (us) | V004 median (us) | Paired delta median (us) | Delta p10 / p90 (us) |
|---|---:|---:|---:|---:|
| FP16, width 12288 | 6.160 | 6.960 | +0.200 | -0.360 / +25.820 |
| FP16, width 32768 | 10.060 | 9.960 | -0.020 | -0.540 / +0.360 |
| BF16, width 12288 | 6.780 | 6.400 | -0.300 | -0.600 / +118.260 |
| BF16, width 32768 | 13.960 | 13.120 | -0.860 | -1.520 / -0.300 |

Large outliers appeared in three cases, including device samples above 100 us. The d6 snapshots show 59876/65536 MB HBM in use (5660 MB available) and an existing `VLLMWorker_TP` using 56355 MB; this process was left unchanged. Timing variation makes these medians diagnostic only: `LOCAL_SCORE=NONE`, `LOCAL_DELTA=NONE`.

## Snapshots

Compile 前快照以及 Correctness、Local 的前后 UTC/device/process 记录均在 `results/snapshots/`。paired runner 使用 `kDevice=6` 构建；未使用其他设备。
