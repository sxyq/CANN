# N02 V002 Handoff

- Candidate source: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V002/submission.asc`
- Source commit: `12a7676ad947dfaf05fabb8e96651b0bbbb59006`
- Source parent: `aac8b47ab640902ef2ed4302650c69c7247aac0d`
- Support binding: `38d5570116cd02fc97ae04692105fa2f38a9b2bd`
- Source SHA-256: `d100688377f62b9307ea4372327336d89f937fbe310bb0b8de39531cb1f6a9c8`
- Compile: Parent/Candidate `full_link` final PASS; paired runner build PASS; first include-binding failure retained; probe not used.
- Correctness: paired `--correctness` PASS for all four public FP16/BF16 cases.
- Local: paired `--paired` returned 0 for four cases and 21 pairs each; timing spikes remain, so `LOCAL_SCORE=NONE`.
- V001 raw review: all four 21-pair blocks were read; outliers and wide p10/p90 ranges do not support a score.
- Device artifact: V002 Candidate is byte-identical to V001 Candidate (`333912` byte object, `77288` executable section bytes). The helper refactor changed source layout but not generated device code.
- Device: d5 / NPU 5 / hwnput3; about 5660 MB free HBM before Local; existing load was preserved.
- Evidence: remote V002 results and required device artifacts are present under this directory.
- NEXT_ACTION: keep V002 as a negative result and choose a new CODE-FOOTPRINT-ICACHE single factor only after this evidence commit; do not reuse the same helper extraction or any noinline experiment.

