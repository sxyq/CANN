# V004 MAIN2-CANONICAL-V1 route-local result

ROUTE: `CROSSROW-EPI-PROLOGUE-X`
REVISION: `V004`
SOURCE_CHECKPOINT_COMMIT: `b2fc0653364788e47082b4ac676eb03cf498bd19`
DIRECT_PARENT: `R31B/V011`
PARENT_SOURCE_SHA256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
CANDIDATE_SOURCE_SHA256: `642b2ae9482ea14e6cde8ed04b457cffc0f1ae2c2d52b242128e85fe36cfd994`
PARENT_ARTIFACT_SHA256: `e5d05ac104c99450904a4dda4417d6d5ecbd50d04412bc129622b51a56e2cf64`
CANDIDATE_ARTIFACT_SHA256: `5d38a83fbab2475226e2a1b0d073bdcba0d0245c12a0332c69854885ecbf95f5`
RUNNER_REF_SHA256: `2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7`
SCORER_SHA256: `f43709695115da53e30e58204f8e7c4716486538397394d7e2cb441f3a1d6d7f`
SUITE_SHA256: `30b6edcd59f19f3e495bb23cb5cf2f41bf5e132a6d96f7b050e32fb6983b0a95`

## Protocol

The published canonical harness was reused without a new Runner or scoreboard. Core cases were C12/C13/C14/C16. Each case has three paired runs in `P-C/C-P/P-C`; every side used `45` warmups, `31` device-event samples, `blocks=1`, `gap=0`, `batch_n=1`, and returned `RC=0` with `bad=0`. All `24` raw files contain `31/31` ordered samples. `SAME_BINARY_QUALIFICATION=NOT_RUN`; Parent and Candidate are separate probe executables built with the same frozen `runner_ref.inc`.

Exact side command:

```text
<clx_ref_parent_probe|clx_ref_candidate_probe> 7 <rows> <width> <dtype> <raw-prefix> 45 31 1 0 1
```

Raw, stats, RC, logs, and per-side pre/post snapshots are in this directory. `SHA256SUMS` covers the route-local evidence.

## Core medians and score

The frozen pure scorer formula is:

```text
MAIN2-CANONICAL-V1 = 100 * exp(mean(log(anchor_median_us / candidate_median_us)))
```

| Core | Anchor median us | Parent run medians us | Candidate run medians us | Parent median us | Candidate median us | Candidate faster pairs |
|---|---:|---|---|---:|---:|---:|
| C12 | 6.58 | 14.50 / 21.38 / 22.56 | 13.38 / 31.92 / 35.46 | 21.38 | 31.92 | 1/3 |
| C13 | 20.80 | 55.72 / 31.34 / 20.92 | 20.38 / 20.40 / 22.74 | 31.34 | 20.40 | 2/3 |
| C14 | 25.96 | 69.64 / 25.86 / 66.34 | 44.04 / 26.70 / 38.34 | 66.34 | 38.34 | 2/3 |
| C16 | 15.06 | 19.06 / 15.18 / 17.50 | 20.04 / 15.04 / 21.16 | 17.50 | 20.04 | 1/3 |

MAIN2-CANONICAL-V1: `57.186564`
DELTA_VS_FROZEN_R31B_V011: `-42.813436`
PAIRED_PARENT_DIAGNOSTIC_SCORE: `111.665582`

## Quality and verdict

CASE_QUALITY: C12=`POOR`, C13=`FAIR`, C14=`POOR`, C16=`POOR`
MEASUREMENT_QUALITY: `POOR`
LOCAL_VERDICT: `LOCAL_NEUTRAL`
RELIABLE_LOCAL_GAIN: `NO`
NO_PROMOTION: `YES`
LOCAL_BEST: `UNCHANGED`
FULL_LOCAL_SCORE: `COMPUTABLE_4_CORE`

NPU: `7`; pre/post HBM usage was `37%` (well above the 100 MB rule). The captured snapshots report AICore/AIVector variation and the device proc query is unsupported; external load metadata is retained as context. The measurement is not represented as a clean/quiet claim. No raw sample was deleted or filtered.

The candidate improves the paired median in C13 and C14 but loses in C12 and C16; run-to-run dispersion makes the aggregate non-promotable. This V004 result is a real complete canonical four-Core measurement, but it is `LOCAL_NEUTRAL`, not `LOCAL_BEST` or Official.

## Closeout evidence

MEASUREMENT_INDEX: `MEASUREMENT-INDEX.tsv`
CORE_RAW_SUMMARY: `CORE-RAW-SUMMARY.tsv`
SCORER_RECEIPT: `CANONICAL-SCORER-CALL.md`
RAW_INTEGRITY: `24/24` side runs have `31/31` device-event rows, `RC=0`, and `bad=0`; no sample was removed.
STATUS: `COMPLETE_MEASURED / POOR / LOCAL_NEUTRAL / NO_PROMOTION`
