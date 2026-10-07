# Canonical Parent C15 reproducibility

- ROUTE / REVISION: `R-W4-4 / MODE-DISPATCH-CUTOFF-X / V028`
- CHECK: correctness-only, device 2, FP32 (`dtype=0`), shape `1x32768`, one invocation
- START / END UTC: `2026-10-07T03:22:22.269496657Z` / `2026-10-07T03:22:33.286626449Z`
- CANONICAL PARENT SHA256: `f0ab43545e13d943e4c5bb5ff22400426e18f560ab313c1c6856b39fa904f033`
- PROBE SHA256: `ddb9e0ba4f628380c9be9028f9d7c9cdc5dc89aadf6a66e14d50712054a26319`
- RUNNER SHA256: `2aa1aa3b2801a9202de01946ff079ec3b580c6ee7218dc03a772a87a4a4467e7`
- PARENT VERDICT: `FAIL`, return code `3`, `bad=28659`, `max_abs=1.2031`
- RAW SAMPLE: `device_us=128699`, `host_wall_us=129139`; one sample only, so jitter/repeatability is not estimable. These are incidental correctness-run timings, not a performance result.
- PRE/POST DEVICE CONTEXT: HBM usage `91%`; AICore `33%` / `15%`; HBM bandwidth `30%` / `30%`; Aivector `20%` / `17%`. `VLLMWorker_TP` PID `92813`, memory `56703 MB`, was present before and after. No other process was modified.
- SCORE TYPE: `CORRECTNESS_ONLY`; `LOCAL_SCORE=NONE`; throughput `NOT_MEASURED`.
- OFFICIAL COMPARABILITY: `NONE`; the route-bound deterministic harness inputs are not official Judge inputs.
- INITIAL ENVIRONMENT ATTEMPT: exit `127` before probe launch because `set_env.sh` saw unset `LD_LIBRARY_PATH` under shell `nounset`; this is not a correctness result. Retry evidence is in `retry1/`.

This reproduces the exact canonical Parent failure. The repaired diagnostic copy is not used as a baseline. Local measurement and performance edits remain gated on exact-Parent correctness.
