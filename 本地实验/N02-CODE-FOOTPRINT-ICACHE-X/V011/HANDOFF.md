# N02 V011 Handoff

- Candidate source: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V011/submission.asc`
- Candidate source commit: `d8b1331eaf91697cc5bd928ff9ac55a9906b88e1`
- Source SHA-256: `957083ebcaf098d6e9ffcd4531714c90d38a6d1e6bfb244f8eec4851eaced9d7`
- Direct parent Candidate: V010 source commit `89b13447234890a5c1b1203885fd7ccbc49f5bc7`.
- Compile: Parent/Candidate full-link and existing paired Runner build returned 0 under CANN 8.5.0.alpha002.
- Device code: Candidate object 339432 B and `.alink` 187608 B. New FP16/BF16 helper symbols are 0x160/0x138 B; the parent hot functions are 0x1234/0x1248 B. Candidate object and `.alink` differ from V006, V009, and V010.
- Correctness: existing V016 Runner passed all four cases on d6, rc=0.
- Local: existing V016 Runner completed 4 cases x 21 pairs on d6, rc=0. Large outliers and loaded d6 make `LOCAL_SCORE=NONE`.
- Mechanism note: source transfer/event order is retained, but the generated helper adds a call path. Device instruction words are unavailable from the current objdump output; do not claim isolated I-cache benefit.
- Results and raw evidence: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V011/RESULTS.md` and `results/`.
- Next hypothesis: reorganize the pass-2 tile prefetch/apply loop into a distinct control-flow shape while retaining parameter values, load conditions, addresses, and event sequence; check emitted object and hot symbols first.
