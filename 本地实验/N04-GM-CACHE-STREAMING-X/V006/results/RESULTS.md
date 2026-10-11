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
- Compact outputs copied to `artifacts/`: `device.alink` (183440 bytes), `submission.alink` (183448 bytes), and `paired_runner_v016` (588640 bytes).
- Raw evidence: `compile.log`, `paired-build.log`.

## Correctness

- Existing command: `./paired_runner_v016 --correctness`, on device 1 with system `libstdc++` first and the CANN loader paths retained.
- The first launch omitted CANN loader paths and exited 127 before loading `libgraph.so`; this failed attempt is preserved in `correctness.log`. The same Runner then completed successfully with the established loader paths.
- Final result: PASS, return code 0; all four public FP16/BF16 cases reported zero failures for Parent V005 and Candidate V006.
- Raw evidence: `correctness.log`.

## Paired Local

- Existing command: `./paired_runner_v016 --paired`; device-event timing is primary, wall time diagnostic.
- Four public cases, each with 45 warmups and 21 alternating Parent V005/Candidate V006 pairs. All 84 pair samples are retained in `local-paired.log`.

| Case | Pairs | V005 median us | V006 median us | Paired delta median us | Delta MAD us | V005 CV | V006 CV |
|---|---:|---:|---:|---:|---:|---:|---:|
| FP16 D=12288 | 21 | 399.480 | 410.380 | 5.840 | 41.380 | 0.23147 | 0.08724 |
| FP16 D=32768 | 21 | 1111.760 | 1122.880 | 16.240 | 45.540 | 0.09540 | 0.22666 |
| BF16 D=12288 | 21 | 509.800 | 481.860 | -5.740 | 42.160 | 0.29515 | 0.28145 |
| BF16 D=32768 | 21 | 1672.240 | 1665.960 | 40.360 | 59.100 | 0.21716 | 0.20926 |

- `LOCAL_SCORE=NONE`: paired deltas vary widely, including -1648.880 to 1624.680 us outliers; median changes are within or smaller than delta MAD in every case. d1 also carried substantial unrelated load. These measurements do not establish a stable benefit for `CACHE_MODE_LAST` on the input path.
- Raw evidence: `local-paired.log`; device snapshots: `pre-local-d1.txt` and `post-local-d1.txt`.

## Device And Host Load

- Device d1 is Ascend910B3. Before Local, HBM usage was `59718/65536 MiB` (5818 MiB free) and AICore utilization was 29%; after Local it was `59718/65536 MiB` and 28%.
- The npu-smi process tables show unrelated VLLM workload on d1 and other devices. Those processes were left unchanged. The paired command completed with return code 0.
- Host disk on `/home/data4t2` had about 34 GB available in both snapshots. Only the three compact build outputs listed above were copied locally; no build tree was copied.

## Next Action

Keep V006 as a negative/inconclusive performance result with `LOCAL_SCORE=NONE`. Any next revision remains on the existing GM/L2 single-factor axis; do not add a second Hint or change DMA, tiling, arithmetic, or parameter residency. Preserve the FULL_SOURCE_SIMILARITY over-limit fact.
