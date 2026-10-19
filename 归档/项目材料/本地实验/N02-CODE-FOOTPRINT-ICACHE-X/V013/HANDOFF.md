# N02 V013 Handoff

- Candidate source: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V013/submission.asc`
- Candidate source commit: `3b0593944`
- Source SHA-256: `e596da15bb9f093bf9003a06753daafede6d50c77fa5376a16542c6c6f23d000`
- Direct parent Candidate: V012 source commit `3fba7ee209e56b68f4e813ac0817120b4515e366`.
- Compile: Candidate full-link and existing V016 paired Runner build returned 0. The Runner display-label rebuild also returned 0. Parent R31B V011 device artifacts were reused from V012; no duplicate Parent build was run.
- Device code: Candidate object is 351416 B and `.alink` is 199896 B. Reused Parent object is 334040 B and `.alink` is 183144 B. Hot low-precision symbols and artifact bytes differ.
- Correctness: 4/4 PASS on d6, rc=0. The retained log prints the old `N02-V012` label. The successful 00:31 UTC Runner compile input, ASC include, dependency record, matching V013 source SHA, and 00:34 UTC pre-run snapshot identify the Candidate as V013. The display-only source edit occurred at 00:39 UTC and changed no kernel code; the log remains unmodified and Correctness was not rerun.
- Local: existing V016 Runner completed 4 cases x 21 pairs on d6, rc=0. Outliers and sustained HBM occupancy make `LOCAL_SCORE=NONE`.
- Mechanism note: explicit two-tile body increases hot code size while preserving per-tile wait/load/apply/release order. Do not report it as an isolated instruction-cache effect.
- Results and raw evidence: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V013/RESULTS.md` and `results/`.
- Next hypothesis: try a one-tile prologue followed by a uniform interior loop, keeping the same per-tile transfer and event order; inspect the emitted object first. This is a distinct loop-layout test and does not repeat inline/noinline or the V008 pragma.
