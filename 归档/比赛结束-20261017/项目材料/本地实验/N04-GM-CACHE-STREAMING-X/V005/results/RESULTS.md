# N04 V005 Results

## Source

- Route: N04-GM-CACHE-STREAMING-X
- Branch: `route/n04-gm-cache-streaming-x-20261011`
- Parent revision: `409df41bee7306ee1ddea8de59e9d0008138b43c` (V004 source)
- Candidate commit: `4d10c4113ce8fb5a6cff52d451f5d77c8f48d077`
- Paired harness commit: `5cc43671434d16050539cee4e60f4e2ca3588e19`
- Candidate source: `本地实验/N04-GM-CACHE-STREAMING-X/V005/submission.asc`
- Source SHA256: `e436967292ce60c958da592f8e62106244b54b8e82169234c7a85f46494c36a9`
- Parent source SHA256: `efaae996d2851b408bb690b13103410a32f65dfe029973ebea7a688eda8f4f35`
- Change: replace the V004 output-path `CACHE_MODE_NORMAL` with one existing `outputGm_.SetL2CacheHint(AscendC::CacheMode::CACHE_MODE_PERSISTENT)` mode.
- The input `CACHE_MODE_DISABLE` remains unchanged. No new Hint call, arithmetic, tiling, prefetch, DMA descriptor, or parameter-residency change.
- Remote source/dependency evidence: `upload-verification.txt`, return code 0.

## Similarity Fact

- Main audit reports ten FULL_SOURCE_SIMILARITY values around `0.982-0.999`, above the 80% limit, because the shared Champion body remains present. This Route did not alter names, comments, formatting, or add meaningless wrappers to lower that value.
- The executable change is the existing output cache-mode enum replacement described above. Effective-change similarity was not independently measured in this Route and is left for the separate audit.

## Compile

- Host: `hwnput3` through `cann-server3`; CANN package: `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`; device target: d1 / Ascend910B3.
- Remote run directory: `~/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V005/4d10c411`.
- Adapter/device CMake configure and `full_link`: PASS, return code 0.
- Existing paired CMake configure and `paired_runner_v016` build: PASS, return code 0, using the current V004/V005 source directory and system C++ include path.
- Paired executable: 588736 bytes. Adapter outputs: `device.alink` and `submission.alink`, each about 180 KiB. Only these compact outputs were copied to `results/artifacts/`.
- Raw compile evidence: `compile.log` and `paired-build.log`.

## Correctness

- Command: `./paired_runner_v016 --correctness`.
- Loader: CANN environment with `/usr/lib/aarch64-linux-gnu` first in `LD_LIBRARY_PATH`.
- Result: PASS, return code 0; all 4 cases completed, V004 Parent and V005 Candidate each reported `failures=0`.
- Cases: FP16 `1024x12288`, FP16 `1024x32768`, BF16 `1024x12288`, BF16 `1024x32768`.
- Raw evidence: `correctness.log`; pre-run state: `pre-correctness-d1.txt`.

## Paired Local

- Command: `./paired_runner_v016 --paired`; device events are primary and wall time is diagnostic.
- Each case has 21 alternating V004 Parent/V005 Candidate pairs after 45 warmups; rows 1024; width 32768 is the large-working-set case.
- Raw evidence: `local-paired.log`; pre/post state: `pre-local-d1.txt`, `post-local-d1.txt`.

| Case | Samples | V004 median us | V005 median us | Delta median us | Delta MAD us | V004 CV | V005 CV |
|---|---:|---:|---:|---:|---:|---:|---:|
| FP16 D=12288 | 21 | 446.660 | 399.380 | -49.040 | 50.320 | 0.29923 | 0.20984 |
| FP16 D=32768 | 21 | 1098.200 | 1127.220 | 40.000 | 137.260 | 0.11377 | 0.28371 |
| BF16 D=12288 | 21 | 529.600 | 525.700 | 1.320 | 14.220 | 0.26909 | 0.25265 |
| BF16 D=32768 | 21 | 1682.300 | 1693.460 | 11.100 | 94.600 | 0.30203 | 0.08253 |

- `LOCAL_SCORE=NONE`: the paired deltas include large outliers, from `-1770.720` to `1124.680` us, and d1 carried sustained unrelated load. The FP16 tail median is smaller than its MAD in magnitude; no stable benefit is claimed for `CACHE_MODE_PERSISTENT`.
- A separate external performance Runner was present in the d1 pre-Local snapshot and was allowed to finish before this Route's Local began; no other process was terminated or changed.
- Runner return code: 0. Post-run process query found no V005 Runner.

## Device And Evidence

- d1 pre-Local snapshot: HBM `59718/65536 MiB` used, about `5818 MiB` free, AICore `33%`; post-Local HBM remained `59718/65536 MiB` used and AICore was `28%`. Existing workloads remained and were left unchanged.
- Host disk: about `34 GB` free on `/home/data4t2`; only small logs, snapshots, and compact binaries were copied locally.
- Evidence files in this directory include source upload verification, compile logs, correctness raw output, Local raw output, device snapshots, and the three compact artifacts.

## Next Action

Retain the FULL_SOURCE_SIMILARITY over-limit fact and the `LOCAL_SCORE=NONE` result. Any further revision must change an existing executable GM/L2 choice only, without adding a Hint or changing DMA, tiling, arithmetic, or parameter residency.
