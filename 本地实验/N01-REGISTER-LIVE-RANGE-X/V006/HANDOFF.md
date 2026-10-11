# N01 V006 Handoff

V006 source, CANN 8.5 build, d5 Correctness, and paired Local are recorded in `RESULTS.md`.

- Source commit: `6cc244b025049552159903387f9f77dc668cdce9`
- Source SHA256: `5116d0e69f1fdc4dcb200a56f1882d259ca81b0a77ec6cbb818c1e13118f3211`
- Compile: PASS, `paired_runner_v016`, CANN `8.5.0.alpha002`
- Correctness: PASS, 4/4 on d5
- Local: 4 x 21 pairs; `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`
- Device: d5 / NPU 5 only
- Raw output and snapshots: `results/`

NEXT_ACTION: start V007 with an independent register-lifetime change. Refresh and save d5 HBM, utilization, and process status before any device run.
