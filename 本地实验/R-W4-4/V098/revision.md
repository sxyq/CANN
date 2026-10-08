# MODE-DISPATCH-CUTOFF-X V098

- `DIRECT_PARENT=exact R31B-V011`; Parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1000`; Candidate SHA256 `064e28930a69c6c921952a554765061f5df780df536ee52eb6f68f177d72a38c`.
- `FOCUS_AXIS=FP32_SMALL_BATCH_CUTOFF_OFAT`; cases: FP32 `128x996`, `128x1000`, `128x1004`.
- `COMPILE=PASS`; Parent and Candidate Correctness PASS on all cases.
- `LOCAL_SCORE=0.972207290914x`; `LOCAL_DELTA=-2.779270909%`; `LOCAL_VERDICT=LOCAL_REJECTED`.
- `CURRENT_LOCAL_BEST=exact R31B-V011`; V098 is not promoted. `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.
- Exact-Parent C15 FP32 `1x32768` remains excluded for the known Parent correctness failure; it was not run.
- Raw measurements and quality detail: `partial-ranking/V098-PARTIAL-RANKING-RESULT.md` and `local-result.json`.
