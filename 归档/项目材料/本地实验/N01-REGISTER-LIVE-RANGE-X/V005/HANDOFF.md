# N01 V005 Handoff

V005 Compile, Correctness, device-code capture, and paired Local are complete.

- Source commit: `9e74552a`
- Source SHA256: `a1a2e93cfc0165c43fb7da5de4a9855d64dd2b0952e24a4714da75743f2513ba`
- Compile: PASS on CANN `8.5.0.alpha002`
- Correctness: PASS, 4/4 on d5
- Local: 4 cases x 21 pairs; `LOCAL_SCORE=NONE`, `LOAD_QUALITY=NOISY`
- Device code: FP16 Candidate is 16 bytes larger than V004; target disassembly has no instruction words
- Remote run: `~/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V005/9e74552a`

The established Runner prints Candidate as `V001` in Correctness and `V016` in paired Local. Both commands used the V005 Candidate path, source commit and SHA recorded in `RESULTS.md`; the extracted Device ELF and FP16 function identity are also recorded there. No rerun was made for the display labels.

NEXT_ACTION: start V006 with a separate FP16 retained-y lifetime hypothesis. N01 remains on d5 only.
