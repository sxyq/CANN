# MODE-DISPATCH-CUTOFF-X V100 Partial Local Result

- `DIRECT_PARENT=exact R31B-V011` (SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`). Candidate SHA256 `1e1147d40dfe76bf37ae665024a8d9d42bef5bb8d2c5afab97e26c646d33cdb9`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 992`; this was the only Candidate-source change. Cutoff 992 was not previously used; V099 used it only as a test shape.
- Selected cases were FP32 `128x988`, `128x992`, and `128x996`. Parent and Candidate Correctness passed all three (`rc=0,bad=0`). C15 `1x32768` remains excluded for its known exact-Parent failure and was not rerun.
- Interleaved Local used device 3, six P/C pairs per shape, 45 warmups, 31 samples/block, two blocks, batch 64. All 36 invocations returned `rc=0,bad=0`; all 2,232 raw device-event samples were retained, with no exclusions.
- Formula: each pair speedup is `median(Parent raw device_us) / median(Candidate raw device_us)` over 62 samples per invocation; shape score is the arithmetic mean of six pair speedups; route score is the equal-weight geomean of the three shape scores.

| Shape | Six pair speedups | Shape score / delta | Candidate faster / within combined MAD | P/C pooled median us | P/C pooled CV | P/C throughput Gelem/s |
|---|---|---:|---:|---:|---:|---:|
| 128x988 | 0.968609, 1.004384, 0.974435, 0.958487, 0.966480, 1.008362 | 0.980125884692x / -1.987412% | 2/6 / 3/6 | 7.515625 / 7.641405 | 3.293188% / 3.976557% | 16.826811 / 16.549836 |
| 128x992 | 0.991034, 0.984200, 0.943241, 0.985551, 0.894634, 0.972273 | 0.961822160777x / -3.817784% | 0/6 / 3/6 | 7.493595 / 7.831720 | 3.556640% / 4.895449% | 16.944604 / 16.213041 |
| 128x996 | 0.952381, 0.938644, 0.992492, 1.006484, 1.012424, 0.933833 | 0.972709496987x / -2.729050% | 2/6 / 3/6 | 7.501565 / 7.739530 | 3.157481% / 4.776498% | 16.994854 / 16.472318 |
| Equal-shape geomean | - | **0.971523398408x / -2.847660%** | **4/18 / 9/18** | - | - | - |

## Quality and Decision

- `LOCAL_SCORE=0.971523398408475x`; `LOCAL_DELTA=-2.847660159152%`. This is a partial route-local score only: `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.
- `LOAD_QUALITY=STABLE_LOW_LOAD_DEVICE3`: across 36 pre/post pair snapshots, device 3 had no NPU process, HBM usage rate 5%, AICore 0%, and AIVector 0%; CtrlCPU ranged 2-14%. Free HBM was 62,108 MB before Local and 62,107 MB after.
- `MEASUREMENT_QUALITY=PAIR_SPREAD_PRESENT`: Parent/Candidate pooled CV stayed 3.157-4.895%; 9/18 pair medians were within combined MAD. The numeric result retains every sample.
- `LOCAL_VERDICT=LOCAL_REJECTED`; `CURRENT_LOCAL_BEST=exact R31B-V011`; V100 is not promoted. All three aggregate shape scores were below 1.0 and Candidate was faster in only 4/18 pairs.
- `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- Local capture ended `2026-10-08T20:14:24.873142564Z`; `LOCAL_RESULT_TIMESTAMP=2026-10-08T20:22:07.076265459Z`; `NEXT_EDIT_TIMESTAMP=PENDING_AFTER_V100_COMMIT`.
- `NON_EXECUTION_GAP=462.203122895 seconds` from final Local capture to numeric result verification. Device 3 was released at `2026-10-08T20:15:21.505975294Z` after the post-Local snapshot.

## Gate Timestamps

- Candidate edit mtime: `2026-10-08T20:03:35.544844440Z`.
- Compile: `2026-10-08T20:03:59.591890521Z` to `2026-10-08T20:04:51.514715536Z`; targets `device`, `submission`, `clx_ref_parent_probe`, and `clx_ref_candidate_probe` passed. See `v100-compile-gate-receipt.txt`; full output is retained in `v100-compile.log`.
- Correctness: `2026-10-08T20:06:45.559240671Z` to `2026-10-08T20:07:26.318083070Z`.
- Local: `2026-10-08T20:09:00.189629587Z` to `2026-10-08T20:14:24.873142564Z`; device 3 release at `20:15:21.505975294Z`.
- Raw event TSVs, per-invocation stats/commands/return codes/stdout/stderr, and pre/post device/process snapshots are retained alongside this note.
