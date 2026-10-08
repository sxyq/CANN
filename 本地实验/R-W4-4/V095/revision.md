# MODE-DISPATCH-CUTOFF-X V095 declaration

- `ROUTE=MODE-DISPATCH-CUTOFF-X` (`R-W4-4`); `REVISION=V095`.
- `DIRECT_PARENT=exact R31B-V011`; Parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1012`; this is the only planned Candidate change.
- `CANDIDATE_SOURCE_SHA256=4f6273566422a96ead41a66e09104618d740ac5f2041f900bf6917833f0d81fb`.
- `FOCUS_AXIS=FP32_SMALL_BATCH_CUTOFF_OFAT`.
- `WHY_NOT_DUPLICATE`: V086-V094 used cutoffs 1152, 1088, 1056, 1040, 1032, 1028, 1024, 1020, and 1016. Width 1012 appeared as a V093 test shape but was not used as a cutoff.
- `SELECTED_CASES=FP32 128x1008, 128x1016, 128x1024`.
- `CURRENT_LOCAL_BEST=R31B-V011`; V094 remains noisy and unpromoted.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for excluded exact-Parent C15 FP32 `1x32768`; do not rerun and do not label as Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- `COMPILE=PASS`; root `device` and `submission` targets returned rc 0.
- `CORRECTNESS=PASS` for Parent and Candidate on all selected shapes; six retry checks returned `rc=0,bad=0`, maximum absolute errors `1.43051e-6`, `1.19209e-6`, and `1.66893e-6` by width.
- The first six probe launches returned `rc=127` because `libgraph.so` was absent from the runtime library path; no kernel ran. The failure is preserved, the recorded toolkit environment was applied, and all six correctness checks then passed.
- `LOCAL_SCORE=0.990660425923x`; `LOCAL_DELTA=-0.933957408%`; `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL` with mixed/noisy measurements.
- `CURRENT_LOCAL_BEST=R31B-V011`; V095 is not promoted. Full calculations and load context are in `partial-ranking/V095-PARTIAL-RANKING-RESULT.md`.
