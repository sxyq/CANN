# N02 V012 Handoff

- Candidate source: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V012/submission.asc`
- Candidate source commit: `3fba7ee209e56b68f4e813ac0817120b4515e366`
- Source SHA-256: `6cdcd84a197061d3d0f81b962309628860cda9fa2892892bdf544d5972c1f27c`
- Direct parent Candidate: V011 source commit `d8b1331eaf91697cc5bd928ff9ac55a9906b88e1`.
- Compile: Parent and Candidate full-link returned 0. The existing V016 Runner build returned 0 after loading the configured CANN environment; the first environment-missing failure remains recorded.
- Device code: Candidate object is 343024 B and `.alink` is 191704 B. Parent object is 334040 B and `.alink` is 183144 B. Both hot low-precision function symbols grew; object and linked artifact bytes differ.
- Correctness: existing V016 Runner passed all four Parent V011 / Candidate V012 cases on d6, rc=0. Initial loader failure was before case execution and is retained.
- Local: existing V016 Runner completed 4 cases x 21 pairs on d6, rc=0. Outliers and sustained HBM occupancy make `LOCAL_SCORE=NONE`.
- Mechanism note: terminal-tile peeling changes control-flow layout and may affect prefetch/compute timing. DataCopy addresses and event kinds remain unchanged. Device instruction words are unavailable in the saved disassembly.
- Results and raw evidence: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V012/RESULTS.md` and `results/`.
- Next hypothesis: manually unroll exactly two adjacent steady-state tiles while keeping each tile's wait, successor load, conversion, apply, and release ordering intact; compile first and compare device objects and hot symbols. This is distinct from the V008 unroll pragma, whose device object matched V006. Treat any changed DMA/compute overlap as part of the result.
