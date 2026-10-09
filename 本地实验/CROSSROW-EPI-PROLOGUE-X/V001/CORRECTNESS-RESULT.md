# Correctness Result

ROUTE: CROSSROW-EPI-PROLOGUE-X
REVISION: V001
STAGE: CORRECTNESS
STATUS: PASS
PARENT: W3 CROSSROW-FULL-PIPELINE V012 source in `Parent.asc`
CANDIDATE: W5-R01 V001 source in `Candidate.asc`
RUNNER: existing V012 paired runner, `crossrow_v001_paired_runner`
SHAPE: rows=16 width=6144 blocks=8 dtype=fp16
PATH: generic cached-row `Process()` path; two rows per effective block
DEVICE: 0
FREE_HBM_MB_PRE: 62259

RESULT: `bit_differences=0 max_abs=0 tolerance_failures=0 nonfinite=0`

The raw runner output is retained in `correctness-v001-r16-d6144.log`; the command output file is `correctness-v001-r16-d6144.tsv`. Device/load context is retained separately in `correctness-load-pre.txt`.
