# MODE-DISPATCH-CUTOFF-X V109 Partial Local Result

- `DIRECT_PARENT=exact R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 928`; Candidate SHA256 `ced5a1298e37ca68ec8e7edf7afb536a149b8e813f1e52a784961af4ed41041c`. Parent-to-Candidate diff contains only this constant.
- Compile PASS: root `device/submission` and route-bound `clx_device`, `clx_v001_submission`, `clx_ref_parent_probe`, `clx_ref_candidate_probe` targets.
- Correctness PASS: Parent and Candidate, FP32 `128x924`, `128x928`, `128x932`; six invocations, all `rc=0,bad=0`. C15 `1x32768` remains excluded as a known exact-Parent failure.
- Local used device 3, six interleaved P/C pairs per shape, 45 warmups, 31 samples/block, two blocks, batch 64. All 36 invocations returned `rc=0,bad=0`; all 2,232 device-event samples are retained.
- Pair speedup is `median(Parent raw device_us)/median(Candidate raw device_us)` over 62 samples per invocation. Shape score is the arithmetic mean of six pair speedups; route score is the equal-weight geomean of the three shape scores. Combined-MAD test is `abs(parent median-candidate median) <= parent MAD+candidate MAD`.

| Shape | Six pair speedups | Shape score / delta | Candidate faster / within combined MAD | P/C pooled median us | P/C pooled CV | P/C throughput Gelem/s |
|---|---|---:|---:|---:|---:|---:|
| 128x924 | 0.992103, 0.948675, 0.991501, 0.929956, 0.997661, 1.042446 | 0.983723636707x / -1.627636% | 1/6 / 4/6 | 7.468280 / 7.699535 | 5.415208% / 4.406735% | 15.836578 / 15.360928 |
| 128x928 | 1.005240, 0.971205, 0.966763, 0.977478, 0.969153, 0.936703 | 0.971090256516x / -2.890974% | 1/6 / 1/6 | 7.430470 / 7.646250 | 3.818691% / 2.718342% | 15.986068 / 15.534935 |
| 128x932 | 1.004391, 0.988396, 1.010766, 0.940755, 0.951954, 1.033355 | 0.988269560898x / -1.173044% | 3/6 / 3/6 | 7.599060 / 7.662655 | 3.603634% / 3.744038% | 15.698784 / 15.568494 |
| Equal-shape geomean | - | **0.981000828440x / -1.899917%** | **5/18 / 8/18** | - | - | - |

## Quality and Decision

- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- `LOAD_QUALITY=LOW_LOAD_DEVICE3`: 36 pre/post `npu-smi info` snapshots show HBM usage 3,428-3,431 MB of 65,536 MB, AICore 0%, and no device-3 process. AIVector/CtrlCPU were not reported by the captured command. Device 3 was released at `2026-10-08T23:49:58.124797567Z` after raw/numeric capture.
- `MEASUREMENT_QUALITY=NOISY`: Candidate was faster in 5/18 pairs; 8/18 are within combined MAD. All three shape aggregates are below 1.0. No samples were excluded.
- `LOCAL_VERDICT=LOCAL_REJECTED`; `CURRENT_LOCAL_BEST=exact R31B-V011`. V109 is not promoted.
- Standalone `npu-smi info proc` reported unsupported; full `npu-smi info` process tables and `ps` snapshots were retained.

## Timestamps

- Candidate edit: `2026-10-08T23:38:42.772911159Z`.
- Compile PASS log mtime: `2026-10-08T23:40:18.510844695Z`.
- Correctness: `2026-10-08T23:41:13.536279336Z` to `2026-10-08T23:41:58.685950218Z`.
- Local capture: `2026-10-08T23:42:49.310305442Z` to `2026-10-08T23:47:43.863775286Z`.
- `LOCAL_RESULT_TIMESTAMP=2026-10-08T23:49:01.803275281Z`; capture-to-result gap `77.939499995 seconds`.
- `DEVICE_RELEASE_TIMESTAMP=2026-10-08T23:49:58.124797567Z`.
- `NEXT_EDIT_TIMESTAMP=PENDING_AFTER_V109_COMMIT`.
