# SYNC-BARRIER-ELISION-X V006

## Parent

- Direct Parent: `R31B V011` (`CURRENT_LOCAL_BEST`).
- Parent SHA-256: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Parent copy: `parent.asc`.

## Single change

Delete the FP16-path `AscendC::PipeBarrier<PIPE_V>()` immediately after `Muls(valueLocal, valueLocal, invRms, valid)` in `ProcessNarrowMidOverlap`. No other kernel operation or pipeline behavior changed.

- Candidate: `submission.asc`.
- Candidate SHA-256: `acff22e50cc4aa2ddea833e6ef8da6625800740ac71cc77841e63cb11c6d6581`.
- Exact source diff: `diff.patch`.

## Gate results

- Compile: `PASS` on `hwnput3`, CANN `8.5.T8.0.B060`, `dav-2201`, completed `2026-10-07T05:35:32Z`. See `compile-result.json` and `logs/compile-local-20261007T053516Z.log`.
- Correctness: `PASS` on device 3 for FP16 widths 128, 256, 1024, 2048, and 4096; zero parent/candidate bit mismatches. See `correctness-result.json`.
- Local: `LOCAL_SCORE=+24.060984210%`, `LOCAL_DELTA=+24.060984210%` from all 62 paired samples, FP16 8x2048 on device 3. Verdict `LOCAL_REJECTED`, `LOAD_QUALITY=LOW/NOISY`; per-run geomean directions disagree substantially. No Local Best promotion. See `local-result.json` and `logs/local-device3-20261007T053818Z.log`.
- Online: `NOT_RUN`.

## Timing and ownership

- Previous Local result end: `2026-10-07T05:04:35Z` (V005).
- V006 source edit mtime: `2026-10-07T05:35:16Z`.
- `LOCAL_RESULT_TO_NEXT_EDIT_SECONDS=1841` (`SLA=180s`, `COMPLIANCE=MISS`).
- V005 result commit: `e26bade18e9acdb11bfb53867b44c75962a224c7`.

V006 is a separate result under the same route branch. `CURRENT_LOCAL_BEST` remains `R31B-V011`; no Online or shared-record action was taken.
