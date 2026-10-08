# MODE-DISPATCH-CUTOFF-X V107 Partial Local Result

- `DIRECT_PARENT=exact R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 944`; Candidate SHA256 `a1afa288059e67a6b3086b9029e192bb3922e58e7c37acd293b13d25505c806c`. Parent-to-Candidate diff contains only this constant.
- Compile PASS: root `device/submission` and route-bound `clx_device`, `clx_v001_submission`, `clx_ref_parent_probe`, `clx_ref_candidate_probe` targets.
- Correctness PASS: Parent and Candidate, FP32 `128x940`, `128x944`, `128x948`; six invocations, all `rc=0,bad=0`. C15 `1x32768` remains excluded as a known exact-Parent failure.
- Local used device 3, six interleaved P/C pairs per shape, 45 warmups, 31 samples/block, two blocks, batch 64. All 36 invocations returned `rc=0,bad=0`; all 2,232 device-event samples are retained.
- Pair speedup is `median(Parent raw device_us)/median(Candidate raw device_us)` over 62 samples per invocation. Shape score is the arithmetic mean of six pair speedups; route score is the equal-weight geomean of the three shape scores. Combined-MAD test is `abs(parent median-candidate median) <= parent MAD+candidate MAD`.

| Shape | Six pair speedups | Shape score / delta | Candidate faster / within combined MAD | P/C pooled median us | P/C pooled CV | P/C throughput Gelem/s |
|---|---|---:|---:|---:|---:|---:|
| 128x940 | 0.968731, 0.986361, 0.974613, 0.929385, 0.972827, 0.964951 | 0.966144618320x / -3.385538% | 0/6 / 3/6 | 7.420155 / 7.691410 | 2.839537% / 11.221420% | 16.215295 / 15.643426 |
| 128x944 | 0.981867, 0.946952, 0.924434, 1.025596, 1.073640, 0.988213 | 0.990117010262x / -0.988299% | 2/6 / 3/6 | 7.642815 / 7.692190 | 5.276149% / 5.656615% | 15.809882 / 15.708400 |
| 128x948 | 0.990397, 0.995286, 0.986224, 0.993445, 0.989163, 1.005062 | 0.993262940081x / -0.673706% | 1/6 / 6/6 | 7.538750 / 7.597500 | 7.155654% / 2.772346% | 16.096037 / 15.971570 |
| Equal-shape geomean | - | **0.983099852290x / -1.690015%** | **3/18 / 12/18** | - | - | - |

## Quality and Decision

- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- `LOAD_QUALITY=LOW_LOAD_DEVICE3`: 36 pre/post `npu-smi info` snapshots show HBM usage 3,428-3,431 MB of 65,536 MB, AICore 0%, and no device-3 process. AIVector/CtrlCPU were not reported by the captured command. Device 3 was explicitly released at `2026-10-08T23:08:42.017509640Z` after raw/numeric result capture.
- `MEASUREMENT_QUALITY=NOISY`: Candidate was faster in 3/18 pairs; 12/18 are within combined MAD. The largest pooled CVs are Candidate 11.221420% at 128x940 and Parent 7.155654% at 128x948. No samples were excluded.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=exact R31B-V011`. V107 is not promoted.
- The attempted `npu-smi info proc` snapshot reported unsupported; the full `npu-smi info` process table and `ps` snapshots were retained. One result-file path lookup used the wrong filename order, failed without modifying data, and was corrected before derivation.

## Timestamps

- Candidate edit: `2026-10-08T22:47:05.975550208Z`.
- Compile PASS log mtime: `2026-10-08T22:48:52.484778086Z`.
- Correctness: `2026-10-08T22:50:50.059715313Z` to `2026-10-08T22:51:34.200304381Z`.
- Local capture: `2026-10-08T22:54:32.404041099Z` to `2026-10-08T22:59:23.939265084Z`.
- `LOCAL_RESULT_TIMESTAMP=2026-10-08T23:03:36.819579764Z`; capture-to-result gap `252.880314680 seconds`.
- `DEVICE_RELEASE_TIMESTAMP=2026-10-08T23:08:42.017509640Z`.
- `NEXT_EDIT_TIMESTAMP=PENDING_AFTER_V107_COMMIT`.
