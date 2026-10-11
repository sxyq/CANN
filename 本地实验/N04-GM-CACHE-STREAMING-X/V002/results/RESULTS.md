# N04 V002 Results

## Source

- Route: N04-GM-CACHE-STREAMING-X
- Branch: `route/n04-gm-cache-streaming-x-20261011`
- Parent revision: `e498788ae5a577ff36970f27e4a8cab2cea45245`
- Candidate commit: `46ee7f3d3cba1c781c3ce68a8dfdd7251a641247`
- Candidate source: `本地实验/N04-GM-CACHE-STREAMING-X/V002/submission.asc`
- Source SHA256: `dcbc9bfbb710daa6829922dac2a762e84cbc54b32f30d64ac6397c056a2dc86e`
- Parent source SHA256: `5bc717eabfcc9fe526c37f1901a283fa5bd3c546c4b33da150b36aec81b21091`
- Change: add one `outputGm_.SetL2CacheHint(AscendC::CacheMode::CACHE_MODE_DISABLE)` call in `ProcessWideLowPrecision`.
- V001 input hint remains unchanged. No arithmetic, tiling, prefetch, DMA descriptor, or parameter-residency change.
- Remote source SHA verification: `upload-verification.txt`, return code 0.

## Compile

- Host: `hwnput3` through `cann-server3`; CANN package: `/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`; device target: d1 / Ascend910B3.
- Remote run directory: `~/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V002/46ee7f3d`.
- Adapter/device CMake configure and `full_link`: PASS, return code 0. The first environment source path was absent, but the existing CMake entry used its explicit CANN 8.5 package path and built both `.alink` outputs.
- Existing paired CMake configure: PASS. First build returned 2 because its default source include directories did not contain the current V001/V002 files; the same CMake entry was reconfigured with both source-dir cache values set to the current remote `source/` directory, then the paired build returned 0.
- Paired executable: `paired_runner_v016`, 588736 bytes. Adapter outputs: `device.alink` and `submission.alink`, each about 180 KiB. Only these compact outputs were copied to `results/artifacts/`.
- Raw compile evidence: `compile.log` and `paired-build.log`.

## Correctness

- Command: `./paired_runner_v016 --correctness`.
- Loader: CANN environment with `/usr/lib/aarch64-linux-gnu` first in `LD_LIBRARY_PATH`.
- Result: PASS, return code 0; `aclInit`/device setup completed, all 4 cases completed, Parent and Candidate each reported `failures=0`.
- Cases: FP16 `1024x12288`, FP16 `1024x32768`, BF16 `1024x12288`, BF16 `1024x32768`.
- Raw evidence: `correctness.log`; pre-run state: `pre-correctness-d1.txt`.

## Paired Local

- Command: `./paired_runner_v016 --paired`; device events are primary and wall time is diagnostic.
- Per case: 45 warmups and 21 alternating Parent/Candidate pairs; rows 1024; width 32768 is the large-working-set case.
- Raw evidence: `local-paired.log`; pre/post state: `pre-local-d1.txt`, `post-local-d1.txt`.

| Case | Parent median us | Candidate median us | Delta median us | Delta MAD us |
|---|---:|---:|---:|---:|
| FP16 D=12288 | 395.280 | 407.740 | -13.320 | 50.580 |
| FP16 D=32768 | 1084.620 | 1049.880 | -23.300 | 77.180 |
| BF16 D=12288 | 503.740 | 448.220 | -67.160 | 30.160 |
| BF16 D=32768 | 1688.980 | 1577.140 | -122.520 | 45.400 |

- `LOCAL_SCORE=NONE`: the paired deltas contain large outliers and the d1 device had sustained unrelated load. The BF16 medians are retained as raw evidence and are not treated as a stable route score.
- Runner return code: 0. Post-run process query found no N04 Runner.

## Device And Evidence

- d1 pre-Local snapshot: HBM `59718/65536 MiB` used, about `5818 MiB` free, AICore `26%`; post-Local AICore was `13%` and the same existing workloads remained. Other users' processes were left unchanged.
- Host disk: about `43 GB` free on `/home/data4t2`; no full remote build tree was copied.
- Evidence files in this directory include source upload verification, compile logs, correctness raw output, Local raw output, device snapshots, and the three compact artifacts.

## Next Action

After this evidence commit, start V003 from V002 with one independent SetL2CacheHint choice on the same GM/L2 axis. Preserve the V001 input hint and change only the selected output-path cache mode.
