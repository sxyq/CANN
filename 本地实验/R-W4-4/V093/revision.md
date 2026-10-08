# MODE-DISPATCH-CUTOFF-X V093 declaration

- `ROUTE=MODE-DISPATCH-CUTOFF-X` (`R-W4-4`); `REVISION=V093`.
- `DIRECT_PARENT=exact R31B-V011`; Parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1020`.
- Candidate SHA256 `9fbe6deda3c5dbfdcf8b95f0b0433e2161e1c78af6578668a97f48b25cd6369e`.
- `FOCUS_AXIS=FP32_SMALL_BATCH_CUTOFF_OFAT`; selected FP32 shapes `128x1012`, `128x1020`, `128x1028`.
- `COMPILE=PASS`; configure/build rc 0 at `2026-10-08T15:42:58.532820065Z`-`2026-10-08T15:43:06.245709969Z`.
- `CORRECTNESS=PASS` for exact Parent and Candidate on all selected shapes; C15 remains excluded as a known Parent failure and was not rerun.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.
- Local used device 2, selected after a fresh snapshot showed 62,114 MB free and no NPU process on that device. Device was released after capture at `2026-10-08T16:00:18.397476895Z`.
- Numeric result and raw evidence are in `partial-ranking/V093-PARTIAL-RANKING-RESULT.md`.
