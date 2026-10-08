# MODE-DISPATCH-CUTOFF-X V100

- `DIRECT_PARENT=exact R31B-V011`; Parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 992`; Candidate SHA256 `1e1147d40dfe76bf37ae665024a8d9d42bef5bb8d2c5afab97e26c646d33cdb9`.
- `FOCUS_AXIS=FP32_SMALL_BATCH_CUTOFF_OFAT`; selected FP32 cases `128x988`, `128x992`, `128x996`. Cutoff 992 was not previously used; V099 used it only as a test shape.
- `COMPILE=PASS` at `2026-10-08T20:04:51.514715536Z`; `device`, `submission`, Parent probe, and Candidate probe targets passed.
- `CORRECTNESS=PASS` for Parent and Candidate on all three selected shapes, device 3; all six invocations were `rc=0,bad=0`.
- `LOCAL_SCORE=0.971523398408475x`; `LOCAL_DELTA=-2.847660159152%`; `LOCAL_VERDICT=LOCAL_REJECTED`.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `CURRENT_LOCAL_BEST=exact R31B-V011`; V100 is not promoted.
- Device 3 load was stable and no NPU process was present in 36/36 per-pair snapshots. Pair spread remains measurable; all samples are retained.
- Exact-Parent C15 FP32 `1x32768` remains excluded for the known Parent correctness failure; it was not rerun.
- Numeric and raw evidence: `partial-ranking/V100-PARTIAL-RANKING-RESULT.md` and retained TSV/log/snapshot files.
- `NEXT_EDIT_TIMESTAMP=PENDING_AFTER_V100_COMMIT`; next cutoff sibling remains rooted at exact R31B-V011.
