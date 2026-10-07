# R-W4-4 V043

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V043`.
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4096 -> 4224`.
- CANDIDATE_SOURCE_SHA256: `3d071857a9c345a555e08d10d44fb5c86d36da90decd3753ba717cc1b9259de7`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not conflated with the distinct V043 Candidate failures below.
- PARTIAL_CORRECTNESS: `YES`; Parent/Candidate both pass only at tested width `4232`; Candidate fails at `4216` and `4224` while Parent passes.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- CURRENT_LOCAL_BEST: `NONE`; V043 is not promoted.
- NEXT_PARENT: exact sibling `R31B-V011` unless Planning directs otherwise.

Full failed correctness evidence and the sole eligible shape's numeric Local result are in `partial-ranking/V043-PARTIAL-RANKING-RESULT.md`.
