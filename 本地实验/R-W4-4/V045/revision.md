# R-W4-4 V045

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V045`.
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4352 -> 4480`.
- CANDIDATE_SOURCE_SHA256: `01ce40919026ab8a312fcf6e2a5aa6a4d1d44b3dfcace36a4fe00afa6aaa14f1`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not conflated with distinct V045 Candidate failures.
- PARTIAL_CORRECTNESS: `YES`; Parent/Candidate both pass only at `4488`; Candidate fails at `4472` and `4480` while Parent passes.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- CURRENT_LOCAL_BEST: `NONE`; V045 is not promoted.
- NEXT_PARENT: exact sibling `R31B-V011` unless Planning directs otherwise.

Full correctness and eligible Local evidence are in `partial-ranking/V045-PARTIAL-RANKING-RESULT.md`.
