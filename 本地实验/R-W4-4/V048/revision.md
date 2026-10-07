# R-W4-4 V048

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V048`.
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4736 -> 4864`.
- CANDIDATE_SOURCE_SHA256: `bd900346df18fd4956a2b6ea2e14ae8bc97950d8b3dff99315bff30cd983dc5f`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not a Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; Candidate fails at 4856/4864 and Parent/Candidate both pass only at 4872.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- LOCAL: `0.969940583691x` at jointly passing `128x4872` control shape (`-3.005941631%`); `NOISY`, `NEEDS_ONE_MORE_LOCAL`.
- CURRENT_LOCAL_BEST: `NONE`; V048 is not promoted.
- OFFICIAL: `NOT_ELIGIBLE`.

Detailed result and raw evidence: `partial-ranking/V048-PARTIAL-RANKING-RESULT.md`.
