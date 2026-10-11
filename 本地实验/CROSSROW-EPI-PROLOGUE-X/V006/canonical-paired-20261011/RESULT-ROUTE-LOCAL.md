# V006 MAIN2-CANONICAL-V1 route-local result

ROUTE: `CROSSROW-EPI-PROLOGUE-X`
REVISION: `V006`
SOURCE_COMMIT: `8b1e6ae0a9dd0ce0136979db6bf5eb3cc4e2765a`
DIRECT_PARENT: `R31B/V011` reliable baseline; V005 was not used as Parent or Best
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
CANDIDATE_SOURCE_SHA256: `d9297e337f4aee09994ff6d7f41ad3d55ff172a1382ee042b44f791dea114c1c`
PARENT_ARTIFACT_SHA256: `e5d05ac104c99450904a4dda4417d6d5ecbd50d04412bc129622b51a56e2cf64`
CANDIDATE_ARTIFACT_SHA256: `59f9c8ab75fb1b2d91c9dc2d1e6cf4c4461b1481acb931c29a6314c4a4b7662a`
RUNNER_REF_SHA256: `2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7`

## Protocol and validity

Build: `PASS`, RC=0 after the environment-only include-path repair recorded in `BUILD-CORRECTNESS-RESULT.md`. Correctness: `PASS_7_OF_7`, 14/14 Parent/Candidate calls RC=0, `bad=0`.

Local used C12/C13/C14/C16, three `P-C/C-P/P-C` pairs each, 45 warmups, 31 device-event samples per side, blocks=1, gap=0, batch_n=1. All 24 final side runs have 31 samples, RC=0, bad=0. `SAME_BINARY_QUALIFICATION=NOT_RUN`; Parent/Candidate are separate executables built with the same frozen runner include.

## Core summary

| Core | Anchor us | Parent run medians us | Candidate run medians us | Parent median | Candidate median | Candidate faster pairs | Candidate quality | Cell quality |
|---|---:|---|---|---:|---:|---:|---|---|
| C12 | 6.58 | 26.28 / 16.40 / 12.80 | 12.28 / 22.06 / 36.08 | 16.40 | 22.06 | 1/3 | POOR | POOR |
| C13 | 20.80 | 22.86 / 49.26 / 22.54 | 21.24 / 56.38 / 21.06 | 22.86 | 21.24 | 2/3 | POOR | POOR |
| C14 | 25.96 | 38.94 / 30.10 / 36.76 | 24.64 / 24.72 / 24.78 | 36.76 | 24.72 | 3/3 | GOOD | POOR |
| C16 | 15.06 | 20.58 / 19.64 / 26.34 | 42.48 / 23.84 / 14.94 | 20.58 | 23.84 | 1/3 | POOR | POOR |

## Verdict

MAIN2-CANONICAL-V1: `66.347725`
DELTA_VS_FROZEN_R31B_V011: `-33.652275`
PAIRED_PARENT_DIAGNOSTIC_SCORE: `100.671503`
MEASUREMENT_QUALITY: `POOR`
LOCAL_VERDICT: `LOCAL_NEUTRAL`
RELIABLE_LOCAL_GAIN: `NO`
NO_PROMOTION: `YES`
LOCAL_BEST: `UNCHANGED`
STATUS: `COMPLETE_MEASURED / POOR / LOCAL_NEUTRAL / NO_PROMOTION`

## Load and contamination context

Target device: NPU7. Pre-measurement HBM usage was 37% (approximately 41 GB free; `npu-smi` reports integer usage, so exact free HBM is not claimed) and AICore was 14%. Post-measurement HBM remained 37% and AICore was 0%. `npu-smi info proc -i 7` reported that process querying is unsupported. Therefore process cleanliness is `UNKNOWN`; no clean-device claim is made. These observations did not block Local because the HBM gate was satisfied. All raw samples and pre/post snapshots are retained.
