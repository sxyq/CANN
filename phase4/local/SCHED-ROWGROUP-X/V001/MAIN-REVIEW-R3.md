# Main Review r3 — SCHED-ROWGROUP-X V001

Received: 2026-09-23T21:59:59Z

- ACCEPT PROBE PARK
- CORRECTNESS=PASS
- source frozen 0fae0a42e3942356fe3477cd518b17180b6104d57350cee705cba37a5895e65c
- set1/set2 preserved
- DECISION: NEEDS_ONE_MORE_LOCAL + PROBES PARKED
- Stop VLLM polling. No V002. No online. No CANNJudge.
- Use the current live-HBM and shared-lease rule before exact-shape same-binary and paired probes; no AICore-zero or VLLM-free prerequisite.
