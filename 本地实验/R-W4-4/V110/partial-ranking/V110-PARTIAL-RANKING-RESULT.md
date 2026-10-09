# MODE-DISPATCH-CUTOFF-X V110 Partial Local Result

- `DIRECT_PARENT=exact R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 920`; Candidate SHA256 `7ea92a5921239f127218b968c298012577491bf1b0ef337d7b6561b9f295c3c4`. Parent-to-Candidate diff contains only this constant.
- Compile PASS: root `device/submission` and route-bound `clx_device`, `clx_v001_submission`, `clx_ref_parent_probe`, `clx_ref_candidate_probe` targets.
- Correctness PASS: Parent and Candidate, FP32 `128x916`, `128x920`, `128x924`; six invocations, all `rc=0,bad=0`. C15 `1x32768` remains excluded as a known exact-Parent failure.
- Local used device 3, six interleaved P/C pairs per shape, 45 warmups, 31 samples/block, two blocks, batch 64. All 36 invocations returned `rc=0,bad=0`; all 2,232 device-event samples are retained.
- Pair speedup is `median(Parent raw device_us)/median(Candidate raw device_us)` over 62 samples per invocation. Shape score is the arithmetic mean of six pair speedups; route score is the equal-weight geomean of the three shape scores. Combined-MAD test is `abs(parent median-candidate median) <= parent MAD+candidate MAD`.

| Shape | Six pair speedups | Shape score / delta | Candidate faster / within combined MAD | P/C pooled median us | P/C pooled CV | P/C throughput Gelem/s |
|---|---|---:|---:|---:|---:|---:|
| 128x916 | 0.984474, 0.956001, 0.945127, 0.946880, 1.044171, 1.004158 | 0.980135156134x / -1.986484% | 2/6 / 2/6 | 7.495000 / 7.713125 | 6.985929% / 4.259942% | 15.643496 / 15.201102 |
| 128x920 | 0.920494, 0.995245, 1.031666, 0.981613, 0.983119, 0.979561 | 0.981949729876x / -1.805027% | 1/6 / 4/6 | 7.456565 / 7.573440 | 89.204505% / 86.429621% | 15.792795 / 15.549077 |
| 128x924 | 0.976647, 1.062353, 0.975022, 0.946202, 0.944854, 0.952755 | 0.976305269157x / -2.369473% | 1/6 / 1/6 | 7.533595 / 7.769380 | 19.330738% / 9.841097% | 15.699278 / 15.222836 |
| Equal-shape geomean | - | **0.979460557368x / -2.053944%** | **4/18 / 7/18** | - | - | - |

## Quality and Decision

- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- `LOAD_QUALITY=LOW_LOAD_DEVICE3_WITH_TIMING_OUTLIERS`: 36 pre/post `npu-smi info` snapshots show HBM usage 3,428-3,431 MB of 65,536 MB, AICore 0%, and no device-3 process. AIVector/CtrlCPU were not reported by the captured command. Device 3 was released at `2026-10-09T00:04:51.265246712Z` after numeric capture.
- `MEASUREMENT_QUALITY=NOISY`: Candidate faster in 4/18 pairs; 7/18 are within combined MAD. At 128x920, Parent/Candidate pooled CV is 89.204505%/86.429621% with retained maxima 143.527/141.671 us; at 128x924 Parent CV is 19.330738%. No samples were excluded.
- `LOCAL_VERDICT=LOCAL_REJECTED`; `CURRENT_LOCAL_BEST=exact R31B-V011`. V110 is not promoted.
- Full `npu-smi info` process tables and `ps` snapshots are retained; the standalone process-query result is preserved as captured.

## Timestamps

- Candidate edit: `2026-10-08T23:53:50.877746290Z`.
- Compile PASS log mtime: `2026-10-08T23:55:25.166676013Z`.
- Correctness: `2026-10-08T23:56:14.798987337Z` to `2026-10-08T23:56:58.729768919Z`.
- Local capture: `2026-10-08T23:57:51.173137714Z` to `2026-10-09T00:02:46.568376459Z`.
- `LOCAL_RESULT_TIMESTAMP=2026-10-09T00:03:48.109151389Z`; capture-to-result gap `61.540774930 seconds`.
- `DEVICE_RELEASE_TIMESTAMP=2026-10-09T00:04:51.265246712Z`.
- `NEXT_EDIT_TIMESTAMP=PENDING_AFTER_V110_COMMIT`.
