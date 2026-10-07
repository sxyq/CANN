# R-W4-4 V054

- Direct Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Single change: `kSmallFp32BatchMaxWidth`, `5504 -> 5632`.
- Candidate SHA256: `8160756d4f1767383af7137f473448e4c9ed5fa5967ced92e73c530be8da1247`.
- Compile: PASS for route targets and isolated Parent/Candidate probe targets; details in `v054-compile.log`.
- Result: partial correctness. Candidate fails at widths 5624 and 5632; only jointly passing tested width 5640 has numeric Local evidence. See `partial-ranking/V054-PARTIAL-RANKING-RESULT.md`.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent correctness failure; it was not repeated and is not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `CURRENT_LOCAL_BEST=NONE`.
