# MODE-DISPATCH-CUTOFF-X V092 declaration

- `ROUTE=MODE-DISPATCH-CUTOFF-X` (`R-W4-4`); `REVISION=V092`.
- `DIRECT_PARENT=exact R31B-V011`; Parent source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1024`; this is the sole performance change.
- `CONTEXT_CLASS=FP32_SMALL_BATCH_CUTOFF_OFAT`.
- `WHY_NOT_DUPLICATE`: V086-V091 narrowed the cutoff through 1152, 1088, 1056, 1040, 1032, and 1028; 1024 is the next lower bounded sibling and has not been used as this cutoff.
- `CURRENT_LOCAL_BEST=R31B-V011`; V091 remains noisy and unpromoted.
- `SELECTED_CASES=FP32 128x1016, 128x1024, 128x1032`.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for excluded C15 FP32 1x32768; this is not a Candidate regression and will not be rerun.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_AUTHORIZED`.
- Device 4 is assigned for this revision; capture a fresh resource/process snapshot after Compile and before NPU use. Do not disturb existing processes.
- The exact Parent is sourced from `线上结果/R31B/V011/submission.asc`, verified against V091's retained Parent copy. The differing `本地实验/R-W4-4/V011/submission.asc` copy is not used.
- `CANDIDATE_SOURCE_SHA256=f4b3623b4a8cb86a43328f5dbc15fe6826ab8f13cbcc88745d0e23aa9da2b49f`.
- `COMPILE=PASS`; configure/build rc 0 at `2026-10-08T15:10:27.464664586Z`-`2026-10-08T15:10:35.174437846Z`.
- `CORRECTNESS=PASS` for Parent and Candidate on all selected shapes. The initial loader-only attempt (`rc=127`, missing `libgraph.so`) and stale harness-copy build are retained; corrected harness build and correctness attempt passed.
- `LOCAL_SCORE=0.990767822684x`; `LOCAL_DELTA=-0.923217732%`; `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL` with noisy quality. `CURRENT_LOCAL_BEST=R31B-V011`; V092 is not promoted.
- `DEVICE4_RELEASED_UTC=2026-10-08T15:32:32.762388823Z`; existing processes were not touched.
- See `partial-ranking/V092-PARTIAL-RANKING-RESULT.md` for formula, pooled stats, load summary, raw paths, and release receipt.
