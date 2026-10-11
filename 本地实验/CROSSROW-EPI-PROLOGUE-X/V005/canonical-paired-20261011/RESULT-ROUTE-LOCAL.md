# V005 MAIN2-CANONICAL-V1 route-local result

ROUTE: `CROSSROW-EPI-PROLOGUE-X`
REVISION: `V005`
SOURCE_COMMITS: `eb1334db2ee928981515b52f9e33b29c2bd1fcd0` declaration/source, `01c48fa49ef7c8cbe650353bdc8753d95bf38ec5` runtime-contract fix
DIRECT_PARENT: `R31B/V011`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
CANDIDATE_SOURCE_SHA256: `a33c185efc618bd8a588e3ea41031bdb9074e18e89b16966421d7b369d35c2a6`
PARENT_ARTIFACT_SHA256: `e5d05ac104c99450904a4dda4417d6d5ecbd50d04412bc129622b51a56e2cf64`
CANDIDATE_ARTIFACT_SHA256: `8770dba6b057f8ba2d33dc402b1397c927c265e7fd1f74dd3ed9764248f53de0`
RUNNER_REF_SHA256: `2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7`

## Protocol and validity

Final Build: `PASS`, RC=0. Final Correctness: `PASS_7_OF_7`, 14/14 Parent/Candidate invocations RC=0 and bad=0. The initial C13 runtime failure (`ACL 507035`) is retained separately and excluded from final correctness.

Final Local used C12/C13/C14/C16, three `P-C/C-P/P-C` pairs each, `45` warmups, `31` device-event samples per side, blocks=1, gap=0, batch_n=1. All 24 final side runs have 31 samples, RC=0, bad=0. `SAME_BINARY_QUALIFICATION=NOT_RUN`; Parent/Candidate are separate executables built with the same frozen runner include.

## Core summary

| Core | Anchor us | Parent run medians us | Candidate run medians us | Parent median | Candidate median | Candidate faster pairs | Candidate quality | Cell quality |
|---|---:|---|---|---:|---:|---:|---|---|
| C12 | 6.58 | 13.42 / 23.88 / 22.36 | 12.78 / 23.66 / 22.30 | 22.36 | 22.30 | 3/3 | POOR | POOR |
| C13 | 20.80 | 20.58 / 25.10 / 43.28 | 23.72 / 27.56 / 30.78 | 25.10 | 27.56 | 1/3 | POOR | POOR |
| C14 | 25.96 | 34.02 / 25.98 / 123.88 | 25.38 / 25.68 / 26.24 | 34.02 | 25.68 | 3/3 | GOOD | POOR |
| C16 | 15.06 | 33.98 / 26.00 / 15.56 | 17.40 / 21.32 / 14.88 | 26.00 | 17.40 | 3/3 | POOR | POOR |

## Verdict

MAIN2-CANONICAL-V1: `66.438931`
DELTA_VS_FROZEN_R31B_V011: `-33.561069`
PAIRED_PARENT_DIAGNOSTIC_SCORE: `115.952791`
MEASUREMENT_QUALITY: `POOR`
LOCAL_VERDICT: `LOCAL_NEUTRAL`
RELIABLE_LOCAL_GAIN: `NO`
NO_PROMOTION: `YES`
LOCAL_BEST: `UNCHANGED`
STATUS: `COMPLETE_MEASURED / POOR / LOCAL_NEUTRAL / NO_PROMOTION`

The single-factor copy change passes correctness and produces a complete Local score, but the aggregate remains below the frozen anchor and is not stable enough for promotion. NPU7 HBM stayed around 37%; AICore/AIVector/load and unsupported proc-query observations are preserved, and no clean-device claim is made.
