# N04 V008 Results

## Candidate

- Route: `N04-GM-CACHE-STREAMING-X`
- Parent revision: N04 V007, candidate source commit `fcb62639229fd9ccf920db879f23f86fa0c0a013`.
- Candidate source commit: `05abc48b`.
- Change: wide-path input `SetL2CacheHint` changes from `CACHE_MODE_LAST` to `CACHE_MODE_NORMAL`; output remains `CACHE_MODE_DISABLE`.
- Scope: no change to arithmetic, DMA descriptors, prefetch, parameter residency, or other route mechanisms.
- Candidate source: `../submission.asc`.
- Candidate source SHA-256: `cb6713340b08095a67a415817e508b79cb4caf1453ccd03dd8b84edce15c11f9`.

## Execution

- Host: `hwnput3` through `cann-server3`; CANN `8.5.0.alpha002`; SoC `Ascend910B3`.
- Performance device: d7 only.
- Paired source inputs: R31B V011 SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; N04 V008 SHA-256 `cb6713340b08095a67a415817e508b79cb4caf1453ccd03dd8b84edce15c11f9`. Remote identities are in `remote-inputs.txt` and `build-identity-confirmation.txt`.
- Existing `paired_runner_v016` target: CMake configure PASS; final build PASS, return code 0. Executable SHA-256: `a7cac03552d1b689fc8e8268aa40b5d18a5068f73dfde8abf8dfe0111696ddc9`.
- The first build reached a Bisheng frontend exit 139 because `ASCEND_HOME_PATH` was absent. The complete command and stack are preserved in `paired-build.log`. Setting `ASCEND_HOME_PATH` exposed a missing `<vector>` search path; that attempt is preserved in `paired-build-env-retry.log`. The existing route include path in `CPATH`, together with `ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002`, produced the successful build in `paired-build-env-cpppath.log`.
- Correctness: PASS, 4/4 cases for R31B V011 and N04 V008 on d7; return code 0. Raw output: `correctness-v011-v008.log`.

## Paired Local

The existing Runner completed 45 warmups and 21 alternating pairs per case on d7. Device-event latency is primary. Delta is V008 minus V011; negative values favor V008.

| Case | Pairs | V011 median us | V008 median us | Paired delta median us | Delta MAD us | Delta range us | Readout |
|---|---:|---:|---:|---:|---:|---:|---|
| FP16 D=12288 | 21 | 353.980 | 339.000 | -15.220 | 1.840 | -23.840..-11.000 | All 21 pairs favored V008; consistent tail-case gain. |
| FP16 D=32768 | 21 | 922.260 | 890.600 | -32.320 | 15.000 | -197.800..4.340 | Wide-case median gain; one large negative outlier and one near-zero positive pair. |
| BF16 D=12288 | 21 | 425.100 | 411.440 | -14.820 | 3.520 | -230.320..15.420 | Median favored V008, but one large Parent outlier makes this tail result noisy. |
| BF16 D=32768 | 21 | 1477.580 | 1402.300 | -77.280 | 13.480 | -97.300..-46.060 | All 21 pairs favored V008; consistent wide-case gain. |

Both large working-set cases show negative paired medians larger than their delta MADs; FP16 D=32768 contains a large outlier. The partial-tail cases also have negative medians, with a clean FP16 result and a noisy BF16 result. `LOCAL_SCORE=INDICATIVE_GAIN_WIDE; TAIL_MIXED`; this is one Local run, not an Official result. Full raw samples and Runner summaries are in `local-paired-v011-v008.log`.

Before Local, d7 reported HBM `24791/65536 MiB`, AICore `12%`, and an existing Python process using `21260 MiB`. After Local, HBM was `24792/65536 MiB`, AICore `0%`, and the same process remained. The process was left unchanged. Device snapshots are `pre-local-d7.txt` and `post-local-d7.txt`; the pre-Correctness snapshot is `pre-correctness-d7.txt`.

## Saved Inputs

- Existing paired entry sources: `paired_runner_v016.asc`, `paired_runner.cpp`, and `paired_runner_abi.h`; CMake entry: `paired-CMakeLists.txt`.
- Built executable: `artifacts/paired_runner_v016` (588384 bytes).
- Configure, failed build attempts, successful build, remote source identities, and device snapshots remain beside this file. No failure log was replaced.
