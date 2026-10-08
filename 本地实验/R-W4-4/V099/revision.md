# MODE-DISPATCH-CUTOFF-X V099

- `DIRECT_PARENT=exact R31B-V011`; Parent SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 996`; Candidate SHA256 `e5904e89a91b4dfe37b54f413585d9bae79cc5322425e98ed6fa2f1b07f4ba3f`.
- `FOCUS_AXIS=FP32_SMALL_BATCH_CUTOFF_OFAT`; selected FP32 cases `128x992`, `128x996`, `128x1000`.
- `COMPILE=PASS`; Parent and Candidate Correctness PASS on all three shapes.
- `LOCAL_SCORE=0.974082257607x`; `LOCAL_DELTA=-2.591774239%`; `LOCAL_VERDICT=LOCAL_REJECTED`; measurement is noisy.
- `CURRENT_LOCAL_BEST=exact R31B-V011`; V099 is not promoted. `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.
- Exact-Parent C15 FP32 `1x32768` remains excluded for the known Parent correctness failure; it was not rerun.
- Numeric and raw evidence: `partial-ranking/V099-PARTIAL-RANKING-RESULT.md` and retained TSV/log/snapshot files.
