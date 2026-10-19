# N04 V006 Results

## Source

- Route: `N04-GM-CACHE-STREAMING-X`
- Branch: `route/n04-gm-cache-streaming-x-20261011`
- Worktree: `/Users/sunyiyang/Desktop/Project/cann/worktrees/n04-gm-cache-streaming`
- Parent commit: `41746fab56e4155cf8ff5a2bf1edb5742e32160d` (V005 result tree)
- Parent Candidate source commit: `4d10c4113ce8fb5a6cff52d451f5d77c8f48d077`
- V006 source commit: `5834f62e2f7ae19979cf1457a0251627819c3d3c`
- V006 paired harness commit: `4e5eb9e6160cd923f4a6890700ba1647078071b4`
- Candidate source: `本地实验/N04-GM-CACHE-STREAMING-X/V006/submission.asc`
- Candidate source SHA256: `3b79b6d432d3e41ffb794a7656f4eddd2e3381efdc9c9717a83fb5f5ef537247`
- Change: change only `xGm_.SetL2CacheHint` from `CACHE_MODE_DISABLE` to `CACHE_MODE_LAST`. The existing output `CACHE_MODE_PERSISTENT` remains unchanged. No extra Hint, DMA, tiling, arithmetic, prefetch, or parameter-residency changes.
- Remote run directory: `~/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V006/5834f62e`
- Source upload SHA verification passed; see `upload-verification.txt`.

## Similarity Fact

- Main audit reports ten FULL_SOURCE_SIMILARITY results around `0.982-0.999`, above the 80% limit, due to the shared Champion body. This revision retains that measured fact. No renaming, comment, formatting, or wrapper changes were made to lower it.
- Effective-change similarity was not measured in this Route.

## Compile

- Host/device: `hwnput3`, d1 / Ascend910B3; CANN `8.5.0.alpha002`.
- Existing adapter/device CMake configure and `full_link`: PASS, return code 0.
- Existing paired CMake configure and `paired_runner_v016` build: PASS, return code 0.
- The adapter's `device.cpp` includes `R31B-V016-WIDE-TILE-SEED_kernel.asc`; its `.alink` files are not V006 Candidate compile evidence. The fixed-identity compile evidence is the paired Runner translation unit, which includes the uploaded V005 Parent and V006 Candidate kernel files. Paired CMakeCache points both kernel source directories at the V006 run's `source/`, and the CMake build rule compiles that translation unit.
- Compact outputs copied to `artifacts/`: `device.alink` (183440 bytes), `submission.alink` (183448 bytes), and `paired_runner_v016` (588640 bytes). Remote SHA256 values match local copies: `device.alink` `9801a1f0aa59cb4eb9e1e915269155d20760201fe9ee42cb3e50ab255fdc03fa`, `submission.alink` `0d5756178282838cddfb84c8480302e47c0126a72c386d3ec6335e54e9faccc5`, paired Runner `d62e3dadbd38471aad69e2596e36d247fc191700ac7d48cba46b65fb529e5408`. The paired Runner source SHA is `8aca32784f3a8bbea1a2f34e98d685259717a2cff35e86533d0d4eaf254935ed`; included host runner SHA is `4d303dc2d60b3a1c7ac46eeadeaa50bb1b557943157d350b5a80ea99f25d02ba`.
- This exact paired Runner binary was then used for correctness, same-binary Parent control, and all paired Local runs. Its source has no `aclrtResetDevice` call; exit calls `aclFinalize`. The compile inputs and artifact identities are preserved in `compile-identity.txt`. No rebuild was needed for the fixed-identity retests.
- Raw evidence: `compile.log`, `paired-build.log`.

## Correctness

- Existing command: `./paired_runner_v016 --correctness`, on device 1 with system `libstdc++` first and the CANN loader paths retained.
- The first launch omitted CANN loader paths and exited 127 before loading `libgraph.so`; this failed attempt is preserved in `correctness.log`. The same Runner then completed successfully with the established loader paths.
- Final result: PASS, return code 0; all four public FP16/BF16 cases reported zero failures for Parent V005 and Candidate V006.
- Raw evidence: `correctness.log`.

## Paired Local

- Existing command: `./paired_runner_v016 --paired`; device-event timing is primary, wall time diagnostic.
- Four public cases, each run with 45 warmups and 21 alternating Parent V005/Candidate V006 pairs per run. The original run and two revalidations are all retained in `local-paired.log`: 63 pairs per case, 252 raw pair samples total.

| Case | Pairs | V005 median us | V006 median us | Paired delta median us | Delta MAD us | V005 CV | V006 CV |
|---|---:|---:|---:|---:|---:|---:|---:|
| FP16 D=12288 | 21 | 399.480 | 410.380 | 5.840 | 41.380 | 0.23147 | 0.08724 |
| FP16 D=32768 | 21 | 1111.760 | 1122.880 | 16.240 | 45.540 | 0.09540 | 0.22666 |
| BF16 D=12288 | 21 | 509.800 | 481.860 | -5.740 | 42.160 | 0.29515 | 0.28145 |
| BF16 D=32768 | 21 | 1672.240 | 1665.960 | 40.360 | 59.100 | 0.21716 | 0.20926 |

- The table above is the first 21-pair run. The second run and the latest run are summarized below; all raw samples remain in `local-paired.log`.

| Latest run case | Pairs | V005 median us | V006 median us | Paired delta median us | Delta MAD us | V005 CV | V006 CV | Delta min..max us |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| FP16 D=12288 | 21 | 452.840 | 435.920 | -3.880 | 57.120 | 0.29393 | 0.29477 | -436.120..426.300 |
| FP16 D=32768 | 21 | 1129.460 | 1112.740 | -29.880 | 149.820 | 0.31311 | 0.13245 | -1190.840..564.200 |
| BF16 D=12288 | 21 | 505.880 | 493.520 | 0.620 | 35.820 | 0.38474 | 0.34463 | -566.740..541.760 |
| BF16 D=32768 | 21 | 1669.200 | 1688.420 | 23.520 | 24.020 | 0.05506 | 0.16921 | -118.500..1150.380 |

The second-run 21-pair medians/deltas were: FP16 D=12288 `352.940/353.000 us`, delta `0.280 us`, delta MAD `1.880 us`; FP16 D=32768 `1118.820/1159.140 us`, delta `-12.500 us`, MAD `95.040 us`; BF16 D=12288 `535.200/493.400 us`, delta `-11.840 us`, MAD `49.980 us`; BF16 D=32768 `1687.440/1702.500 us`, delta `0.740 us`, MAD `61.920 us`. Full Parent/Candidate medians and distributions for this run are in `local-paired.log`.

| All three runs, case | Pairs | V005 median us | V006 median us | Paired delta median us | Delta MAD us |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 63 | 385.940 | 391.440 | 0.280 | 30.180 |
| FP16 D=32768 | 63 | 1119.400 | 1119.980 | 3.320 | 108.380 |
| BF16 D=12288 | 63 | 522.380 | 493.400 | -2.340 | 49.520 |
| BF16 D=32768 | 63 | 1673.220 | 1688.420 | 23.520 | 46.720 |

- The existing `--qualify-parent` same-binary control completed 62 V005 samples per case (248 total), return code 0. Outcomes: FP16 D=12288 `NEEDS_VALIDATION` (MAD/median 0.09498; block drift 0.10389); FP16 D=32768 `PASS` (0.04198; 0.02320); BF16 D=12288 `PASS` (0.06531; 0.06237); BF16 D=32768 `PASS` (0.04710; 0.07488). Control raw samples are in `same-binary-parent.log`.
- Latest paired run returned 0 and passed correctness for all four Parent/Candidate cases. There were no runtime, OOM, or resource errors. Across all 63 pairs per case, every absolute paired-delta median is below its delta MAD. The latest run also contains large individual outliers, up to -1190.840 and +1150.380 us. The Parent-only control has one `NEEDS_VALIDATION` case. `LOCAL_SCORE=NONE`: measurements are complete, but variation and the lack of a consistent per-shape effect do not establish a repeatable benefit for `CACHE_MODE_LAST`.
- Device snapshots: `pre-local-d1.txt`, `post-local-d1.txt`, `pre-revalidation-d1.txt`, `pre-revalidation-local-d1.txt`, `post-revalidation-local-d1.txt`, `pre-control-d1.txt`, `pre-final-paired-d1.txt`, and `post-final-paired-d1.txt`. The first attempted process-filter snapshots for `pre-control-d1.txt` and `pre-final-paired-d1.txt` report a shell quoting error after `npu-smi` completed; their device/process-table output is retained. The final post-pair snapshot succeeded and reported no local Runner.

## Device And Host Load

- Device d1 is Ascend910B3. Before the latest paired run, HBM was `59718/65536 MiB` (5818 MiB free) and AICore utilization 29%; afterward HBM was `59719/65536 MiB` (5817 MiB free) and AICore utilization 27%.
- The npu-smi process tables show unrelated VLLM/Python workload on d1 and other devices. Those processes were left unchanged. The paired and control commands completed with return code 0.
- Host disk on `/home/data4t2` had about 34 GB available in both snapshots. Only the three compact build outputs listed above were copied locally; no build tree was copied.

## Outcome

Fixed V006 measurement loop is complete. Candidate and Parent source identities, paired Runner compile inputs and output identity, correctness, Parent control, all three paired runs, device evidence, and raw samples are retained in this V006 results directory. No later Revision or Candidate source change was made in this work.
