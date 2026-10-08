# MODE-DISPATCH-CUTOFF-X V096 declaration

- `ROUTE=MODE-DISPATCH-CUTOFF-X` (`R-W4-4`); `REVISION=V096`.
- `DIRECT_PARENT=exact R31B-V011`; Parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1008`; no other Candidate-source change.
- `FOCUS_AXIS=FP32_SMALL_BATCH_CUTOFF_OFAT`.
- `WHY_NOT_DUPLICATE`: the route-local cutoff declarations through V095 do not use 1008 as the cutoff; V094 includes width 1008 only as a tested shape.
- `SELECTED_CASES=FP32 128x1008, 128x1012, 128x1016`.
- `CURRENT_LOCAL_BEST=exact R31B-V011`; V095 is not promoted.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for excluded exact-Parent C15 FP32 `1x32768`; do not rerun.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; no shared records, Online, or push.
