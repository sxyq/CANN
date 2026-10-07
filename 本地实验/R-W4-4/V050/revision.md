# R-W4-4 V050

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V050`.
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4992 -> 5120`.
- CANDIDATE_SOURCE_SHA256: `aa2cffe3ce75d6e4d1074e5989a73cde49a59bea76c8fd64ca4d9abcab30c9d0`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not a Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; Candidate fails at 5112/5120 and Parent/Candidate both pass only at 5128.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- LOCAL: `1.004957477180x` at jointly passing `128x5128` control shape (`+0.495747718%`); `NOISY`, `NEEDS_ONE_MORE_LOCAL`.
- CURRENT_LOCAL_BEST: `NONE`; V050 is not promoted.
- OFFICIAL: `NOT_ELIGIBLE`.

Detailed result and raw evidence: `partial-ranking/V050-PARTIAL-RANKING-RESULT.md`.
