# N02 V010 Handoff

- Candidate source: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V010/submission.asc`
- Candidate source commit: `89b13447234890a5c1b1203885fd7ccbc49f5bc7`
- Source SHA-256: `22cb1a23baa96a4864913f8c8189cd3ab8ae2df0fd361d456c3e65d17185845a`
- Direct parent Candidate: V009 source commit `389db691e67c0941989761e80934168498d831fe`.
- Compile: Parent/Candidate full-link and paired Runner build returned 0 after the paired build used the existing GCC 11 `CPATH`; the first paired build failure is retained.
- Device code: Candidate object 337880 B and `.alink` 187240 B; FP16/BF16 hot symbols grew to 0x19d8/0x16c0 B. Both Candidate artifacts differ from V006 and V009.
- Correctness: existing V016 Runner passed all four cases on d6, rc=0.
- Local: existing V016 Runner completed 4 cases x 21 pairs on d6, rc=0. Large positive outliers and loaded d6 make `LOCAL_SCORE=NONE`.
- Mechanism note: per-unit source transfer/event order remains the same; generated MTE/Vector instruction scheduling could not be decoded from available objdump output. Do not claim isolated I-cache benefit.
- Results and raw evidence: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V010/RESULTS.md` and `results/`.
- Next: test a distinct straight-line layout for the per-unit compute body while retaining the same flat traversal, addresses, transfer order, and event order. Compare device object and hot symbol sizes before deciding whether to run d6.
