# N04 V013 Results

## Candidate

- Route: `N04-GM-CACHE-STREAMING-X`.
- Direct parent: N04 V012, source commit `5216bad0d`.
- Candidate source commit: `3b12f050`.
- Candidate source: `../submission.asc`.
- Candidate source SHA-256: `173e9233b51f07d2666130cf27409a23431eac61ac3fe353804a5200f1ee6c6b`.
- Single change: wide low-precision `xGm_` hint changes from `CACHE_MODE_PERSISTENT` to `CACHE_MODE_DISABLE`. `residualGm_` stays `CACHE_MODE_NORMAL`; `outputGm_` stays `CACHE_MODE_DISABLE`.
- Arithmetic, Vector instructions, Scalar indexing, tile traversal, DMA descriptors, and parameter residency are unchanged.

## Build and Correctness

- Host: `hwnput3` via `cann-server3`; CANN `8.5.0.alpha002`; SoC `Ascend910B3`.
- Parent input: R31B V011 Champion SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate input: N04 V013 SHA-256 `173e9233b51f07d2666130cf27409a23431eac61ac3fe353804a5200f1ee6c6b`. Both inputs and Runner identities are in `remote-inputs.txt`.
- Existing `paired_runner_v016` CMake configure passed; Compile passed with return code 0 using the CANN 8.5 environment and C++ include paths established in V012.
- Executable: `artifacts/paired_runner_v016`, SHA-256 `a2f72736807ee243b9fefbc771a7fce4281ef414df74133565eaf739ea7ed2a0`.
- Correctness: PASS, all four shapes for Champion and Candidate on d7; Runner return code 0. Raw output: `correctness-v011-n04v013.log`.

## Paired Local

- Device: d7 only. Existing Runner: `paired_runner_v016`; 45 warmups and 21 alternating pairs per shape; device-event latency is primary.
- Timed Parent is R31B V011 Champion; Candidate is N04 V013. Delta is Candidate minus Champion. The complete 84-pair output is `local-paired-v011-n04v013.log`; every sample is retained.

| Shape | Champion median us | V013 median us | Paired delta median us | Delta MAD us | Delta range us | Negative pairs |
|---|---:|---:|---:|---:|---:|---:|
| FP16 D=12288 | 371.680 | 360.700 | -14.120 | 4.960 | -23.940..14.660 | 18/21 |
| FP16 D=32768 | 914.520 | 879.380 | -40.520 | 4.220 | -53.760..-21.140 | 21/21 |
| BF16 D=12288 | 429.980 | 418.440 | -12.200 | 4.540 | -32.240..7.380 | 18/21 |
| BF16 D=32768 | 1478.160 | 1401.380 | -85.280 | 10.760 | -1143.320..1296.960 | 20/21 |

`LOCAL_SCORE=NONE` for this run. FP16 D=32768 is a consistent per-shape signal, with 21/21 negative pairs and no sign crossing. Both tail cases cross zero. BF16 D=32768 contains large two-sided outliers, so its negative median is not treated as a stable result. d7 snapshots also show shared-load variation: before the final pre-Local recheck AIVector was 9% and HBM bandwidth 44%; the recheck read 4% and 0%; after Local they read 7% and 31%. An existing Python process (PID 439848, 21260 MiB) remained untouched. No sample was omitted or selected by result direction.

This is one Local run and does not represent an Official result.

## Evidence Files

- Source identities: `remote-inputs.txt`.
- Configure, Compile, and executable: `configure.log`, `paired-build.log`, `artifacts/paired_runner_v016`.
- Correctness output and d7 state: `correctness-v011-n04v013.log`, `pre-correctness-d7.txt`, `post-correctness-d7.txt`.
- Full Local samples and d7 state: `local-paired-v011-n04v013.log`, `pre-local-d7.txt`, `pre-local-d7-recheck-1.txt`, `post-local-d7.txt`.
- Existing paired entry sources: `paired_runner_v016.asc`, `paired_runner.cpp`, `paired_runner_abi.h`, and `paired-CMakeLists.txt`.
