# R-W4-4 V053

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V053`.
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `5376 -> 5504`.
- CANDIDATE_SOURCE_SHA256: `138f425c48bfb37904a6d6ca3143fb4f2b85b2bad6afe346ba9c4f937bdc6201`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not a Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; Candidate fails at 5496/5504 and Parent/Candidate both pass only at 5512.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- LOCAL: `0.997523767319x` at jointly passing `128x5512` control shape (`-0.247623268%`); `NOISY`, `NEEDS_ONE_MORE_LOCAL`.
- CURRENT_LOCAL_BEST: `NONE`; V053 is not promoted.
- OFFICIAL: `NOT_ELIGIBLE`.

Detailed result and raw evidence: `partial-ranking/V053-PARTIAL-RANKING-RESULT.md`.
