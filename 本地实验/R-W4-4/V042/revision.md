# R-W4-4 V042

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V042`.
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `3968 -> 4096` (restores the exact Parent value).
- CANDIDATE_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SOURCE_IDENTITY: Candidate is byte-identical to exact Parent R31B-V011; endpoint measurement only, not a distinct Candidate.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not a V042 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; only Parent-valid widths `4088`, `4096`, and `4104` are included.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- CURRENT_LOCAL_BEST: `NONE`; V042 is not a distinct Candidate and is not promoted.

Compile, correctness, and numeric endpoint/control Local details are in `partial-ranking/V042-PARTIAL-RANKING-RESULT.md`.
