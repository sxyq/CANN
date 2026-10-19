# N01 V003 Handoff

V003 is complete and ready for a local evidence commit.

- Source commit: `c72be019d2066fb90c6a803e1640af4c12f15a72`
- Source SHA256: `ee8a2adf42b858921f8dab97cabf734c654bee1b7a480b713df285d043b9af60`
- Compile: PASS
- Correctness: PASS, 4/4
- Device code: differs from Champion; identical to V002 Candidate for all dtypes
- Local: 4 cases x 21 pairs, valid per-case statistics
- Device: d7 / `hwnput3`
- FULL_SOURCE_SIMILARITY: `0.982-0.999` across the ten route comparisons reported by Main; this remains above the 80% target because the Champion body is shared. V003 has a real executable temporary-placement change, with no cosmetic similarity reduction.
- Remote root: `~/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V003/`

NEXT_ACTION: commit the V003 evidence with exact paths, then create the next
REGISTER-LIVE-RANGE revision from V003 using one different temporary FP32
last-use or reuse point. Preserve V002's committed 4/4 Correctness and
`LOCAL_SCORE=NONE` evidence unchanged.
