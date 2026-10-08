# MODE-DISPATCH-CUTOFF-X V101 Partial Local Result

- `DIRECT_PARENT=exact R31B-V011` (`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`). Candidate SHA256: `bd17c9fd3d500629c96895774e87fa72b518fc643a77f0d66902158cc7e948e5`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 988`; the parent-to-candidate diff is one constant only. C15 `1x32768` remains excluded for its known exact-Parent failure and was not rerun.
- Parent and Candidate Correctness passed FP32 `128x984`, `128x988`, and `128x992` (`6/6`, all `rc=0,bad=0`).
- Local used device 3, six interleaved P/C pairs per shape, 45 warmups, 31 samples/block, two blocks, batch 64. All 36 invocations returned `rc=0,bad=0`; all 2,232 raw device-event samples are retained.
- Pair speedup is `median(Parent raw device_us) / median(Candidate raw device_us)` over 62 samples per invocation. Shape score is the arithmetic mean of its six pair speedups; route score is the equal-weight geomean of the three shape scores. Combined-MAD direction test is `abs(parent median - candidate median) <= parent MAD + candidate MAD`.

| Shape | Six pair speedups | Shape score / delta | Candidate faster / within combined MAD | P/C pooled median us | P/C pooled CV | P/C throughput Gelem/s |
|---|---|---:|---:|---:|---:|---:|
| 128x984 | 0.993628, 0.995255, 1.012443, 1.039676, 0.975343, 1.012034 | 1.004729958812x / +0.472996% | 3/6 / 5/6 | 7.584845 / 7.580620 | 9.564015% / 2.429021% | 16.605745 / 16.615000 |
| 128x988 | 0.994505, 0.990832, 0.940739, 0.949784, 0.922350, 0.980547 | 0.963126018979x / -3.687398% | 0/6 / 3/6 | 7.435155 / 7.694690 | 2.691432% / 7.801535% | 17.008926 / 16.435230 |
| 128x992 | 0.996947, 0.991611, 1.016697, 1.782730, 0.952147, 0.958041 | 1.116362291323x / +11.636229% | 2/6 / 4/6 | 7.651405 / 7.647655 | 25.668040% / 6.151179% | 16.595122 / 16.603259 |
| Equal-shape geomean | - | **1.026075241946x / +2.607524%** | **5/18 / 12/18** | - | - | - |

## Quality and Decision

- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- `LOAD_QUALITY=STABLE_LOW_LOAD_DEVICE3`: 36 pre/post pair snapshots show no NPU process on device 3; HBM usage was 5%, AICore 0%, AIVector 0%, and CtrlCPU 2-17%. Free HBM was 62,108 MB in the before/after device snapshots. All samples remain included.
- `MEASUREMENT_QUALITY=NOISY`: 12/18 pair medians are within combined MAD and Candidate is faster in only 5/18 pairs. The 128x992 Parent pair 04 median is 13.310300 us versus 7.466245 us for Candidate, producing a 1.782730x pair speedup outside combined MAD; its Parent pooled CV is 25.668040%. This value was not excluded.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=exact R31B-V011`. V101 is not promoted despite its positive partial route score.
- The first default-all build failed (`rc=2`) because it selected an absent generic `runner_main.inc` path and unrelated `clx_full_link` target. The existing route-bound `ref_*` probe targets and root `device/submission` targets passed on the direct target rebuild. Both failed and passing build logs are retained; no extra Candidate change was made.
- Runtime setup attempts that failed before entering a probe are preserved: initial invocations lacked `libgraph.so`; the next attempt selected an older HCC `libstdc++`. With `set_env.sh` sourced and `/lib/aarch64-linux-gnu` first in `LD_LIBRARY_PATH`, all six Correctness probes passed.

## Timestamps

- Candidate edit mtime: `2026-10-08T20:36:38.242095005Z`.
- Compile log birth/pass-end mtimes: `20:37:08.694407605Z` / `20:39:27.031813146Z`.
- Correctness: `20:44:46.942278077Z` to `20:45:28.763211036Z`.
- Local capture: `20:47:58.445373080Z` to `20:53:19.342221313Z`; device 3 released at `20:53:40.011869427Z`.
- `LOCAL_RESULT_TIMESTAMP=2026-10-08T20:58:57.309556102Z`; capture-to-result gap `337.967334789 seconds`.
- `NEXT_EDIT_TIMESTAMP=PENDING_AFTER_V101_COMMIT`.
