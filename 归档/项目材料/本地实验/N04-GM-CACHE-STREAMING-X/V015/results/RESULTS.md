# N04 V015 Results

## Candidate

- Route: `N04-GM-CACHE-STREAMING-X`.
- Direct parent: N04 V014, source commit `3dbb15d5e`.
- Candidate source commit: `1516f32f5`.
- Candidate source: `../submission.asc`.
- Candidate source SHA-256: `a480f26fa101170550d1091d34fb5c5cca09cbc247e9e2ababa8110e5368e5cb`.
- Single change: add `gammaGm_.SetL2CacheHint(CACHE_MODE_PERSISTENT)` in the wide low-precision path. Existing x, residual, and output hints are unchanged.
- Gamma/bias UB staging, arithmetic, Vector instructions, Scalar indexing, tile traversal, and DMA descriptors are unchanged.

## Build and Correctness

- Host: `hwnput3` via `cann-server3`; CANN `8.5.0.alpha002`; SoC `Ascend910B3`.
- Parent input: R31B V011 Champion SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate input: N04 V015 SHA-256 `a480f26fa101170550d1091d34fb5c5cca09cbc247e9e2ababa8110e5368e5cb`. Exact remote input paths and other Runner inputs are in `remote-inputs.txt`.
- Existing `paired_runner_v016` configure and Compile passed; build return code 0.
- Executable: `artifacts/paired_runner_v016`, SHA-256 `1b074ba5f02c4e8e9f3ac7331ba793779f78fc18e3d579f08452d228279a7d14`.
- Correctness: PASS, Champion and Candidate across all four shapes on d7; Runner return code 0. Raw output: `correctness-v011-n04v015.log`.

## Paired Local

- Device: d7 only. Existing Runner: `paired_runner_v016`; 45 warmups and 21 alternating pairs per shape; device-event latency is primary.
- Timed Parent: R31B V011 Champion. Candidate: N04 V015. Delta is Candidate minus Champion. All 84 samples are in `local-paired-v011-n04v015.log`.

| Shape | Champion median us | V015 median us | Paired delta median us | Delta MAD us | Delta range us | Negative pairs |
|---|---:|---:|---:|---:|---:|---:|
| FP16 D=12288 | 393.420 | 377.260 | -13.580 | 15.300 | -422.600..375.200 | 17/21 |
| FP16 D=32768 | 1922.080 | 1818.100 | -77.620 | 178.040 | -1196.320..920.400 | 14/21 |
| BF16 D=12288 | 423.760 | 410.940 | -13.740 | 4.340 | -22.220..-4.580 | 21/21 |
| BF16 D=32768 | 1487.940 | 1387.600 | -101.580 | 17.240 | -121.720..62.060 | 20/21 |

`LOCAL_SCORE=NONE` overall. FP16 tail and wide contain large two-sided variation; their negative medians do not establish stable gains. BF16 D=12288 is consistently negative in this run. BF16 D=32768 has a strong negative median and 20/21 negative pairs, with one positive pair. Preserve these per-shape signals without promoting the single run to an overall gain. No sample was removed or selected by result direction.

Immediately before Local, d7 showed HBM use 37%, AICore 0%, AIVector 0%, and HBM bandwidth 0%. After Local it showed AICore 3%, AIVector 0%, HBM bandwidth 0%. During the run, the FP16 timing distributions include large outliers, indicating load variation not captured by the endpoint snapshot. An existing Python process (PID 439848, 21260 MiB) remained untouched. Device snapshots are adjacent. This is one Local run, not an Official result.

## Evidence Files

- Input identities: `remote-inputs.txt`.
- Configure, Compile, executable: `configure.log`, `paired-build.log`, `artifacts/paired_runner_v016`.
- Correctness and device state: `correctness-v011-n04v015.log`, `pre-correctness-d7.txt`, `post-correctness-d7.txt`.
- Paired samples and device state: `local-paired-v011-n04v015.log`, `pre-local-d7.txt`, `post-local-d7.txt`.
- Existing paired entry sources: `paired_runner_v016.asc`, `paired_runner.cpp`, `paired_runner_abi.h`, and `paired-CMakeLists.txt`.
