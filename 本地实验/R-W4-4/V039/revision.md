# R-W4-4 V039

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V039`.
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `3072 -> 3584`.
- CANDIDATE_SOURCE_SHA256: `453f7acd16f48d5cc53879466883a09b8b4ee42daaa5a5415d3eab649fd08e8d`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not a V039 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; only Parent-valid widths `3576`, `3584`, and `3592` are included.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- CURRENT_LOCAL_BEST: `NONE`; V039 is not promoted.
- NEXT_PARENT: exact sibling `R31B-V011` unless Planning directs otherwise.

Compile, correctness, and numeric partial Local details are in `partial-ranking/V039-PARTIAL-RANKING-RESULT.md`.
