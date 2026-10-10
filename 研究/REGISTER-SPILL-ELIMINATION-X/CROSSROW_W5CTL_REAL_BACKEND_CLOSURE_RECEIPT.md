# CROSSROW public w5ctl real-backend closure receipt

AGENT_ID=W5-R02-REGISTER-SPILL-ELIMINATION-X
WORKTREE=/home/data4t2/lelinfeng/cann-w5-r02-register
BRANCH=research/w5-r02-register-spill
W5CTL_COMMIT=b0c16aac7ee0ce9455291ce6e75c0ae28a3dd73a
PUBLIC_INPUT_COMMIT=d2412ab5cb2d091679947c65d90d45912541b1a5
PUBLIC_ROUTE=CROSSROW-EPI-PROLOGUE-X
PUBLIC_REVISION=V001
R2_REGISTER_SPILL=FEASIBILITY_UNPROVEN
R2_KERNEL_CANDIDATE=NONE

## Identity

SOURCE_PATH=研究/REGISTER-SPILL-ELIMINATION-X/public-crossrow-v001/Candidate.asc
SOURCE_SHA256=3698c229dfb6a4b336e7ee0506eb1ceaf898c1c37c937c0af7d3eef71fbc685d
ARTIFACT_PATH=研究/REGISTER-SPILL-ELIMINATION-X/public-crossrow-v001/build/crossrow_v001_paired_runner
ARTIFACT_SHA256=70fce83a4c70cfd16de3433b2c6beb72a9c0a5b5b2600cd76da3b39550ac75c9

## Real backend cycle

The cycle used the existing R2-owned real build adapter
`研究/REGISTER-SPILL-ELIMINATION-X/crossrow_public_build_backend.sh` and the
compiled public paired runner as the Correctness/Local backend. No fake
backend, second Runner, scorer, or R1 private worktree was used.

```text
BUILD_BACKEND_RC=0
BUILD_W5CTL_STATUS=PASS
CORRECTNESS_BACKEND_RC=0
CORRECTNESS_RESULT=PASS
CORRECTNESS_BIT_DIFFERENCES=0
CORRECTNESS_MAX_ABS=0
CORRECTNESS_TOLERANCE_FAILURES=0
CORRECTNESS_NONFINITE=0
LOCAL_RUNNER_RC=0
W5CTL_LOCAL_RC=66
```

The cycle command was `w5ctl cycle --stages build,correctness,local,record`
with the public Candidate source, the R2 build adapter, and the existing
`crossrow_v001_paired_runner`; full arguments and subprocess output are in
`crossrow-real-w5ctl-cycle.stdout.log` and the stage logs.

## Local result boundary

The real runner used device 3, rows=16, width=6144, fp16, 45 warmups and 21
PC/CP pairs. The raw TSV was preserved unchanged:

```text
RAW_PATH=研究/REGISTER-SPILL-ELIMINATION-X/crossrow-real-paired-local.tsv
RAW_SHA256=a42a3303aebb50409a627751aa2154530c8fc15c427334b8901107a6d75ebac2
RAW_PAIR_COUNT=21
RAW_SAMPLE_COUNT=42
SHAPE_CANDIDATE_MEDIAN_US=21.340000
SHAPE_PAIRED_DELTA_US=4.000000
SHAPE_PAIRED_PERCENT_DELTA=0.201472
SHAPE_LOCAL_VERDICT=SHAPE_UNCLASSIFIED_NO_PROMOTION
MEASUREMENT_QUALITY=UNCLASSIFIED
NOISE_STATUS=NOISY
FULL_LOCAL_SCORE=NOT_COMPUTABLE
PROMOTION_ELIGIBLE=NO
```

This is one shape-level timing result only; it is not a full Local score and
does not advance any Local Best.

## Record, checkpoint, report, resume

```text
CHECKPOINT_PATH=研究/REGISTER-SPILL-ELIMINATION-X/crossrow-real-checkpoint.json
MANIFEST_PATH=研究/REGISTER-SPILL-ELIMINATION-X/crossrow-real-record.json
RECORD_BACKEND=NOT_CONFIGURED
RECORD_STATUS=LOCAL_MANIFEST_WRITTEN
RECORD_INTEGRATION_STATUS=INTEGRATION_INCOMPLETE
REPORT_RC=0
REPORT_STATUS=INCOMPLETE
RESUME1_RC=78
RESUME1_IDEMPOTENT_NOOP=YES
RESUME2_RC=78
RESUME2_IDEMPOTENT_NOOP=YES
REVISION_CREATED=NO
RUNNER_CREATED=NO
ONLINE=NOT_SUBMITTED
W5CTL_E2E=INCOMPLETE
```

The manifest contains source/artifact/raw SHA-256 values, evidence path/SHA,
the complete shape-level Local result, and explicit no-Revision/no-Runner
fields. The checkpoint retains PASS, UNCLASSIFIED and
INTEGRATION_INCOMPLETE stage states. Repeated resume skipped every terminal
stage and did not invoke Build, Correctness, Local, or Record again.

## Resource context and retained evidence

Pre/post device snapshots recorded HBM usage at 96% on device 3, with no
confirmed active device-3 lease in the last-known shared lease file. That file
had mtime `2026-09-28 19:39:58 UTC`; therefore lease authority is
`UNKNOWN_STALE_LOG`, not live authorization. No process was killed or changed.

Evidence files for this closure are the `crossrow-real-w5ctl-*` logs, raw TSV,
checkpoint, and manifest in this route directory. The ignored build directory
and artifact are not staged in the evidence commit.

NEXT_ACTION=Tooling/Record Owner must supply and accept the shared Record backend and a full published Local suite before any E2E/score claim.
