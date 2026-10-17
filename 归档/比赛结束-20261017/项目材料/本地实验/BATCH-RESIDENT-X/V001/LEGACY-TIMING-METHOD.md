# LEGACY_TIMING_METHOD

All prior BATCH-RESIDENT-X V001 timing numbers were produced by the legacy CPU
wall-clock harness (`phase4/workspaces/BATCH-RESIDENT-X/timing_probe.asc`:
steady_clock around launch+sync, warmup 5, 21 samples, process-per-rep, no
per-sample dump, no device events). Method tag: LEGACY_TIMING_METHOD.

Tagged summary files in this revision (data retained, nothing deleted):

- `local-result.json` (paired parent/candidate wall-clock deltas and jitter)
- `support/results-window-qual/` (gates-summary.json, gate-d4.json, gate-d6.json,
  window-qual-handoff.txt, q-*.tsv)
- `support/results-round2/` (round2-summary.json, baseline-stability*.json,
  round2-handoff.txt, baseline-*.tsv)

Rules:

- Not comparable to DEVICE_EVENT numbers produced by the new unified runner
  (`workspaces/BATCH-RESIDENT-X/support/`).
- Do not merge medians across methods.
- Legacy BATCH decisions (WINDOW_UNQUALIFIED 2/2, NEEDS_ONE_MORE_LOCAL,
  MEASUREMENT_BLOCKED) remain as recorded; none of these numbers is
  decision-grade under the unified protocol.
