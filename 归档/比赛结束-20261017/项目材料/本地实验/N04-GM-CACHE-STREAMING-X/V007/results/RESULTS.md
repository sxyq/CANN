# N04 V007 Results

## Candidate and Build

- Route: `N04-GM-CACHE-STREAMING-X`
- Branch: `route/n04-gm-cache-streaming-x-20261011`
- Candidate commit: `fcb62639229fd9ccf920db879f23f86fa0c0a013`
- Candidate source: `../submission.asc`
- Change: output `SetL2CacheHint` only, from `CACHE_MODE_PERSISTENT` to `CACHE_MODE_DISABLE`. Input hint remains `CACHE_MODE_LAST`; compute and data flow are unchanged.
- Candidate source SHA-256: `dad7d9870a596294762e9b35062b121b089a22cedfc99565defffe441da1e367`
- CANN: `8.5.0.alpha002`, host `hwnput3`, SoC `Ascend910B3`.
- Paired Runner: existing `paired_runner_v016`, device d7, 45 warmups and 21 alternating pairs per case.
- Champion source: R31B V011 `R31B-V011-LP-ROW-PIPELINE_kernel.asc`, SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source used by the Champion run: SHA-256 `dad7d9870a596294762e9b35062b121b089a22cedfc99565defffe441da1e367`.
- Runner translation unit includes the above V011 and V007 sources. The dependency file records both paths. Final paired executable SHA-256: `64d15aff1a5f9ec5f1831a9c0ef71dbcd61407c215a70c0b58c9df963264320b`.
- Compile: PASS. See `configure-champion-v007.log`, `paired-build-champion-v007.log`, and `paired-build-champion-v007-labels.log`. Earlier failed attempts remain in their original logs.

## Correctness

Champion V011 and Candidate V007 both passed all four FP16/BF16 cases on d7; return code 0. Raw output: `correctness-v011-v007.log`.

## Champion V011 vs V007 Local

Device-event latency is primary. The delta is Candidate minus Champion; negative values mean Candidate was faster.

| Case | Pairs | Champion median us | V007 median us | Paired delta median us | Delta MAD us | Readout |
|---|---:|---:|---:|---:|---:|---|
| FP16 D=12288 | 21 | 739.940 | 736.700 | 10.040 | 100.180 | Noise dominates; no benefit claim. |
| FP16 D=32768 | 21 | 908.580 | 871.480 | -37.080 | 3.520 | Every paired delta was negative; repeatable gain in this run. |
| BF16 D=12288 | 21 | 451.800 | 440.860 | -16.640 | 10.300 | Large positive and negative outliers; inconclusive. |
| BF16 D=32768 | 21 | 1487.800 | 1414.900 | -61.780 | 24.880 | Large positive and negative outliers; inconclusive. |

Raw samples and distributions: `local-paired-champion-v007.log`. Before sampling, d7 reported HBM `24813/65536 MiB`, AICore `0%`, with an existing Python process using `21260 MiB`; after sampling HBM was `24791/65536 MiB`, AICore `2%`, and the same process remained. No other device was used or changed. The large timing outliers are retained. Overall `LOCAL_SCORE=NONE`: only FP16 D=32768 showed a clean gain in this run.

## Other V007 Logs

- `correctness.log` and `local-paired.log` are preserved as requested; these outputs use the V005 Parent/V006 Candidate pairing and are excluded from V007 scoring.
- `local-paired-revalidation.log` is the separate Direct-Parent measurement using V006 source SHA-256 `3b79b6d432d3e41ffb794a7656f4eddd2e3381efdc9c9717a83fb5f5ef537247` and V007 source SHA-256 `dad7d9870a596294762e9b35062b121b089a22cedfc99565defffe441da1e367`. The Runner's printed V005/V006 labels are stale; retain this run as V006/V007 data.
- Champion V011/V007 raw files are `correctness-v011-v007.log` and `local-paired-champion-v007.log`; the Champion executable is `artifacts/paired_runner_v016_champion_v007`.

