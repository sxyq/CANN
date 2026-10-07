# R-W4-4 V049

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V049`.
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4864 -> 4992`.
- CANDIDATE_SOURCE_SHA256: `574ef10c053f111ead3104cf1105bc6ae0cf7e8e5e050928a7f200c9e17fee23`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; not rerun and not a Candidate regression.
- PARTIAL_CORRECTNESS: `YES`; Candidate fails at 4984/4992 and Parent/Candidate both pass only at 5000.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- LOCAL: `0.996829813022x` at jointly passing `128x5000` control shape (`-0.317018698%`); `NOISY`, `NEEDS_ONE_MORE_LOCAL`.
- CURRENT_LOCAL_BEST: `NONE`; V049 is not promoted.
- OFFICIAL: `NOT_ELIGIBLE`.

Detailed result and raw evidence: `partial-ranking/V049-PARTIAL-RANKING-RESULT.md`.
