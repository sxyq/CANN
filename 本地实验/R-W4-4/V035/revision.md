# R-W4-4 V035

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`
- REVISION: `V035`
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- SOURCE_SHA256: `f9c24130799116c202f6f4c499b314ba65c320d7e7f5d517a24699875eb104ce`
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4096 -> 512`; no other source delta.
- COMPILE: `PASS`; `device` and `submission` targets.
- PROBE_BUILD: `PASS`; exact Parent and Candidate probes.
- CORRECTNESS: `PASS` for both sides on FP32 `128x504`, `128x512`, `128x520`; all calls `rc=0`, `bad=0`.
- PARTIAL_CORRECTNESS: `YES`; the known exact route Parent C15 failure was not rerun and is not attributed to this Candidate.
- LOCAL_SCORE: `0.979251925323x` partial route-local speedup geomean; `LOCAL_DELTA=-2.074807468%`.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- LOAD_QUALITY: `NOISY`; MEASUREMENT_QUALITY: `NOISY`; NEEDS_ONE_MORE_LOCAL: `YES`.
- SAME_BINARY_QUALIFICATION: `NOT_RECORDED` for these exact shapes; keep the numeric paired result diagnostic and do not promote it.
- CURRENT_LOCAL_BEST: `NONE`.
- NEXT_PARENT_SELECTION: exact sibling `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- EVIDENCE: `本地实验/R-W4-4/V035/partial-ranking/`.
