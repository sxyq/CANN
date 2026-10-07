# R-W4-4 V051

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V051`.
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `5120 -> 5248`.
- CANDIDATE_SOURCE_SHA256: `794e8ae72f1d914e144043eb5e359ad605f804ac35a6362dbcd80000d36afe01`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not a Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; Candidate fails at 5240/5248 and Parent/Candidate both pass only at 5256.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- LOCAL: `1.037518210295x` at jointly passing `128x5256` control shape (`+3.751821030%`); `NOISY`, `NEEDS_ONE_MORE_LOCAL`.
- CURRENT_LOCAL_BEST: `NONE`; V051 is not promoted.
- OFFICIAL: `NOT_ELIGIBLE`.

Detailed result and raw evidence: `partial-ranking/V051-PARTIAL-RANKING-RESULT.md`.
