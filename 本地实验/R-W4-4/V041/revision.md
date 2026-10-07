# R-W4-4 V041

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V041`.
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `3840 -> 3968`.
- CANDIDATE_SOURCE_SHA256: `bb051a75e2aee04938473de4cfab35dcdfc89769dd6e7d85c12563b744ea4a47`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not a V041 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; only Parent-valid widths `3960`, `3968`, and `3976` are included.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- CURRENT_LOCAL_BEST: `NONE`; V041 is not promoted.
- NEXT_PARENT: exact sibling `R31B-V011` unless Planning directs otherwise.

Compile, correctness, and numeric partial Local details are in `partial-ranking/V041-PARTIAL-RANKING-RESULT.md`.
