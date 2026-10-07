# R-W4-4 V060

- Comparison Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Single change: `kSmallFp32BatchMaxWidth`, `6272 -> 6400`.
- Candidate SHA256: `0bff5757051627eb1952c34b02f1dc82b6494480aa8a5d68adc663d97ceaf370`.
- Compile: PASS for route targets and isolated Parent/Candidate probe targets; details in `v060-compile.log`.
- Result: partial correctness. Candidate fails at widths 6392 and 6400; only jointly passing tested width 6408 has numeric Local evidence. See `partial-ranking/V060-PARTIAL-RANKING-RESULT.md`.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent correctness failure; it was not repeated and is not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `CURRENT_LOCAL_BEST=NONE`.
