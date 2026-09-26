# LEGACY_TIMING_METHOD

Round2 parent-only probes in this directory were produced by the legacy CPU
wall-clock harness (`timing_probe.asc`: steady_clock around launch+sync,
warmup 5, 21 samples, process-per-rep, no device events). Method tag:
LEGACY_TIMING_METHOD.

- Files tagged: `round2-summary.json`, `baseline-stability.json`,
  `baseline-stability-d5.json`, `baseline-stability-d6.json`,
  `round2-handoff.txt`, `baseline-*.tsv`.
- Not comparable to DEVICE_EVENT numbers from the unified runner.
- Do not merge medians across methods.
- Data retained unchanged; see `../../LEGACY-TIMING-METHOD.md`.
