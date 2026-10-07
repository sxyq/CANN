# R-W4-4 V055

- Comparison Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Single change: `kSmallFp32BatchMaxWidth`, `5632 -> 5760`.
- Candidate SHA256: `76d126423789f18d9f35311999b7323397232154c65f705c188ed324aaeea814`.
- Compile: PASS for route targets and isolated Parent/Candidate probe targets; details in `v055-compile.log`.
- Result: partial correctness. Candidate fails at widths 5752 and 5760; only jointly passing tested width 5768 has numeric Local evidence. See `partial-ranking/V055-PARTIAL-RANKING-RESULT.md`.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent correctness failure; it was not repeated and is not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `CURRENT_LOCAL_BEST=NONE`.
