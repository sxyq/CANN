# MODE-DISPATCH-CUTOFF-X V105 Partial Local Result

- `DIRECT_PARENT=exact R31B-V011` (`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`). Candidate SHA256: `981007693524c7513021a112c0cc99900c08ce73d83c59caef44883286196c24`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 960`; the only Parent-to-Candidate source difference is this constant. C15 `1x32768` remains excluded for its known exact-Parent failure and was not rerun.
- Parent and Candidate Correctness passed FP32 `128x956`, `128x960`, and `128x964` (`6/6`, all `rc=0,bad=0`).
- Local used device 3, six interleaved P/C pairs per shape, 45 warmups, 31 samples/block, two blocks, batch 64. All 36 invocations returned `rc=0,bad=0`; all 2,232 raw device-event samples are retained.
- Pair speedup is `median(Parent raw device_us) / median(Candidate raw device_us)` over 62 samples per invocation. Shape score is the arithmetic mean of its six pair speedups; route score is the equal-weight geomean of the three shape scores. Combined-MAD test is `abs(parent median - candidate median) <= parent MAD + candidate MAD`.

| Shape | Six pair speedups | Shape score / delta | Candidate faster / within combined MAD | P/C pooled median us | P/C pooled CV | P/C throughput Gelem/s |
|---|---|---:|---:|---:|---:|---:|
| 128x956 | 0.912912, 0.967585, 1.052213, 0.985143, 1.053827, 0.988996 | 0.993446130864x / -0.655387% | 2/6 / 2/6 | 7.523595 / 7.614535 | 11.178894% / 6.782540% | 16.264565 / 16.070318 |
| 128x960 | 0.985769, 0.989503, 0.992156, 0.969481, 0.986655, 1.021338 | 0.990817217703x / -0.918278% | 1/6 / 6/6 | 7.458750 / 7.537190 | 3.505247% / 12.294530% | 16.474610 / 16.303158 |
| 128x964 | 1.058870, 1.063039, 1.004978, 0.980498, 0.935765, 0.946005 | 0.998192439500x / -0.180756% | 3/6 / 2/6 | 7.541095 / 7.615310 | 4.877045% / 4.216292% | 16.362610 / 16.203149 |
| Equal-shape geomean | - | **0.994147247782x / -0.585275%** | **6/18 / 10/18** | - | - | - |

## Quality and Decision

- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- `LOAD_QUALITY=STABLE_LOW_LOAD_DEVICE3`: all 36 pre/post pair snapshots show no NPU process on device 3; HBM usage 5%, AICore 0%, AIVector 0%, CtrlCPU 1-18%. Before/after Local snapshots indicate approximately 62,107 MB free HBM. All samples remain included.
- `MEASUREMENT_QUALITY=NOISY`: Candidate is faster in 6/18 pairs and 10/18 are within combined MAD. The 128x956 and 128x964 pair directions are mixed; no sample was excluded.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=exact R31B-V011`. V105 is not promoted.

## Timestamps

- Candidate edit: `2026-10-08T22:04:25.282224539Z`.
- Compile log final timestamp: `2026-10-08T22:05:39.943028696Z`; root `device/submission` and both route-bound reference probe targets passed.
- Correctness: `2026-10-08T22:06:52.410432350Z` to `2026-10-08T22:07:33.086302800Z`.
- Local capture: `2026-10-08T22:08:15.727711524Z` to `2026-10-08T22:13:34.930145336Z`; device 3 released at `2026-10-08T22:13:58.741701775Z`.
- `LOCAL_RESULT_TIMESTAMP=2026-10-08T22:15:13.209936583Z`; Local-capture-to-result gap `98.279791247 seconds`.
- `NEXT_EDIT_TIMESTAMP=PENDING_AFTER_V105_COMMIT`.
