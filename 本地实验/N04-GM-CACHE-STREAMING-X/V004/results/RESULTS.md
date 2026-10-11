# N04 V004 Results

## Source

- Route: N04-GM-CACHE-STREAMING-X
- Branch: `route/n04-gm-cache-streaming-x-20261011`
- Parent revision: `6c53cf0e9410825cfae2edf38b7da1502b3afa9e` (V003 source)
- Candidate commit: `409df41bee7306ee1ddea8de59e9d0008138b43c`
- Paired harness commit: `d74d9d3eff21646b6137ad64c2238fca2aa534e1`
- Candidate source: `本地实验/N04-GM-CACHE-STREAMING-X/V004/submission.asc`
- Source SHA256: `efaae996d2851b408bb690b13103410a32f65dfe029973ebea7a688eda8f4f35`
- Parent source SHA256: `c00d8d4d5ea1c16636c1df1f0c541f42a4bdbde1e4c21c4f434e723e2cabb6db`
- Change: replace the V003 output-path `CACHE_MODE_LAST` with one `outputGm_.SetL2CacheHint(AscendC::CacheMode::CACHE_MODE_NORMAL)` call in `ProcessWideLowPrecision`.
- The input `CACHE_MODE_DISABLE` remains unchanged. No arithmetic, tiling, prefetch, DMA descriptor, or parameter-residency change.
- Remote source/dependency and final layout evidence: `upload-verification.txt`, return code 0.

## Compile

- Host: `hwnput3` through `cann-server3`; CANN package: `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`; device target: d1 / Ascend910B3.
- Remote run directory: `~/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V004/409df41b`.
- Adapter/device CMake configure and `full_link`: PASS, return code 0.
- Existing paired CMake configure and `paired_runner_v016` build: PASS, return code 0, using the current V003/V004 source directory and system C++ include path.
- Paired executable: 588736 bytes. Adapter outputs: `device.alink` and `submission.alink`, each about 180 KiB. Only these compact outputs were copied to `results/artifacts/`.
- Raw compile evidence: `compile.log` and `paired-build.log`.

## Correctness

- Command: `./paired_runner_v016 --correctness`.
- Loader: CANN environment with `/usr/lib/aarch64-linux-gnu` first in `LD_LIBRARY_PATH`.
- Result: PASS, return code 0; all 4 cases completed, V003 Parent and V004 Candidate each reported `failures=0`.
- Cases: FP16 `1024x12288`, FP16 `1024x32768`, BF16 `1024x12288`, BF16 `1024x32768`.
- Raw evidence: `correctness.log`; pre-run state: `pre-correctness-d1.txt`.

## Paired Local

- Command: `./paired_runner_v016 --paired`; device events are primary and wall time is diagnostic.
- Each case has 21 alternating V003 Parent/V004 Candidate pairs after 45 warmups; rows 1024; width 32768 is the large-working-set case.
- Raw evidence: `local-paired.log`; pre/post state: `pre-local-d1.txt`, `post-local-d1.txt`.

| Case | Samples | V003 median us | V004 median us | Delta median us | Delta MAD us | V003 CV | V004 CV |
|---|---:|---:|---:|---:|---:|---:|---:|
| FP16 D=12288 | 21 | 389.960 | 460.380 | 17.680 | 37.080 | 0.27289 | 0.22440 |
| FP16 D=32768 | 21 | 1088.980 | 1116.040 | 22.440 | 89.080 | 0.22251 | 0.09133 |
| BF16 D=12288 | 21 | 513.600 | 504.580 | -29.100 | 38.800 | 0.37799 | 0.24916 |
| BF16 D=32768 | 21 | 1671.900 | 1685.020 | 7.680 | 54.180 | 0.06622 | 0.16550 |

- `LOCAL_SCORE=NONE`: all four cases contain large outliers, including paired deltas from `-961.220` to `1458.980` us, while d1 carried sustained unrelated load. The BF16 tail median improvement is retained as raw evidence and is not treated as a stable result.
- Runner return code: 0. Post-run process query found no N04 Runner.

## Device And Evidence

- d1 pre-Local snapshot: HBM `59718/65536 MiB` used, about `5818 MiB` free, AICore `28%`; post-Local HBM was `59719/65536 MiB` used and AICore `25%`. Existing workloads remained and were left unchanged.
- Host disk: about `34 GB` free on `/home/data4t2`; only small logs, snapshots, and compact binaries were copied locally.
- Evidence files in this directory include source upload verification, compile logs, correctness raw output, Local raw output, device snapshots, and the three compact artifacts.

## Next Action

Continue on the same GM/L2 axis by changing the existing output Hint mode only. Do not add a new Hint or alter DMA, tiling, arithmetic, or parameter residency.
