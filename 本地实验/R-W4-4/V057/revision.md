# R-W4-4 V057

- Comparison Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Single change: `kSmallFp32BatchMaxWidth`, `5888 -> 6016`.
- Candidate SHA256: `68e468935400d2645812e7de24497f4705892e0fcc7536fbea88dec3327c729e`.
- Compile: PASS for route targets and isolated Parent/Candidate probe targets; details in `v057-compile.log`.
- Result: partial correctness. Candidate fails at widths 6008 and 6016; only jointly passing tested width 6024 has numeric Local evidence. See `partial-ranking/V057-PARTIAL-RANKING-RESULT.md`.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent correctness failure; it was not repeated and is not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `CURRENT_LOCAL_BEST=NONE`.
