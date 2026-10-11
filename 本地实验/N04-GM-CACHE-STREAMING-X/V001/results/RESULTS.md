# N04 V001 Results

## Source

- Route: N04-GM-CACHE-STREAMING-X
- Branch: `route/n04-gm-cache-streaming-x-20261011`
- Direct parent: `1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f`
- Candidate commit: `e498788ae5a577ff36970f27e4a8cab2cea45245`
- Candidate source: `本地实验/N04-GM-CACHE-STREAMING-X/V001/submission.asc`
- Source SHA256: `5bc717eabfcc9fe526c37f1901a283fa5bd3c546c4b33da150b36aec81b21091`
- Parent source SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Change: one `SetL2CacheHint(CACHE_MODE_DISABLE)` call on the wide-path `xGm_` input in `ProcessWideLowPrecision`. No arithmetic, tiling, prefetch, DMA descriptor, or parameter-residency change.
- Mechanism: bypass L2 for the large streaming `x` input on the GM/L2 access path.

## Build And Correctness

- Host: `hwnput3`, device d1, Ascend910B3; CANN `8.5.0.alpha002`.
- Remote run directory: `~/cann/server_runs/N04-GM-CACHE-STREAMING-X-RESET-20261011/V001/e498788a`.
- Adapter/device CMake configure and `full_link`: PASS, return code 0.
- Existing paired Runner CMake build: PASS, return code 0. The tool-generated registration unit needed `CPATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/c++/11/backward` to resolve `<vector>`.
- Correctness command: `paired_runner_v016 --correctness`; PASS, return code 0, four cases, Parent and Candidate each had zero failures.
- Cases: 1024x12288 FP16, 1024x32768 FP16, 1024x12288 BF16, 1024x32768 BF16.
- The separate `probe_v011` / `probe_v016` targets did not link: `rtLinkedDevBinaryRegisterDelay` and `__cce_rtKernelLaunch` were unresolved. They were not used for correctness.
- Earlier environment attempts and their return codes are retained in the neighboring logs; the successful Runner invocation loaded CANN and placed `/usr/lib/aarch64-linux-gnu` first for the system `libstdc++`.

## Paired Local

- Command: `paired_runner_v016 --paired`; 45 warmups and 21 alternating Parent/Candidate pairs per case; device events are primary, wall time diagnostic.
- Input rows: 1024. At width 32768, `x + residual` occupies about 128 MiB, targeting the large working-set route.
- Raw samples: `local-paired.log`; all 21 pairs for each of four cases are retained.
- Parent/Candidate device medians and paired-delta median/MAD (microseconds):

| Case | Parent median | Candidate median | Paired delta median | Delta MAD | Parent/Candidate CV |
|---|---:|---:|---:|---:|---:|
| FP16 D=12288 | 388.860 | 387.620 | -2.600 | 37.060 | 0.25470 / 0.31847 |
| FP16 D=32768 | 1111.620 | 1112.840 | -24.500 | 114.020 | 0.22497 / 0.09641 |
| BF16 D=12288 | 517.720 | 523.180 | 18.640 | 30.160 | 0.20755 / 0.30474 |
| BF16 D=32768 | 1679.420 | 1690.200 | 17.340 | 47.180 | 0.26491 / 0.21502 |

- Result: `LOCAL_SCORE=NONE`, benefit not demonstrated. Each paired-delta median is smaller than its MAD, with substantial outliers and sustained unrelated d1 load; no local extreme is treated as a score.
- Runner return code: 0. Post-run process query found no N04 Runner.

## Device And Evidence

- Pre-Local d1 snapshot at `pre-local-d1.txt`: HBM 59718/65536 MiB used (about 5818 MiB free), AICore 30%; existing VLLMEngineCor PID 2999855 used about 55686 MiB. It was left unchanged.
- Post-Local snapshot: `post-local-d1.txt`; HBM and the existing process remained at the same reported values. Other NPU processes and loads are recorded in the snapshots and were left unchanged.
- `/home/data4t2` had about 43 GB free (99% used); no large build tree was copied. Only the two 180 KiB adapter links and the 575 KiB paired Runner were copied under `artifacts/`.
- Compact build artifacts: `artifacts/device.alink`, `artifacts/submission.alink`, `artifacts/paired_runner_v016`.
- Compile, upload, dependency, device, Correctness, and Local command output and return codes are kept in the other files in this directory.
- `NEXT_ACTION`: test one output-path L2 hint as independent V002 on the same GM/L2 streaming axis; do not add another parameter or mechanism in that revision.
