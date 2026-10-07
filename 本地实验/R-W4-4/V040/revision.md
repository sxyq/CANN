# R-W4-4 V040

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V040`.
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `3584 -> 3840`.
- CANDIDATE_SOURCE_SHA256: `e06436370b797fe9d26c52354f328b1023e4dfee2192eac1880efe7f23d6c4ee`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not a V040 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; only Parent-valid widths `3832`, `3840`, and `3848` are included.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- CURRENT_LOCAL_BEST: `NONE`; V040 is not promoted.
- NEXT_PARENT: exact sibling `R31B-V011` unless Planning directs otherwise.

Compile, correctness, and numeric partial Local details are in `partial-ranking/V040-PARTIAL-RANKING-RESULT.md`.
