# N02 V003 Handoff

- Candidate source: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V003/submission.asc`
- Source commit: `7132c86d32e8fa1d57e2ae8edb7846e3b4775b2a`
- Source parent: `12a7676ad947dfaf05fabb8e96651b0bbbb59006`
- Support binding: `c414a9ce`
- Source SHA-256: `fa85bdc5b686d9dd49cdc6f0a6e6f02f25753ebd921cb31e12c5253fe829b22d`
- Compile: Parent/Candidate `full_link` PASS; paired runner build PASS; probe not used.
- Correctness: paired `--correctness` PASS for all four public FP16/BF16 cases.
- Local: paired `--paired` returned 0 for four cases and 21 pairs each; variation is high, so `LOCAL_SCORE=NONE`. V002 raw 21-pair blocks were also read: median deltas -0.140, +0.060, -0.300, and -0.840 us, with p10/p90 ranges (-39.580,+0.540), (-0.340,+0.500), (-0.660,+6.780), and (-1.160,-0.600) us.
- Artifact result: V003 Candidate is byte-identical to V001/V002 Candidate (`333912` byte object, `77288` executable section bytes). The `always_inline` attribute caused no new device code structure.
- Similarity: Main reports FULL_SOURCE_SIMILARITY approximately 0.982-0.999 for the visible route set; the route leaves this over-limit fact explicit and does not manipulate source text to lower it.
- Device: d5 / NPU 5 / hwnput3; snapshots and existing load were preserved.
- NEXT_ACTION: stop adding N02 revisions after this negative code-generation result. V001 remains the only route revision with an observed executable-size reduction; V002 and V003 are retained as negative evidence. Main may separately record effective-change similarity.
