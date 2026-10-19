# N02 V001 Handoff

- Source candidate: `本地实验/N02-CODE-FOOTPRINT-ICACHE-X/V001/submission.asc`
- Source commit: `aac8b47ab640902ef2ed4302650c69c7247aac0d`
- Parent: `1c51a5b2eb9b5b9dffc031c1fbbbc35e9506b49f`
- Source SHA-256: `d9297e337f4aee09994ff6d7f41ad3d55ff172a1382ee042b44f791dea114c1c`
- Compile: Parent/Candidate device `full_link` PASS; paired runner final build PASS; standalone probe link failure retained as a separate failed target.
- Correctness: paired `--correctness` PASS, four public FP16/BF16 cases, zero failures.
- Local: paired `--paired` returned 0, but timing variation is too high; `LOCAL_SCORE=NONE`.
- Device: d5 / NPU 5 / hwnput3; approximately 5659 MB free HBM before Local; existing load recorded and left unchanged.
- Artifact result: Candidate object is 128 bytes smaller and executable sections are 52 bytes smaller; equal-size `.alink` files differ in content.
- Evidence: all remote `results` were copied into this V001 directory; required object and `.alink` files are under `results/artifacts/`.
- NEXT_ACTION: V002 will test one independent code-footprint hypothesis in the same low-precision helper, with no input-path or algorithm change. Follow `ONE CHANGE -> COMPILE -> CORRECTNESS -> LOCAL -> RESULT -> COMMIT -> VERSION_RECORD_EVENT` and keep Local as `NONE` if d5 noise remains high.

