# MODE-DISPATCH-CUTOFF-X V104 Partial Local Result

- `DIRECT_PARENT=exact R31B-V011` (`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`). Candidate SHA256: `e4f18e486d90f67f9b0a2907fe47e7b8625d567bc3141c369f4b56d4ba307b0d`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 968`; the only Parent-to-Candidate source difference is this constant. C15 `1x32768` remains excluded for its known exact-Parent failure and was not rerun.
- Parent and Candidate Correctness passed FP32 `128x964`, `128x968`, and `128x972` (`6/6`, all `rc=0,bad=0`).
- Local used device 3, six interleaved P/C pairs per shape, 45 warmups, 31 samples/block, two blocks, batch 64. All 36 invocations returned `rc=0,bad=0`; all 2,232 raw device-event samples are retained.
- Pair speedup is `median(Parent raw device_us) / median(Candidate raw device_us)` over 62 samples per invocation. Shape score is the arithmetic mean of its six pair speedups; route score is the equal-weight geomean of the three shape scores. Combined-MAD test is `abs(parent median - candidate median) <= parent MAD + candidate MAD`.

| Shape | Six pair speedups | Shape score / delta | Candidate faster / within combined MAD | P/C pooled median us | P/C pooled CV | P/C throughput Gelem/s |
|---|---|---:|---:|---:|---:|---:|
| 128x964 | 0.964680, 0.941963, 0.992594, 0.946133, 0.934935, 0.992842 | 0.962191032763x / -3.780897% | 0/6 / 3/6 | 7.482810 / 7.757345 | 11.693578% / 113.301677% | 16.490062 / 15.906473 |
| 128x968 | 1.027392, 0.974763, 0.990827, 0.986763, 1.002753, 1.007914 | 0.998401983586x / -0.159802% | 3/6 / 5/6 | 7.548595 / 7.568440 | 6.126281% / 2.506362% | 16.414180 / 16.371141 |
| 128x972 | 1.026284, 1.019278, 0.984508, 1.035416, 0.995268, 0.976046 | 1.006133330112x / +0.613333% | 3/6 / 4/6 | 7.776565 / 7.621565 | 24.444587% / 8.977212% | 15.998838 / 16.324206 |
| Equal-shape geomean | - | **0.988721759601x / -1.127824%** | **6/18 / 12/18** | - | - | - |

## Quality and Decision

- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- `LOAD_QUALITY=STABLE_LOW_LOAD_DEVICE3`: all 36 pre/post pair snapshots show no NPU process on device 3; HBM usage 5%, AICore 0%, AIVector 0%, CtrlCPU 2-14%. Before/after Local snapshots indicate approximately 62,107-62,108 MB free HBM. All samples remain included.
- `MEASUREMENT_QUALITY=NOISY`: Candidate is faster in only 6/18 pairs; 12/18 are within combined MAD. Candidate pooled CV at 128x964 is 113.301677%; no sample was excluded.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=exact R31B-V011`. V104 is not promoted.

## Timestamps

- Candidate edit: `2026-10-08T21:48:51.333011646Z`.
- Compile log final timestamp: `2026-10-08T21:50:02.645628446Z`; root `device/submission` and both route-bound reference probe targets passed.
- Correctness: `2026-10-08T21:51:04.561373960Z` to `2026-10-08T21:51:44.893772834Z`.
- Local capture: `2026-10-08T21:52:39.635449567Z` to `2026-10-08T21:58:01.281674302Z`; device 3 released at `2026-10-08T21:58:26.129543142Z`.
- `LOCAL_RESULT_TIMESTAMP=2026-10-08T21:59:18.536935416Z`; Local-capture-to-result gap `77.255261114 seconds`.
- `NEXT_EDIT_TIMESTAMP=PENDING_AFTER_V104_COMMIT`.
