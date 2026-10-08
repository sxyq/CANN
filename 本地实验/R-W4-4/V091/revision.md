# MODE-DISPATCH-CUTOFF-X V091 declaration

- `ROUTE=MODE-DISPATCH-CUTOFF-X` (`R-W4-4`); `REVISION=V091`.
- `DIRECT_PARENT=exact R31B-V011`; Parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1028`; no other performance change.
- `CANDIDATE_SOURCE_SHA256=f41d58080592a7dfb3d9b827d8b8bebe69a2977fa85098a5d660bd334f5df952`.
- `SOURCE_EDIT_UTC=2026-10-08T12:53:54.147992662Z` (submission source file mtime).
- V090 raw Local ended `2026-10-08T12:20:08.216139406Z`; result-to-edit gap is `2025.932 s` (33m45.932s), exceeding the 180-second target by `1845.932 s` (30m45.932s). This is the actual gap; no backdating.
- `CURRENT_LOCAL_BEST=R31B-V011`; V090 is not a promoted baseline. C15 FP32 1x32768 remains excluded as a known exact-Parent correctness failure, not a Candidate regression.
- `SELECTED_CASES=FP32 128x1020, 128x1028, 128x1036`.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_AUTHORIZED`.
- `COMPILE=PASS`; CMake configure rc 0 and `device`/`submission` build rc 0 at `2026-10-08T12:54:29.163374365Z`. See `v091-cmake-configure.log`, `v091-compile.log`, and `v091-compile-result.txt`.
- V091 Parent/Candidate correctness and Local timing completed; see `partial-ranking/V091-PARTIAL-RANKING-RESULT.md` for the raw-derived result. Device 4 was released at `2026-10-08T13:15:05.582658988Z`.
