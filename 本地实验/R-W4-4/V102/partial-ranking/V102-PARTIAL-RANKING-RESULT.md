# MODE-DISPATCH-CUTOFF-X V102 Partial Local Result

- `DIRECT_PARENT=exact R31B-V011` (`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`). Candidate SHA256: `281fa9bc2eeb6ec6040f88ae26fac9dfef72e1e4f637fdcd1bc7ee580d195b7e`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 984`; the only Parent-to-Candidate source difference is this constant. C15 `1x32768` remains excluded for its known exact-Parent failure and was not rerun.
- Parent and Candidate Correctness passed FP32 `128x980`, `128x984`, and `128x988` (`6/6`, all `rc=0,bad=0`).
- Local used device 3, six interleaved P/C pairs per shape, 45 warmups, 31 samples/block, two blocks, batch 64. All 36 invocations returned `rc=0,bad=0`; all 2,232 raw device-event samples are retained.
- Pair speedup is `median(Parent raw device_us) / median(Candidate raw device_us)` over 62 samples per invocation. Shape score is the arithmetic mean of its six pair speedups; route score is the equal-weight geomean of the three shape scores. Combined-MAD test is `abs(parent median - candidate median) <= parent MAD + candidate MAD`.

| Shape | Six pair speedups | Shape score / delta | Candidate faster / within combined MAD | P/C pooled median us | P/C pooled CV | P/C throughput Gelem/s |
|---|---|---:|---:|---:|---:|---:|
| 128x980 | 1.011234, 0.965841, 1.011075, 0.960678, 0.954751, 0.939707 | 0.973881047803x / -2.611895% | 2/6 / 2/6 | 7.541715 / 7.802345 | 9.121839% / 37.787313% | 16.632822 / 16.077218 |
| 128x984 | 0.996589, 1.012898, 0.970943, 0.972378, 1.051672, 1.028449 | 1.005488239257x / +0.548824% | 3/6 / 2/6 | 7.599685 / 7.581875 | 3.773659% / 2.553847% | 16.573318 / 16.612250 |
| 128x988 | 0.950375, 0.994809, 0.985411, 1.045469, 0.989401, 0.999161 | 0.994104359897x / -0.589564% | 1/6 / 5/6 | 7.471245 / 7.566250 | 5.365965% / 3.497537% | 16.926764 / 16.714224 |
| Equal-shape geomean | - | **0.991071443565x / -0.892856%** | **6/18 / 9/18** | - | - | - |

## Quality and Decision

- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- `LOAD_QUALITY=STABLE_LOW_LOAD_DEVICE3`: all 36 pre/post pair snapshots show no NPU process on device 3; HBM usage 5%, AICore 0%, AIVector 0%, CtrlCPU 2-14%. Before and after Local snapshots reported 62,107-62,108 MB free HBM. All samples remain included.
- `MEASUREMENT_QUALITY=NOISY`: only 6/18 pair medians favor Candidate and 9/18 are within combined MAD. Candidate pooled CV at 128x980 is 37.787313%; no sample was excluded.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=exact R31B-V011`. V102 is not promoted.

## Timestamps

- Candidate edit: `2026-10-08T21:16:26.657196023Z`.
- Compile log final timestamp: `2026-10-08T21:17:35.350088577Z`; root `device/submission` and both route-bound reference probe targets passed.
- Correctness: `2026-10-08T21:18:33.336204863Z` to `2026-10-08T21:19:14.517161299Z`.
- Local capture: `2026-10-08T21:19:51.923464083Z` to `2026-10-08T21:25:04.682017570Z`; device 3 released at `2026-10-08T21:25:20.449028262Z`.
- `LOCAL_RESULT_TIMESTAMP=2026-10-08T21:26:25.647986193Z`; Local-capture-to-result gap `80.965968623 seconds`.
- `NEXT_EDIT_TIMESTAMP=PENDING_AFTER_V102_COMMIT`.
