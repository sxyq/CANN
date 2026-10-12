# N01 V021 Results

ROUTE: N01-REGISTER-LIVE-RANGE-X
REVISION: V021
SOURCE_PARENT_REVISION: V020 (`934e9c99ef531799c4cb9645f45372909bef5248`)
SOURCE_COMMIT: `c50d6f4a7f466fbde2c2b92eb544082cf3a1a0b5`
SOURCE_SHA256: `d4f9994b9eb43e75b454143cbdb3f2b8a2222545d6ad4496fc6022c1682e5134`
SOURCE_PATH: `本地实验/N01-REGISTER-LIVE-RANGE-X/V021/submission.asc`
MEASUREMENT_PARENT: R31B V011
MEASUREMENT_PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
DEVICE: d5 / NPU 5 / `hwnput3` / Ascend910B3

## Change

The source change is in the FP32 path `ProcessWideFp32FullCacheRows`: the square operation writes `valueTile * valueTile` to `residualLocal`, then `ReduceSum` reads that tile and uses the no-longer-needed `xLocal` as scratch. This tests reuse of a dead FP32 temporary during reduction. No Scalar address generation, UB allocation, core assignment, arithmetic formula, or DMA ordering changed.

## Build And Device Code

- CANN `8.5.0.alpha002`, Ascend910B3, device 5. Configure, compile, and device-code extraction returned `0`.
- Candidate input: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V021/c50d6f4a/candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc`; build record identifies its source as V021 with SHA256 above.
- Measurement Parent input: `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V004/e833b161/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc`, SHA256 above.
- Existing Runner target `paired_runner_v016`; before the FP32 extension build, its binary SHA256 was `bbd673d981aaa371e1d65a288c7b7939b69c2afcffa1a8d9e6e0e0e47e794bbf`.
- FP32 extension build on 2026-10-12 reused the same target and build directory with the previously recorded system C++ include paths. Build returned `0`; resulting Runner binary SHA256 is `88025ed8e024a4b223f289799a038d82b1837daf26bc97bffc6a337f06436730`. Candidate and Parent source SHA values remained unchanged.
- Extracted AICore binary SHA256 `fe3c6595abd33f115b03ee03b5b87b8e81be8ee2b7db1ec61d8d084832258323`; it differs from V020. Register count is unknown.
- Runner labels `V001` in Correctness and `V016` in Local are generic slots. Actual Candidate input in both runs is the V021 build above; Local Parent is R31B V011.
- Committed build and device-code evidence: `results/compile-v021.command`, `results/build-v021.log`, `results/device-code-v021.command`, and `results/device-code-v021.log`. The extracted binary remains locally at `results/aicore_binary_v021.bin`; the repository's existing `*.bin` ignore rule excludes it from the commit.

## Correctness

- d5: PASS, 4/4 cases; Runner exit `0`.
- Raw output: `results/correctness-v021.raw.log`.
- Candidate source and Runner identity: `results/correctness-v021.command`.
- Coverage note: these four cases are FP16/BF16. They do not execute the changed FP32 function and are not correctness evidence for the source change described above.

## Paired Local

The existing Runner completed 21 interleaved pairs per case (84 total) on d5, exit `0`. Times and paired deltas are device-event microseconds; delta is Candidate minus Parent. Every sample is preserved in `results/local-v021.raw.log`.

| Case | Parent median (p10-p90; min-max) | Candidate median (p10-p90; min-max) | Paired delta median (p10-p90; min-max) |
|---|---|---|---|
| FP16 D=12288 | 6.600 (5.540-15.660; 5.500-19.440) | 6.100 (5.720-14.200; 5.540-43.860) | +0.160 (-6.080-+7.460; -13.340-+38.040) |
| FP16 D=32768 | 10.240 (9.700-11.780; 9.600-198.080) | 10.380 (9.760-11.900; 9.640-210.900) | +0.060 (-0.300-+1.060; -188.440-+200.320) |
| BF16 D=12288 | 6.800 (6.340-7.140; 6.280-7.320) | 6.580 (6.200-7.140; 6.020-7.500) | -0.160 (-0.660-+0.420; -0.780-+0.720) |
| BF16 D=32768 | 14.020 (13.580-14.780; 13.320-56.980) | 13.400 (12.800-17.680; 12.720-110.720) | -0.520 (-1.520-+3.380; -43.120-+79.540) |

FP16 D=12288, FP16 D=32768, and BF16 D=32768 contain severe event-time outliers and broad paired-delta tails. BF16 D=12288 is comparatively stable but its -0.160 us paired median is small and the p10-p90 interval crosses zero. `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`; no gain is claimed and no sample was removed.

## FP32 Path Verification Attempt

- The FP32 case was selected from N01 source dispatch: `D=12288` exceeds `kCacheElems=8192`, and the source uses 4096-element tiles, so this input enters `ProcessWideFp32FullCacheRows` and runs three tile iterations. This selection does not assume any Judge shape/dtype mapping.
- Existing `paired_runner_v016` has only FP16/BF16 case definitions. Its existing source was extended locally at `results/paired_runner-v021.cpp` with FP32 input/reference support and FP32-only modes. Candidate kernel source was unchanged; local and remote Candidate SHA256 remain `d4f9994b9eb43e75b454143cbdb3f2b8a2222545d6ad4496fc6022c1682e5134`.
- Runner source SHA256: original `4f6701ea7edbbcfd3e410e22c0c3ea39a25bc4996b9bf05be4c422827c56f789`; FP32-enabled source `7836fb57a342642132a3345209796665b7fa900b599d3842f37560b446b5ba3b`.
- Earlier build failures remain recorded: the first attempt lacked `<vector>`; retry with system C++ include directories failed when `/home/data4t2` had no free space. On 2026-10-12, the same build target succeeded when the volume reported about 6.8 GB available. No files were deleted or cleaned.
- Reusing the same target, build directory, and system C++ include paths succeeded on 2026-10-12 (exit `0`). The new Runner binary SHA256 is `88025ed8e024a4b223f289799a038d82b1837daf26bc97bffc6a337f06436730`. Pre-build free space was `6,816,476 KiB` on `/home/data4t2` and `88,273,860 KiB` on `/`; post-build free space was `6,817,936 KiB` and `88,273,588 KiB`, respectively.
- FP32 Correctness was attempted on d5 using `[2,12288]`, dtype FP32, with the recorded Candidate and R31B V011 source identities. The Runner launched Parent and Candidate, synchronized both, then the Parent comparison failed first: `failures=15389 max_abs=0.092312813 max_rel=29.889896`; exit code `3`. Since `CheckOutput` exits immediately on Parent failure, Candidate output was not compared. This result does not establish whether the V021 Candidate passes or fails correctness.
- FP32 paired Local was not run. No FP32 timing samples exist. The 45 warmups and 21 paired samples remain pending until a correctness run can compare both Parent and Candidate successfully.
- Build commands/logs, binary identities, device snapshots, and the unchanged Candidate identity are retained in the `fp32-*v021*` evidence files. No Candidate source was changed.

## One-Time FP32 Runner Diagnosis

- No FP32 `CaseSpec` exists in the shared pre-extension R31B paired Runner; its cases are FP16/BF16. The V011 kernel ABI explicitly maps dtype `0` to FP32 and supports flattened leading dimensions. The extension case is `RunCase<float>` with data shape `[2,12288]`, parameter shape `[12288]`, epsilon `1e-5`, and core request `8`. V011 dispatches `D=12288` to its wide FP32 path; the 4096-element tile yields three iterations. No pre-existing V011-specific FP32 Runner configuration was found in the shared files examined.
- Remote CMake cache configures `R31B_V011_SOURCE_DIR` as `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V004/e833b161/parent` and `R31B_V016_SOURCE_DIR` as `/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V021/c50d6f4a/candidate`. The compiled dependency files name those exact Parent and Candidate `.asc` files and the V021 support `paired_runner.cpp`. Recorded source SHA256 values are Parent `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`, Candidate `d4f9994b9eb43e75b454143cbdb3f2b8a2222545d6ad4496fc6022c1682e5134`, Runner source `7836fb57a342642132a3345209796665b7fa900b599d3842f37560b446b5ba3b`, and wrapper `b827960aa2d60e6efd953b30f3ecfcfab17af5111f4d03e5309c4c94c68d1a55`. The built executable remains `88025ed8e024a4b223f289799a038d82b1837daf26bc97bffc6a337f06436730`.
- `RunCase` allocates `parentOutput` and `candidateOutput` separately, each `98304` bytes, and keeps both allocations live through both launches. Each kernel receives its corresponding output pointer. The Runner does not print the runtime pointer values. It synchronizes both kernels and copies both outputs to host; `CheckOutput` evaluates V011 first and exits immediately on its failure, before evaluating the Candidate buffer.
- The host reference uses FP32 inputs directly and computes `y=x+residual`, `sum(y*y)/D + epsilon`, reciprocal square root, then `y*invRms*gamma+bias`, matching the V011 FP32 operation sequence. Its sum accumulator is `float`, not a wider CPU accumulator. The Runner sets the same `1e-4` value for its absolute and relative terms and requires every element to pass. No V011-specific FP32 threshold record was found. The generic float precision guide uses different mixed thresholds and also caps maximum absolute error at `1e-2`; the observed `.092312813` exceeds that cap, so threshold choice alone does not explain the Parent failure. The raw output contains no per-element values or error distribution, preventing a more specific diagnosis.
- No input, source path, output-buffer, or formula mismatch was found. No test parameter or Candidate source was changed, and the failed Correctness command was not repeated. Record the FP32 attempt as an invalid measurement chain stopped at the V011 Parent/reference comparison; Candidate correctness is unknown and paired Local remains unrun.

## Device State And Evidence

Before sampling on d5: HBM use `91%`, AICore use `0%`, temperature `39 C`; existing `VLLMWorker_TP` PID `3355594` used `56354 MB`. After sampling: HBM use `91%`, AICore use `0%`, temperature `42 C`; the same process remained at `56354 MB`. No other process was changed; the paired Runner exited.

After the failed FP32 Runner build, d5 still showed HBM use `91%`, AICore use `0%`, and only the existing `VLLMWorker_TP` PID `3355594` at `56354 MB`; no N01 Runner was present. This is a post-build snapshot, not a pre-run snapshot for FP32 testing.

The user-provided live reading before this attempt was d5 HBM `59876/65536 MiB` with AICore `0%`. For the 2026-10-12 FP32 attempt, pre-build and pre-Correctness snapshots reported HBM use `91%`, AICore use `0%`, with the same VLLMWorker PID and memory use. The post-Correctness snapshot again showed HBM `91%`, AICore `0%`, the same VLLMWorker, and no paired Runner process. The VLLMWorker was not modified.

The live pre-run snapshot is `results/pre-local-device-recheck-01.txt`; the post-run snapshot is `results/post-local-device.txt`. The earlier snapshot remains at `results/pre-local-device.txt`.

Full evidence includes `results/local-v021.command`, `results/local-v021.raw.log`, `results/pre-local-device-recheck-01.command`, `results/pre-local-device-recheck-01.txt`, `results/post-local-device.command`, `results/post-local-device.txt`, `results/correctness-v021.command`, and `results/correctness-v021.raw.log`.

LOW_PRECISION_LOCAL_SCORE: NONE
FP32_CORRECTNESS: MEASUREMENT_CHAIN_INVALID; V011 Parent/reference comparison failed with 15,389 failures, max_abs=0.092312813, max_rel=29.889896, exit=3; Candidate comparison not reached and Candidate correctness unknown.
FP32_LOCAL: NOT RUN (V011 Parent reference failed; Candidate output was not compared)
NEXT_ACTION: do not repeat the unchanged Correctness command or run paired Local until a concrete Parent/reference diagnosis is available; do not repeat the four completed low-precision cases.
