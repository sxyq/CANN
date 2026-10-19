# N01 V002 Handoff

V002 source and experiment evidence are complete and ready for a results
commit.

- Source commit: `737143f3e5b0780a0cd96ec31ddfc2f5476e50e5`
- Source SHA256: `f26bd3160c34039b535014c7b475ccbe3b6b5ed447d8efc37caee280288b345e`
- Compile: PASS after one recorded `<vector>` include-path failure
- Correctness: PASS, 4/4
- Device code: Parent/Candidate bytes differ; instruction decoding unavailable
- Local: raw samples saved; AIVector load 91%; `LOCAL_SCORE=NONE`
- Device: d7 / `hwnput3`
- Remote root: `~/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V002/`

NEXT_ACTION: commit this V002 evidence with exact paths, then create V003 from
V002 with one different FP16 temporary-value placement: square `valueFp32`
in-place and pass `xFp32` as the `ReduceSum` scratch tensor. Compile the
committed V003 source before running Correctness and paired Local.
