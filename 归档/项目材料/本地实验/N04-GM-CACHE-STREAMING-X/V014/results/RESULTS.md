# N04 V014 Results

## Candidate

- Route: `N04-GM-CACHE-STREAMING-X`.
- Direct parent: N04 V013, source commit `3b12f050a`.
- Candidate source commit: `3dbb15d5e`.
- Candidate source: `../submission.asc`.
- Candidate source SHA-256: `0fa1b334eeaac39e032adffa6771e9537d6bcb0a44900ea2f588e4bc5abe95e6`.
- Single change: wide low-precision `residualGm_` hint changes from `CACHE_MODE_NORMAL` to `CACHE_MODE_PERSISTENT`. `xGm_` and `outputGm_` remain `CACHE_MODE_DISABLE`.
- Arithmetic, Vector instructions, Scalar indexing, tile traversal, DMA descriptors, and parameter residency are unchanged.

## Build and Correctness

- Host: `hwnput3` via `cann-server3`; CANN `8.5.0.alpha002`; SoC `Ascend910B3`.
- Parent input: R31B V011 Champion SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate input: N04 V014 SHA-256 `0fa1b334eeaac39e032adffa6771e9537d6bcb0a44900ea2f588e4bc5abe95e6`. Remote paths and Runner inputs are recorded in `remote-inputs.txt`.
- Existing `paired_runner_v016` configure and Compile passed; build return code 0. CANN version: `8.5.0.alpha002`.
- Executable: `artifacts/paired_runner_v016`, SHA-256 `dc745c50a133fa5766664067b22a2111c523ca92cef19b64dcefe8af4f4399a9`.
- Correctness: PASS for Champion and Candidate on all four cases, d7; Runner return code 0. Raw output: `correctness-v011-n04v014.log`.

## Paired Local

- Device: d7 only. Existing Runner: `paired_runner_v016`; 45 warmups and 21 alternating pairs per case; device-event latency is primary.
- Timed Parent: R31B V011 Champion. Candidate: N04 V014. Delta is Candidate minus Champion. Full samples: `local-paired-v011-n04v014.log`.

| Case | Champion median us | V014 median us | Paired delta median us | Delta MAD us | Delta range us | Negative pairs |
|---|---:|---:|---:|---:|---:|---:|
| FP16 D=12288 | 464.800 | 423.420 | -31.960 | 52.860 | -552.620..472.100 | 14/21 |
| FP16 D=32768 | 1942.200 | 1859.220 | -93.960 | 217.220 | -1063.460..934.780 | 14/21 |
| BF16 D=12288 | 423.140 | 410.460 | -12.560 | 1.440 | -27.240..-4.980 | 21/21 |
| BF16 D=32768 | 1418.920 | 1356.180 | -63.280 | 5.740 | -77.940..-38.240 | 21/21 |

`LOCAL_SCORE=NONE` overall because both FP16 cases contain large two-sided variation; their negative medians do not establish a run-wide effect. Preserve the per-case signal: both BF16 cases have compact distributions with 21/21 negative paired deltas, while FP16 D=32768 is noisy and not a reliable gain. The full sample set is retained without filtering.

Before Correctness, d7 had AIVector at 88%; a later snapshot fell to 0% with HBM bandwidth at 31%. Before Local recheck, AIVector and AICore were 0% and HBM bandwidth 4%; after Local they were 0%, 0%, and 5%. The same existing Python process (PID 439848, 21260 MiB) remained untouched. Snapshots are adjacent. This is one Local run and is not an Official result.

## Evidence Files

- Source identities: `remote-inputs.txt`.
- Configure, build, and binary: `configure.log`, `paired-build.log`, `artifacts/paired_runner_v016`.
- Correctness and d7 state: `correctness-v011-n04v014.log`, `pre-correctness-d7.txt`, `pre-correctness-d7-recheck-1.txt`, `post-correctness-d7.txt`.
- Paired samples and d7 state: `local-paired-v011-n04v014.log`, `pre-local-d7.txt`, `pre-local-d7-recheck-1.txt`, `post-local-d7.txt`.
- Existing paired entry sources: `paired_runner_v016.asc`, `paired_runner.cpp`, `paired_runner_abi.h`, `paired-CMakeLists.txt`.
