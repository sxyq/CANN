# N04 V003 Results

## Source

- Route: N04-GM-CACHE-STREAMING-X
- Branch: `route/n04-gm-cache-streaming-x-20261011`
- Parent revision: `46ee7f3d3cba1c781c3ce68a8dfdd7251a641247` (V002 source)
- Candidate commit: `6c53cf0e9410825cfae2edf38b7da1502b3afa9e`
- Paired harness commit: `4857b562`
- Candidate source: `本地实验/N04-GM-CACHE-STREAMING-X/V003/submission.asc`
- Source SHA256: `c00d8d4d5ea1c16636c1df1f0c541f42a4bdbde1e4c21c4f434e723e2cabb6db`
- Parent source SHA256: `dcbc9bfbb710daa6829922dac2a762e84cbc54b32f30d64ac6397c056a2dc86e`
- Change: replace the V002 output-path `CACHE_MODE_DISABLE` with one `outputGm_.SetL2CacheHint(AscendC::CacheMode::CACHE_MODE_LAST)` call in `ProcessWideLowPrecision`.
- V001 input `CACHE_MODE_DISABLE` remains unchanged. No arithmetic, tiling, prefetch, DMA descriptor, or parameter-residency change.
- CANN 8.5 header confirms `CACHE_MODE_LAST` is available. Remote source and dependency verification: `upload-verification.txt`, return code 0.

## Compile

- Host: `hwnput3` through `cann-server3`; CANN package: `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`; device target: d1 / Ascend910B3.
- Remote run directory: `~/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V003/6c53cf0e`.
- Adapter/device configure passed. The first `full_link` build returned 2 because the existing Adapter/Device source includes the seed by filename from its support directory; the seed was then placed in that support directory and the same target returned 0.
- Existing paired CMake configure and `paired_runner_v016` build passed, return code 0, with the current V002/V003 source directory supplied through existing CMake cache values and `CPATH` set for system C++ headers.
- Paired executable: 588736 bytes. Adapter outputs: `device.alink` and `submission.alink`, each about 180 KiB. Only these compact outputs were copied to `results/artifacts/`.
- Raw compile evidence: `compile.log` and `paired-build.log`.

## Correctness

- Command: `./paired_runner_v016 --correctness`.
- Loader: CANN environment with `/usr/lib/aarch64-linux-gnu` first in `LD_LIBRARY_PATH`.
- Result: PASS, return code 0; all 4 cases completed, V002 Parent and V003 Candidate each reported `failures=0`.
- Cases: FP16 `1024x12288`, FP16 `1024x32768`, BF16 `1024x12288`, BF16 `1024x32768`.
- Raw evidence: `correctness.log`; pre-run state: `pre-correctness-d1.txt`.

## Paired Local

- Command: `./paired_runner_v016 --paired`; device events are primary and wall time is diagnostic.
- Per case: 45 warmups and 21 alternating V002 Parent/V003 Candidate pairs; rows 1024; width 32768 is the large-working-set case.
- Raw evidence: `local-paired.log`; pre/post state: `pre-local-d1.txt`, `post-local-d1.txt`.

| Case | V002 median us | V003 median us | Delta median us | Delta MAD us |
|---|---:|---:|---:|---:|
| FP16 D=12288 | 386.600 | 433.700 | 15.880 | 70.540 |
| FP16 D=32768 | 1026.260 | 1046.020 | 37.500 | 120.520 |
| BF16 D=12288 | 489.020 | 488.840 | 8.040 | 52.920 |
| BF16 D=32768 | 1581.800 | 1682.940 | 88.260 | 43.220 |

- `LOCAL_SCORE=NONE`: the paired deltas include large positive and negative outliers under sustained d1 load. No stable benefit is claimed from the output `CACHE_MODE_LAST` trial.
- Runner return code: 0. Post-run process query found no N04 Runner.

## Device And Evidence

- d1 pre-Local snapshot: HBM `59718/65536 MiB` used, about `5818 MiB` free, AICore `24%`; post-Local AICore was `27%`. Existing workloads remained and were left unchanged.
- Host disk: about `43 GB` free on `/home/data4t2`; no full remote build tree was copied.
- Evidence files in this directory include source upload verification, compile logs, correctness raw output, Local raw output, device snapshots, and the three compact artifacts.

## Next Action

Keep V003 as a negative/indeterminate output-policy result and select the next independent GM/L2 single-factor hypothesis only after this evidence commit. Do not add another Hint or alter DMA, tiling, arithmetic, or parameter residency.
