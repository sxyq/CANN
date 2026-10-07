# R-W4-4 V059

- Comparison Parent: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Single change: `kSmallFp32BatchMaxWidth`, `6144 -> 6272`.
- Candidate SHA256: `8b9ecec2d72ea45c0bbac7391c16dc2fcdc2aac6fdd3cbf55fb36458acd63412`.
- Compile: PASS for route targets and isolated Parent/Candidate probe targets; details in `v059-compile.log`.
- Result: partial correctness. Candidate fails at widths 6264 and 6272; only jointly passing tested width 6280 has numeric Local evidence. See `partial-ranking/V059-PARTIAL-RANKING-RESULT.md`.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent correctness failure; it was not repeated and is not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `CURRENT_LOCAL_BEST=NONE`.
