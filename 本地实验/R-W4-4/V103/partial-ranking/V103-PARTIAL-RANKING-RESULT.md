# MODE-DISPATCH-CUTOFF-X V103 Partial Local Result

- `DIRECT_PARENT=exact R31B-V011` (`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`). Candidate SHA256: `1d8237b0ec2b2b58cbc628089ca75ba8875b1af1872bc1dafc3bb2551908b442`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 976`; the only Parent-to-Candidate source difference is this constant. C15 `1x32768` remains excluded for its known exact-Parent failure and was not rerun.
- Parent and Candidate Correctness passed FP32 `128x972`, `128x976`, and `128x980` (`6/6`, all `rc=0,bad=0`).
- Local used device 3, six interleaved P/C pairs per shape, 45 warmups, 31 samples/block, two blocks, batch 64. All 36 invocations returned `rc=0,bad=0`; all 2,232 raw device-event samples are retained.
- Pair speedup is `median(Parent raw device_us) / median(Candidate raw device_us)` over 62 samples per invocation. Shape score is the arithmetic mean of its six pair speedups; route score is the equal-weight geomean of the three shape scores. Combined-MAD test is `abs(parent median - candidate median) <= parent MAD + candidate MAD`.

| Shape | Six pair speedups | Shape score / delta | Candidate faster / within combined MAD | P/C pooled median us | P/C pooled CV | P/C throughput Gelem/s |
|---|---|---:|---:|---:|---:|---:|
| 128x972 | 0.988325, 0.972190, 0.963031, 0.919209, 1.004357, 1.113775 | 0.993481133530x / -0.651887% | 2/6 / 2/6 | 7.547815 / 7.714845 | 46.508142% / 12.943489% | 16.483711 / 16.126831 |
| 128x976 | 0.955018, 1.046141, 1.007766, 1.005218, 0.939715, 1.006016 | 0.993312589117x / -0.668741% | 4/6 / 4/6 | 7.586410 / 7.720315 | 38.912071% / 5.684600% | 16.467341 / 16.181723 |
| 128x980 | 0.944141, 1.040649, 0.968679, 0.958309, 0.942797, 0.980099 | 0.972445763924x / -2.755424% | 1/6 / 2/6 | 7.580625 / 7.835625 | 3.581895% / 8.872559% | 16.547448 / 16.008934 |
| Equal-shape geomean | - | **0.986363480112x / -1.363652%** | **7/18 / 8/18** | - | - | - |

## Quality and Decision

- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- `LOAD_QUALITY=STABLE_LOW_LOAD_DEVICE3`: all 36 pre/post pair snapshots show no NPU process on device 3; HBM usage 5%, AICore 0%, AIVector 0%, CtrlCPU 1-14%. Before/after Local snapshots indicate approximately 62,107-62,108 MB free HBM. All samples remain included.
- `MEASUREMENT_QUALITY=NOISY`: only 7/18 pair medians favor Candidate and 8/18 are within combined MAD. Parent pooled CV at 128x972 and 128x976 reached 46.508142% and 38.912071%; no sample was excluded.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=exact R31B-V011`. V103 is not promoted.

## Timestamps

- Candidate edit: `2026-10-08T21:32:57.874567742Z`.
- Compile log final timestamp: `2026-10-08T21:34:05.411057867Z`; root `device/submission` and both route-bound reference probe targets passed.
- Correctness: `2026-10-08T21:34:55.806241178Z` to `2026-10-08T21:35:36.504224098Z`.
- Local capture: `2026-10-08T21:36:28.698623663Z` to `2026-10-08T21:41:47.476934962Z`; device 3 released at `2026-10-08T21:42:20.516406799Z`.
- `LOCAL_RESULT_TIMESTAMP=2026-10-08T21:43:31.886413762Z`; Local-capture-to-result gap `104.409478800 seconds`.
- `NEXT_EDIT_TIMESTAMP=PENDING_AFTER_V103_COMMIT`.
