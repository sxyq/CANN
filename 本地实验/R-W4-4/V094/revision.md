# MODE-DISPATCH-CUTOFF-X V094 declaration

- `ROUTE=MODE-DISPATCH-CUTOFF-X` (`R-W4-4`); `REVISION=V094`.
- `DIRECT_PARENT=exact R31B-V011`; Parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1016`; no other performance change.
- `CANDIDATE_SOURCE_SHA256=cfa9be0899be047b47c92186188529e2c69a8eb476d898aa07b73005398c78d8`.
- `FOCUS_AXIS=FP32_SMALL_BATCH_CUTOFF_OFAT`; selected FP32 shapes `128x1008`, `128x1016`, `128x1024`.
- `WHY_NOT_DUPLICATE`: V086-V093 tested cutoffs 1152, 1088, 1056, 1040, 1032, 1028, 1024, and 1020. A cutoff of 1016 is unused; prior uses of width 1016 as a test shape do not duplicate the cutoff change.
- `COMPILE=PASS`; environment, CMake configure, and `device`/`submission` build all returned 0 at `2026-10-08T16:26:29.966700022Z`-`2026-10-08T16:26:40.873568134Z`.
- `CORRECTNESS=PASS` for exact Parent and Candidate on all three selected shapes; six checks returned `rc=0,bad=0`. Maximum absolute errors by width were `1.43051e-6`, `1.19209e-6`, and `1.66893e-6`.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for excluded C15 FP32 `1x32768`; C15 was not rerun and this is not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- `LOCAL_SCORE=0.995114327762x`; `LOCAL_DELTA=-0.488567224%`; `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`.
- `CURRENT_LOCAL_BEST=R31B-V011`; V094 is not promoted. See `partial-ranking/V094-PARTIAL-RANKING-RESULT.md`.
