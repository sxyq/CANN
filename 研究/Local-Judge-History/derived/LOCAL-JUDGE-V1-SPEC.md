# Local Judge V1 Specification

## Status

- Version: `LOCAL-JUDGE-V1`
- Mode: `CALIBRATED_SURROGATE`
- Exact 15-case reproduction: `NO`; testcase IDs and tbest are known, hidden shape/dtype/input mapping is not.
- Official anchor: `R31B V011`, 15/15, 45.16.
- Direct Online submission: disabled.

## Formula

`s_i = 100 / (1 + log_1.5(time_i / best_time_i)); total = mean(s_i)`

The scorer uses the ordered 15-case vector. It does not average local percentage deltas and it does not mix throughput into Official score.

## Input Contract

A candidate is numerically scorable only when the input contains exactly 15 ordered candidate timings and the matching 15 parent timings from the same suite, device class, binary/timing API, warmup, and sampling policy. Each row must retain raw samples, median, mean, throughput, load snapshot, and source identity.

Required JSON shape for the score command:

```json
{
  "label": "candidate",
  "parent_us": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15],
  "candidate_us": [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15],
  "quality": "QUALIFIED",
  "source_sha": "..."
}
```

The scorer returns per-case parent/candidate latency, local ratio, predicted point score, predicted total Official score, delta versus the parent, and quality. It rejects incomplete or POOR vectors rather than filling missing cases.

## Gate

`ONLINE_ELIGIBLE=YES` requires correctness clean, source identity clean, exact 15-case coverage, qualified same-binary measurements, no outlier-dominated result, consistent latency/throughput direction, and a held-out-calibrated prediction above the current champion by a margin derived from validation error. Since validation error is currently unavailable, the margin and Online eligibility are `UNDEFINED`/`NO`.

## Case Reproducibility

Current classification is `PARTIAL`: the public API confirms the 15 IDs, order, and current best-time snapshot; the public problem description confirms supported dtype and dimension ranges, but exact per-case shapes, dtypes, row counts, and hidden inputs are not exposed.

## ADDR H3 Policy

ADDR V001/H3 is retained as a validation/negative-gate case. Its old `-24.193320%` is an engineering delta with `POOR` quality, not a predicted Official score. V1 returns `UNAVAILABLE` for this candidate because the exact 15-case vector is missing, which blocks Online instead of declaring a numeric win.
