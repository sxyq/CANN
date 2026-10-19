# N04 V011 Results

## Candidate

- Route: `N04-GM-CACHE-STREAMING-X`
- Parent revision: N04 V010, candidate source commit `3beafc05`.
- Candidate source commit: `6eef84ad`.
- Change: wide-path `residualGm_` changes from `CACHE_MODE_DISABLE` to `CACHE_MODE_LAST`; x remains `CACHE_MODE_PERSISTENT` and output remains `CACHE_MODE_DISABLE`.
- Scope: arithmetic, Vector instructions, Scalar indexing, tile traversal, DMA descriptors, and parameter residency are unchanged.
- Candidate source: `../submission.asc`.
- Candidate source SHA-256: `857ac8241985f3f501f8742c687cb7eb00604f5df3a730a6e7fc3f06d98c1bcf`.

## Execution

- Host: `hwnput3` through `cann-server3`; CANN `8.5.0.alpha002`; SoC `Ascend910B3`.
- Performance device: d7 only.
- Paired source inputs: R31B V011 SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; N04 V011 SHA-256 `857ac8241985f3f501f8742c687cb7eb00604f5df3a730a6e7fc3f06d98c1bcf`. Remote identities are in `remote-inputs.txt`.
- Existing `paired_runner_v016` target: CMake configure PASS and build PASS, return code 0. Executable SHA-256: `650cebab3110693be9e93481e56e8f8f6a5c17d33da2451315a0c8fabfdf0934`.
- Correctness: PASS, 4/4 cases for R31B V011 and N04 V011 on d7; return code 0. Raw output: `correctness-v011-n04v011.log`.

## Paired Local

The existing Runner completed 45 warmups and 21 alternating pairs per case on d7. Device-event latency is primary. Delta is N04 V011 minus R31B V011; negative values favor the Candidate.

| Case | Pairs | Parent median us | Candidate median us | Paired delta median us | Delta MAD us | Delta range us | Readout |
|---|---:|---:|---:|---:|---:|---:|---|
| FP16 D=12288 | 21 | 352.960 | 339.760 | -12.960 | 2.020 | -17.840..-4.720 | All 21 pairs negative; stable gain in this run. |
| FP16 D=32768 | 21 | 921.060 | 883.280 | -42.400 | 12.320 | -86.360..9.540 | 19/21 pairs negative; gain signal with moderate spread. |
| BF16 D=12288 | 21 | 886.080 | 483.260 | -11.740 | 27.780 | -465.760..437.160 | Large positive/negative outliers dominate; no usable effect. |
| BF16 D=32768 | 21 | 1424.220 | 1371.040 | -53.420 | 4.820 | -134.800..-42.480 | All 21 pairs negative; stable gain despite one larger negative delta. |

Three shapes show consistently negative paired distributions; BF16 D=12288 is inconclusive. `LOCAL_SCORE=INDICATIVE_GAIN; BF16_D12288=NONE`. This is one Local run, not an Official result. Full samples and summaries are in `local-paired-v011-n04v011.log`; Runner return code was 0 and its final correctness checks passed all four shapes.

Before Local, d7 reported HBM `24791/65536 MiB`, AICore `0%`, and an existing Python process using `21260 MiB`. After Local, HBM was `24792/65536 MiB`, AICore `2%`, and the process remained. It was left unchanged. The pre-Correctness snapshot showed d7 AICore at `42%`; the pre-Local snapshot was taken again immediately before sampling. Snapshots are `pre-correctness-d7.txt`, `pre-local-d7.txt`, and `post-local-d7.txt`.

## Saved Inputs

- Existing paired entry sources: `paired_runner_v016.asc`, `paired_runner.cpp`, and `paired_runner_abi.h`; CMake entry: `paired-CMakeLists.txt`.
- Built executable: `artifacts/paired_runner_v016` (588384 bytes).
- Configure, build, Correctness, full paired samples, and before/after d7 snapshots are retained beside this file.
