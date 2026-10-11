# N02 V009 Handoff

- Candidate source: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V009/submission.asc`
- Candidate source commit: `389db691e67c0941989761e80934168498d831fe`
- Source SHA-256: `6c46e66c94007f606f3abf3ca5a76744ec6b1865a81081297e1d9b3f862a1e00`
- Direct prior Candidate: V008 source commit `d5739dc618126766d96ab83356858ef2fc82438f`.
- Compile: Parent/Candidate full-link and existing V016 paired Runner build returned 0 with CANN 8.5.0.alpha002.
- Device code: Candidate `.o` is 334152 B versus Parent 334040 B. FP16/BF16 hot symbols grew by 0x78/0x44 B. Candidate `.alink` has the same length as Parent and different contents. Both Candidate artifacts differ from V006.
- Correctness: first launch exited 1 before cases because of the selected HCC C++ runtime. Attempt 2 used the system GCC 11 library and passed 4/4 on d6 with rc=0.
- Local: existing V016 paired Runner completed 4 cases x 21 pairs on d6 with rc=0. Large outliers, mixed paired deltas, and d6 VLLM occupancy make the result `LOCAL_SCORE=NONE`.
- Mechanism note: the source keeps flattened indexing and per-unit DataCopy/event order; the compiler's emitted MTE schedule could not be decoded from the available objdump output. Do not claim an isolated I-cache effect.
- Results and raw evidence: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V009/RESULTS.md` and `results/`.
- Next hypothesis: split pass-1 traversal into an initial load, a steady loop, and a final unit while preserving each DataCopy/prefetch point and the existing address expressions; compare emitted hot symbols before running d6.
