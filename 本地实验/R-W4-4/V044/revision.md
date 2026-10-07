# R-W4-4 V044

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V044`.
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4224 -> 4352`.
- CANDIDATE_SOURCE_SHA256: `370cce9ee13c13ac793de74ca936ddfa177a259383c12e7a395c34fdfe7251a1`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not conflated with distinct V044 Candidate failures.
- PARTIAL_CORRECTNESS: `YES`; Parent/Candidate both pass only at tested width `4360`; Candidate fails at `4344` and `4352` while Parent passes.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- CURRENT_LOCAL_BEST: `NONE`; V044 is not promoted.
- NEXT_PARENT: exact sibling `R31B-V011` unless Planning directs otherwise.

Full correctness and eligible Local evidence are in `partial-ranking/V044-PARTIAL-RANKING-RESULT.md`.
