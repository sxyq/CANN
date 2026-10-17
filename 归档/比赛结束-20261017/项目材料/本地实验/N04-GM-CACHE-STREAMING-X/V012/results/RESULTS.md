# N04 V012 Results

## Candidate

- Route: `N04-GM-CACHE-STREAMING-X`.
- Direct parent revision: N04 V011, source commit `6eef84ad3`.
- Candidate source commit: `5216bad0d`.
- Candidate source: `../submission.asc`.
- Candidate source SHA-256: `0863241837520aec1d42cd1a3bcea7bec3fbad82b05aa4fad43aa2283f401600`.
- Change: in the wide low-precision path, `residualGm_` changes from `CACHE_MODE_LAST` to `CACHE_MODE_NORMAL`; `xGm_` remains `CACHE_MODE_PERSISTENT` and `outputGm_` remains `CACHE_MODE_DISABLE`.
- Arithmetic, Vector instructions, Scalar indexing, tile traversal, DMA descriptors, and parameter residency are unchanged.

## Build and Correctness

- Host: `hwnput3` via `cann-server3`; CANN `8.5.0.alpha002`; SoC `Ascend910B3`.
- Paired sources: R31B V011 Champion SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`; N04 V012 SHA-256 `0863241837520aec1d42cd1a3bcea7bec3fbad82b05aa4fad43aa2283f401600`. Remote paths and Runner input identities are in `remote-inputs-retry.txt`.
- Existing `paired_runner_v016` CMake configure passed with return code 0. Final Compile passed with return code 0 after setting `ASCEND_HOME_PATH` and the C++ include paths used by the V011 build.
- Earlier Compile attempts remain in separate files: `paired-build-attempt1.log` records the missing `ASCEND_HOME_PATH` plugin crash (exit 139); `paired-build-attempt2.log` records missing `<vector>` when `CPATH` was unset; `paired-build-success.log` records the successful build. No failure log was replaced.
- Built executable: `artifacts/paired_runner_v016`, SHA-256 `d39dea43fb8880ffac2d29d0c3aa43eebb930082a4a502f67ef7fa42989d34f9`.
- Correctness: PASS, all four shapes for R31B V011 Champion and N04 V012, d7, Runner return code 0. The first launch failure caused by HCC `libstdc++` precedence remains in `correctness-v011-n04v012.log`; the successful run using the V011 `LD_LIBRARY_PATH` order is `correctness-v011-n04v012-retry.log`.

## Paired Local

- Device: d7 only. Existing Runner: `paired_runner_v016`; 45 warmups and 21 alternating pairs per shape; device-event time is primary. Timed Parent is R31B V011 Champion; Candidate is N04 V012. The complete 84-pair raw log is `local-paired-v011-n04v012.log`.
- Delta is Candidate minus Champion; negative values favor V012. All values below use the full 21-pair distribution; no sample was selected or discarded.

| Shape | Champion median us | V012 median us | Paired delta median us | Delta MAD us | Delta range us | Negative pairs |
|---|---:|---:|---:|---:|---:|---:|
| FP16 D=12288 | 353.040 | 338.580 | -14.120 | 2.080 | -28.420..-8.320 | 21/21 |
| FP16 D=32768 | 910.540 | 872.920 | -36.540 | 3.420 | -49.920..-22.360 | 21/21 |
| BF16 D=12288 | 423.060 | 410.860 | -12.760 | 1.240 | -19.440..-4.980 | 21/21 |
| BF16 D=32768 | 1413.060 | 1358.380 | -54.340 | 4.560 | -66.260..-43.340 | 21/21 |

All four paired-delta ranges remain below zero. The distributions are compact relative to the observed shifts; there is no sign-crossing outlier. `LOCAL_SCORE=INDICATIVE_GAIN` for this single Local run, not an Official result.

Before sampling, d7 reported 37% HBM use, AICore 2%, AIVector 4%, and HBM bandwidth 0%; an existing `python3` process (PID 439848, 21260 MiB) remained untouched. After sampling, the Runner had exited; AICore was 0%, AIVector 7%, and HBM bandwidth 0%. Snapshots are `pre-local-d7-recheck-1.txt` and `post-local-d7.txt`.

## Evidence Files

- CMake configure and all Compile attempts: `configure.log`, `paired-build-attempt1.log`, `paired-build-attempt2.log`, `paired-build-success.log`.
- Correctness raw output and snapshots: `correctness-v011-n04v012.log`, `correctness-v011-n04v012-retry.log`, `pre-correctness-retry-d7.txt`, `pre-correctness-retry-d7-recheck.txt`, `post-correctness-d7.txt`.
- Local raw output and snapshots: `local-paired-v011-n04v012.log`, `pre-local-d7.txt`, `pre-local-d7-recheck-1.txt`, `post-local-d7.txt`.
- Runner sources, CMake input, ABI header, and executable are retained in this directory.
