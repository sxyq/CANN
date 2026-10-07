# R-W4-4 V058

- Comparison Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Single change: `kSmallFp32BatchMaxWidth`, `6016 -> 6144`.
- Candidate SHA256: `b86de625abbe610f4c80e95d87b6aa3503b02cc7bdb8512c0a530ca1551dde44`.
- Compile: PASS for route targets and isolated Parent/Candidate probe targets; details in `v058-compile.log`.
- Result: partial correctness. Candidate fails at widths 6136 and 6144; only jointly passing tested width 6152 has numeric Local evidence. See `partial-ranking/V058-PARTIAL-RANKING-RESULT.md`.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent correctness failure; it was not repeated and is not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `CURRENT_LOCAL_BEST=NONE`.
