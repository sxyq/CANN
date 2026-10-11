# V003 candidate-only canonical diagnostics

- `CLASSIFICATION=UNPAIRED`
- `SCORE_STATUS=EXCLUDED_FROM_SCORE`
- `CANONICAL_SCORE=NONE`
- `VALID_PAIRED_RUNS=0`

This directory contains 12 Candidate-only raw runs: C12, C13, C14 and C16, three runs per case. The saved runner logs report 45 warmups, 31 device-event samples and `bad=0`; each run has its own raw TSV, stats, RC, and pre/post device snapshots. No Parent observations are present in this directory, so these files do not form paired runs and are not inputs to the V003 MAIN2-CANONICAL-V1 score.

The raw and log files are retained unchanged as diagnostic evidence. The scored V003 Parent/Candidate measurements and result are separate under `../canonical-paired-20261011/`.
