# MAIN-2 Champion Local Baseline Calibration — 2026-10-04

## Anchor and identity

```text
OFFICIAL_CHAMPION=R31B V011
OFFICIAL_SCORE=45.16
OFFICIAL_RESULT=PASS 15/15
OFFICIAL_SCORE_FORMULA=mean of the 15 official per-case scores
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
LOCAL_SUBMISSION_SHA256=MATCH
LOCAL_SIDECAR_SHA256=MATCH
```

The exact local `线上结果/R31B/V011/submission.asc` and `submission.sha256`
were re-hashed and match. Official testcase latencies and the Official formula
were not converted into local latency or throughput.

## Canonical-local-suite check

```text
CANONICAL_LOCAL_SHAPE_SUITE=NOT_FOUND
CANONICAL_LOCAL_COMPOSITE_SCORE=NOT_FOUND
CROSS_ROUTE_SCORE_NOT_DIRECTLY_COMPARABLE=YES
```

The project defines the local measurement protocol, but not one canonical
cross-route local shape suite or composite score. Therefore each baseline row
is scoped to its exact route target shape, dtype, runner and device. The 45.16
Official Champion value is an anchor only, not a local score.

## Baseline construction

`R31B-V011-LOCAL-BASELINE.tsv` extracts the exact R31B V011 Parent side from
each already-recorded three-run interleaved Parent/Candidate measurement. The
parent SHA in every revision record is the exact Official V011 SHA above, and
the corresponding score audit identifies the same target shape, dtype,
runner and device. Reusing these matched Parent observations keeps the
baseline directly comparable to each existing Candidate record; it avoids
mixing a new isolated load window with historical Candidate runs.

```text
PARENT_RUN_COUNT=3 per scored context
FRESH_NPU_TIMING_RUNS_THIS_TURN=0
BASELINE_LATENCY=arithmetic mean of the three recorded per-run Parent means
THROUGHPUT_INVOCATIONS_PER_SECOND=1,000,000 / PARENT_AVG_US
THROUGHPUT_DELTA_PERCENT=(PARENT_AVG_US / CANDIDATE_AVG_US - 1) * 100
```

Raw samples and pre/post `npu-smi` snapshots remain at the evidence paths in
the TSV. The original measurements show poor stability/load quality; this
calibration preserves their runs rather than removing tails or inventing a
cleaner score. In particular, ADDR V001 and UB V001 each have a single very
large faster run amid slower runs, which dominates their average. These values
are engineering comparisons, not Official scores.

## Historical resource/lease completeness

| Context | HBM/AICore/process snapshot | AIVector | Host load | Formal shared lease |
|---|---|---|---|---|
| ADDR V001 | pre/post snapshots retained | not recorded | not recorded | no matching ledger row found |
| ADDR V002 | pre/post snapshots retained; VLLM/Python noted | not recorded | not recorded | no matching ledger row found |
| UB V001 | pre/post snapshots retained | not recorded | not recorded | no matching ledger row found |
| BRANCH V002 | pre/post snapshots retained; Python noted | not recorded | not recorded | no matching ledger row found |
| REDUCE V001 | pre/post snapshots retained; VLLM noted | not recorded | not recorded | `M2-REDUCE-V001-D5-20261003T182337Z`, released |

Consequently, all five historical score contexts remain `QUALITY=POOR`; the
four missing lease entries and missing host/AIVector telemetry are preserved
as evidence gaps, not inferred from the current device snapshot. This task
started no fresh NPU timing, so it created no new device lease or load record.

Branch V001 is included in the normalized score audit as a historical
`LOCAL_REJECTED` revision but has no three-run scalar Local Score. Its two
formal windows had mixed paired directions; the normalized table leaves its
average/delta fields `NA` rather than manufacturing one.

## Correctness and Online implication

- R31B V011's Official result is 15/15 PASS.
- ADDR V001's selected BF16-D32768 shape passed Parent/Candidate correctness,
  but its required BF16-D40960 path remains a shared Parent/Candidate
  `507035` runtime blocker. Its three timing runs show Candidate faster only
  1/3 times, so it fails the requested 2/3 direction gate despite a positive
  average reciprocal-latency ratio. `ONLINE_ELIGIBLE=NO`.
- ADDR V002, BRANCH V002 and REDUCE V001 do not show a stable positive signal
  beyond their POOR quality/noise evidence. UB V001's full correctness domain
  remains unclean. None is Online eligible.
- `DIRECT_ONLINE_SUBMISSION=0`.

No Kernel, Revision, shared canonical score ledger, or NPU task was changed or
started by this calibration. The existing local score calculations were
rechecked against their same-run exact V011 Parent observations.
