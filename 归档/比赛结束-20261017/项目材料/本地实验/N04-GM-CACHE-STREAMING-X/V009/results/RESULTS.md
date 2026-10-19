# N04 V009 Results

## Candidate

- Route: `N04-GM-CACHE-STREAMING-X`
- Parent revision: N04 V008, candidate source commit `05abc48b`.
- Candidate source commit: `61054394`.
- Change: wide-path input `SetL2CacheHint` changes from `CACHE_MODE_NORMAL` to `CACHE_MODE_PERSISTENT`; output remains `CACHE_MODE_DISABLE`.
- Scope: no change to arithmetic, DMA descriptors, prefetch, parameter residency, or other route mechanisms.
- Candidate source: `../submission.asc`.
- Candidate source SHA-256: `90a281d563d7a9fc9a9434af0e68984ad9edb961e0a6a6acca9572224c98eef6`.

## Execution

- Host: `hwnput3` through `cann-server3`; CANN `8.5.0.alpha002`; SoC `Ascend910B3`.
- Performance device: d7 only.
- Paired source inputs: R31B V011 SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; N04 V009 SHA-256 `90a281d563d7a9fc9a9434af0e68984ad9edb961e0a6a6acca9572224c98eef6`. Remote identities are in `remote-inputs.txt`.
- Existing `paired_runner_v016` target: CMake configure PASS and build PASS, return code 0. Executable SHA-256: `03af5f90a33182e0096b5261650566f43a96ee5eccc3321146e9fff1514a7f76`.
- Build environment: CANN `8.5.0.alpha002`, with `ASCEND_HOME_PATH` and the route's established `CPATH`; raw output is in `configure.log` and `paired-build.log`.
- Correctness: PASS, 4/4 cases for R31B V011 and N04 V009 on d7; return code 0. Raw output: `correctness-v011-v009.log`.

## Paired Local

The existing Runner completed 45 warmups and 21 alternating pairs per case on d7. Device-event latency is primary. Delta is V009 minus V011; negative values favor V009.

| Case | Pairs | V011 median us | V009 median us | Paired delta median us | Delta MAD us | Delta range us | Readout |
|---|---:|---:|---:|---:|---:|---:|---|
| FP16 D=12288 | 21 | 384.040 | 372.920 | -10.920 | 9.520 | -28.660..16.720 | Median favored V009, with mixed pairs and visible variation. |
| FP16 D=32768 | 21 | 933.260 | 893.660 | -37.540 | 6.940 | -53.280..-16.700 | All 21 pairs favored V009; consistent wide-case gain. |
| BF16 D=12288 | 21 | 443.580 | 429.240 | -12.320 | 5.860 | -28.040..3.200 | 20 of 21 pairs favored V009; modest gain with some variation. |
| BF16 D=32768 | 21 | 1442.520 | 1387.160 | -55.540 | 10.240 | -83.940..-35.760 | All 21 pairs favored V009; consistent wide-case gain. |

Both large working-set cases show negative paired medians larger than their delta MADs, and every pair favored V009. Tail medians were also negative, with FP16 showing more variation and BF16 20 of 21 pairs negative. `LOCAL_SCORE=INDICATIVE_GAIN_WIDE; TAIL_MIXED`; this is one Local run, not an Official result. Full raw samples and Runner summaries are in `local-paired-v011-v009.log`.

Before Local, d7 reported HBM `24791/65536 MiB`, AICore `10%`, and an existing Python process using `21260 MiB`. After Local, HBM remained `24791/65536 MiB`, AICore was `0%`, and the same process remained. The process was left unchanged. Device snapshots are `pre-local-d7.txt` and `post-local-d7.txt`; the pre-Correctness snapshot is `pre-correctness-d7.txt`.

## Saved Inputs

- Existing paired entry sources: `paired_runner_v016.asc`, `paired_runner.cpp`, and `paired_runner_abi.h`; CMake entry: `paired-CMakeLists.txt`.
- Built executable: `artifacts/paired_runner_v016` (588384 bytes).
- `pre-build-server-state.txt` preserves the first incomplete metadata capture; the complete capacity and process record is `pre-build-server-state-v009.txt`.
- Candidate upload identity, build output, Correctness, paired samples, and device snapshots are retained beside this file.
