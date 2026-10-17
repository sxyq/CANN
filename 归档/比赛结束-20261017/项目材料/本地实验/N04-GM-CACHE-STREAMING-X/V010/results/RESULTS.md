# N04 V010 Results

## Candidate

- Route: `N04-GM-CACHE-STREAMING-X`
- Parent revision: N04 V009, candidate source commit `61054394`.
- Candidate source commit: `3beafc05`.
- Change: wide-path `residualGm_` receives `CACHE_MODE_DISABLE`; x remains `CACHE_MODE_PERSISTENT` and output remains `CACHE_MODE_DISABLE`.
- Scope: no change to arithmetic, DMA descriptors, prefetch, parameter residency, or other route mechanisms.
- Candidate source: `../submission.asc`.
- Candidate source SHA-256: `4142e7e3f915a09b1f332a66b7b551328ef4c084ad269881463e2746c26efeee`.

## Execution

- Host: `hwnput3` through `cann-server3`; CANN `8.5.0.alpha002`; SoC `Ascend910B3`.
- Performance device: d7 only.
- Paired source inputs: R31B V011 SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; N04 V010 SHA-256 `4142e7e3f915a09b1f332a66b7b551328ef4c084ad269881463e2746c26efeee`. Remote identities are in `remote-inputs.txt`.
- Existing `paired_runner_v016` target: CMake configure PASS and build PASS, return code 0. Executable SHA-256: `0f4c018f68fdf2d6468e7257bebd3e9c6e33d3f313e4b39057c128bfa7002e4c`.
- Correctness: PASS, 4/4 cases for R31B V011 and N04 V010 on d7; return code 0. Raw output: `correctness-v011-v010.log`.

## Paired Local

The existing Runner completed 45 warmups and 21 alternating pairs per case on d7. Device-event latency is primary. Delta is V010 minus V011; negative values favor V010.

| Case | Pairs | V011 median us | V010 median us | Paired delta median us | Delta MAD us | Delta range us | Readout |
|---|---:|---:|---:|---:|---:|---:|---|
| FP16 D=12288 | 21 | 751.680 | 728.140 | -2.040 | 137.940 | -608.940..492.720 | No usable effect; both binaries and paired deltas vary heavily. |
| FP16 D=32768 | 21 | 944.040 | 883.760 | -57.340 | 6.540 | -78.460..-39.240 | All 21 deltas are negative; stable shape-specific gain in this run. |
| BF16 D=12288 | 21 | 436.000 | 424.140 | -12.760 | 8.560 | -476.240..541.600 | No usable effect; large positive and negative outliers. |
| BF16 D=32768 | 21 | 1515.900 | 1447.560 | -51.920 | 20.000 | -1824.560..1415.060 | Median is negative, but large outliers and broad distribution make the shape inconclusive. |

The FP16 D=32768 paired distribution is consistently negative, with a median magnitude about 8.8 times its delta MAD. The other three shapes contain substantial outliers; BF16 D=32768 includes both large positive and negative paired deltas. Overall `LOCAL_SCORE=NONE` because the complete four-shape run is noisy; retain the FP16 wide result as a shape-specific Local signal only. No samples were omitted. Raw samples and summaries are in `local-paired-v011-v010.log`; Runner return code was 0 and its end-of-run correctness checks passed all four cases.

Before Local, d7 reported HBM `24792/65536 MiB`, AICore `0%`, and an existing Python process using `21260 MiB`. After Local, HBM remained `24792/65536 MiB`, AICore was `1%`, and that process remained. It was left unchanged. The fresh resume snapshot is `pre-local-d7-resume.txt`; `pre-local-d7.txt` is the earlier pre-measurement snapshot. The post-run state is `post-local-d7.txt`.

## Saved Inputs

- Existing paired entry sources: `paired_runner_v016.asc`, `paired_runner.cpp`, and `paired_runner_abi.h`; CMake entry: `paired-CMakeLists.txt`.
- Built executable: `artifacts/paired_runner_v016` (588384 bytes).
- Configure, build, Correctness, full paired samples, and before/after d7 snapshots are retained beside this file.
