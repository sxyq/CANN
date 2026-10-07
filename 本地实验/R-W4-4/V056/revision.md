# R-W4-4 V056

- Comparison Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Single change: `kSmallFp32BatchMaxWidth`, `5760 -> 5888`.
- Candidate SHA256: `0e7bcb75487d0c1d106ec428262c0c457f15fa2b203e006bacb4b7f5f2611957`.
- Compile: PASS for route targets and isolated Parent/Candidate probe targets; details in `v056-compile.log`.
- Result: partial correctness. Candidate fails at widths 5880 and 5888; only jointly passing tested width 5896 has numeric Local evidence. See `partial-ranking/V056-PARTIAL-RANKING-RESULT.md`.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent correctness failure; it was not repeated and is not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `CURRENT_LOCAL_BEST=NONE`.
