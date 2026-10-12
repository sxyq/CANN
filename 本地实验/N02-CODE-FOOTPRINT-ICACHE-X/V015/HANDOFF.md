# N02 V015 Handoff

- Candidate source commit: `e89597723a19c60d9b05e6889e3adc96b33f1987`.
- Candidate source SHA-256: `531751ef88ca63ec4adb4da24b2089b6b429a4b08cb4ee1da88f826479c468a4`.
- Parent: R31B V011, source SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Change: predicted-cold single-tile branch jumps to the existing terminal block; the multi-tile pass-2 path falls through the prologue and interior loop. Per-tile operation and event order remain unchanged.
- Candidate configure/full-link: RC=0. Existing V016 paired Runner configure/build: RC=0.
- Runner SHA-256: `3c7e855cb5127ee3c28dd60aa3e3a5d4cfdd9f7895809d596cdf3424684c1fc6`; dependency file binds V011 Parent and V015 Candidate.
- Correctness: Parent and Candidate passed all 4 local cases, RC=0.
- Same-binary: four 62-sample cases passed robust classification, RC=0; raw spikes remain up to 210.720 us.
- Paired Local: four cases x 21 alternating pairs, RC=0. Concurrent VLLM load and large event-time tails make results non-attributable; `LOCAL_SCORE=NONE`, `LOCAL_DELTA=NONE`.
- Candidate device object: 346,024 B; Candidate `.alink`: 191,704 B. Device objects differ from V014; Parent object byte-compares equal to the retained V014 Parent.
- No usable device disassembly or ICache counter was collected. Do not claim ICache improvement.
- d6 snapshots and complete raw outputs are under this revision's `results/` directory. `/home/data4t2` stayed read-only; temporary build/run files used `/tmp` and were removed after retrieval.
- No Judge submission. Official testcase-to-shape mapping remains unknown.

Full details: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V015/RESULTS.md`.
