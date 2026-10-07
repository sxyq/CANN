# R-W4-4 V038

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V038`.
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4096 -> 3072`.
- CANDIDATE_SOURCE_SHA256: `2d7f27e684d1a07a35a7a0ec7178a4589dbf3acf975ac77dd9a5a3994c00bdea`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not a V038 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; only Parent-valid widths `3064`, `3072`, and `3080` are included.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- CURRENT_LOCAL_BEST: `NONE`; V038 is not promoted.
- NEXT_PARENT: exact sibling `R31B-V011` unless Planning directs otherwise.

Compile, correctness, and numeric partial Local details are in `partial-ranking/V038-PARTIAL-RANKING-RESULT.md`.
