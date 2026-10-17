# N01 V001 Handoff

V001 is complete and locally auditable.

- Source commit: `a3661ee74a72e3bf3890ff7517488b8e8fb746de`
- Evidence commit: this results-only commit
- Compile: PASS
- Correctness: PASS, 4/4
- Codegen: IDENTICAL for FP16/BF16/FP32
- Local: raw paired samples saved; `LOCAL_SCORE=NONE`
- Device: d7 / `hwnput3`
- Remote root: `~/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V001/`

The V001 hypothesis is invalid as a device-code lifetime improvement. The next
single-factor candidate should change a real last-use or reuse point of a
temporary FP32 tensor in `ProcessWideLowPrecision`, while preserving math,
addresses, transfers, synchronization, core allocation, and cache policy.

NEXT_ACTION: commit the selected V001 evidence files, then create V002 with one
temporary FP32 scratch reuse change and run Compile before Correctness and Local.
