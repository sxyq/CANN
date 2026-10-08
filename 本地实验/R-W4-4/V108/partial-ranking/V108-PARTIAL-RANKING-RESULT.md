# MODE-DISPATCH-CUTOFF-X V108 Partial Local Result

- `DIRECT_PARENT=exact R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 936`; Candidate SHA256 `38834732c2d50c1be910a93b1c153068314ae56d5dbe88b0141512c7a49d5bc5`. Parent-to-Candidate diff contains only this constant.
- Compile PASS: root `device/submission` and route-bound `clx_device`, `clx_v001_submission`, `clx_ref_parent_probe`, `clx_ref_candidate_probe` targets.
- Correctness PASS: Parent and Candidate, FP32 `128x932`, `128x936`, `128x940`; six invocations, all `rc=0,bad=0`. C15 `1x32768` remains excluded as a known exact-Parent failure.
- Local used device 3, six interleaved P/C pairs per shape, 45 warmups, 31 samples/block, two blocks, batch 64. All 36 invocations returned `rc=0,bad=0`; all 2,232 device-event samples are retained.
- Pair speedup is `median(Parent raw device_us)/median(Candidate raw device_us)` over 62 samples per invocation. Shape score is the arithmetic mean of six pair speedups; route score is the equal-weight geomean of the three shape scores. Combined-MAD test is `abs(parent median-candidate median) <= parent MAD+candidate MAD`.

| Shape | Six pair speedups | Shape score / delta | Candidate faster / within combined MAD | P/C pooled median us | P/C pooled CV | P/C throughput Gelem/s |
|---|---|---:|---:|---:|---:|---:|
| 128x932 | 0.988538, 0.938496, 0.986421, 0.976796, 0.960385, 0.987661 | 0.973049273415x / -2.695073% | 0/6 / 4/6 | 7.443120 / 7.628440 | 3.056373% / 88.339988% | 16.027687 / 15.638322 |
| 128x936 | 1.030617, 1.013288, 0.983341, 0.968952, 0.937685, 1.068618 | 1.000416852827x / +0.041685% | 3/6 / 3/6 | 7.552185 / 7.570470 | 29.092042% / 4.386409% | 15.864018 / 15.825702 |
| 128x940 | 0.995018, 0.927520, 1.025601, 1.015455, 0.992680, 0.921710 | 0.979663821830x / -2.033618% | 2/6 / 4/6 | 7.604220 / 7.798125 | 7.732814% / 7.734493% | 15.822793 / 15.429350 |
| Equal-shape geomean | - | **0.984307892322x / -1.569211%** | **5/18 / 11/18** | - | - | - |

## Quality and Decision

- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- `LOAD_QUALITY=LOW_LOAD_DEVICE3`: 36 pre/post `npu-smi info` snapshots show HBM usage 3,428-3,431 MB of 65,536 MB, AICore 0%, and no device-3 process. AIVector/CtrlCPU were not reported by the captured command. Device 3 was explicitly released at `2026-10-08T23:32:42.994994762Z` after numeric capture.
- `MEASUREMENT_QUALITY=NOISY`: Candidate faster in 5/18 pairs; 11/18 are within combined MAD. At 128x932, Candidate pooled CV is 88.339988% with a retained 145.287 us device event; at 128x936, Parent pooled CV is 29.092042% with a retained 48.7409 us event. No samples were excluded.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=exact R31B-V011`. V108 is not promoted.
- The standalone `npu-smi info proc` snapshot reported unsupported; full `npu-smi info` process tables and `ps` snapshots were retained.
- The first post-run JSON/raw validator had a JavaScript syntax error before file access; the corrected validator passed. Details are retained in `v108-result-analysis-command-errors.txt`.

## Timestamps

- Candidate edit: `2026-10-08T23:19:58.592058126Z`.
- Compile PASS log mtime: `2026-10-08T23:21:33.848219118Z`.
- Correctness: `2026-10-08T23:22:36.013424910Z` to `2026-10-08T23:23:19.650833282Z`.
- Local capture: `2026-10-08T23:24:28.058931718Z` to `2026-10-08T23:29:21.916145320Z`.
- `LOCAL_RESULT_TIMESTAMP=2026-10-08T23:31:22.155935465Z`; capture-to-result gap `120.239790145 seconds`.
- `DEVICE_RELEASE_TIMESTAMP=2026-10-08T23:32:42.994994762Z`.
- `NEXT_EDIT_TIMESTAMP=PENDING_AFTER_V108_COMMIT`.
