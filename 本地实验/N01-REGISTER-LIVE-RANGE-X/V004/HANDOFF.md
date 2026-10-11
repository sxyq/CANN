# N01 V004 Handoff

V004 fixed-source testing is complete. Candidate source was not changed after
commit `e833b161dd2ffede6ba8fda34ae0073d67f85ac9`.

- Source SHA256: `9e19b6ab1a9ac71ceee22b92587f8deb879ab32e1b317b3ed6671ddd5cbc8a29`
- Parent: R31B V011 Champion, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Compile: PASS; not repeated
- Correctness: PASS, 4/4
- Local: 4 cases x 21 pairs; `LOCAL_SCORE=NONE` due high paired device-event spread
- Device: d7 / `hwnput3`; no paired Runner remained after testing
- Remote run: `~/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V004/e833b161`
- Full-source similarity reported by Main: `0.982-0.999`, above the 80% target; no cosmetic changes used

NEXT_ACTION: retain this evidence commit. This handoff does not authorize or
start a later revision.
