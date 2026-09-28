# MAIN Review: DTYPE-SPECIAL-X V001

Date: 2026-09-27

## Current Evidence

- Direct parent: R31B-V011, source SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source SHA-256: `e2717055199f541d98d85ceef2e011e52e2749d932ca954dc887868de7db880f`; server3 copy matches.
- Server3 Parent, Candidate, and unified-runner targets compiled and linked with RC 0 using the existing GCC 11 include setup.
- Executable SHA-256 values: Parent `11e3c07c4e35181153b20f5acd90cad700261af9d309d52724cb0713a1646434`; Candidate `14cdc6cf6f986fd49f92662b4ff180266d0265cafdc0145706e37153b984a90b`; unified runner `b14b57e32d3b9029bf01f1dca339599063b6be05bda750e189de01c7c2a81306`.
- Existing exact-source NPU correctness remains 39/39 PASS, max absolute error `2.6226044e-06`.

## Disposition

`BUILD=PASS`; `CORRECTNESS=PASS`; `EXECUTABLE_IDENTITY=PASS`. Shape `[12,64]` failed same-binary block-drift qualification and was not timed. Shape `[12,8192]` passed the empirical Parent noise-floor criteria, then completed four interleaved Parent/Candidate device-event blocks on d6. The median pair delta was `-0.929099%` (`-0.07 us`) against a `0.21 us` median Parent MAD; pair directions were mixed. `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`. No Local Best change and no Online submission.

The measurement ran with AICore at 0%, HBM at 91%, and existing VLLMWorker_TP PID 91228. The Parent same-binary p90 values were `140.72 us` and `8.38 us`; these tails remain in raw evidence. This result supports a local shape verdict only and does not establish a clean-load performance gain.

Next action: keep V001 unchanged; on a fresh eligible lease, repeat the exact-shape qualification and paired measurement before deciding whether another local sample changes the verdict.

## Second Local Set

On 2026-09-26 23:09-23:14 UTC, the same exact source, Parent, Candidate, and unified runner were reverified on d6. Shape `[12,8192]` passed a new Parent same-binary qualification (45 warmups, 31 samples x 2 blocks; MAD/median `0.033505/0.027432`, drift `0.032953`). The following four interleaved pairs were `-0.496282%`, `-1.017813%`, `0.000000%`, and `-0.761427%`; their median was `-0.628855%` (`-0.05 us`) against `0.24 us` Parent median MAD. The full distribution still contains long tails (Parent p90 `20.02/217.86 us`, raw CV `2.051/1.721`), retained in the raw files. The device had AICore 0%, HBM 91%, and VLLMWorker_TP PID 91228. Lease `M1-DTYPE-V001-D6-S07R-20260926T230927Z` was released after the postflight snapshot.

Verdict remains `NEEDS_ONE_MORE_LOCAL`: the second set is within the measured noise floor and does not establish an improvement. `LOCAL_BEST` is unchanged; the only timing-shape blocker is `[12,64]`, which remains unqualified.

## Third local attempt: d7, 2026-09-27

- Reused the exact existing Parent, Candidate, and unified runner identities. No build, link, or correctness rerun occurred.
- Lease: `M1-DTYPE-V001-D7-R3-20260927T064606Z`; shape `[12,8192]`; 45 warmups and 31 samples in each of two Parent blocks.
- Parent same-binary: `NEEDS_VALIDATION`; block MAD/median `0.056831` and `0.076546`; block drift `0.107549`; Parent correctness bad `0`. P/C timing was not run because the shape did not qualify.
- d7 HBM was `5%` before and after (`3431-3432/65536 MB`); AICore and processes were recorded only, with no process on d7. HBM admission passed.
- Raw evidence: `logs/unified-dtype-noise-floor-20260927T064728Z-61624.raw.tsv`, `logs/unified-dtype-noise-floor-20260927T064728Z-61624.jitter.txt`, and the runner result `logs/unified-dtype-noise-floor-20260927T064728Z-61624.log`.

This attempt leaves `BUILD=PASS`; `CORRECTNESS=PASS`; `EXECUTABLE_IDENTITY=PASS`; `SAME_BINARY=NEEDS_VALIDATION` for this attempt; `TIMING=MEASUREMENT_BLOCKED` for this shape; `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`. Earlier valid shape-7 P/C evidence remains retained and still does not establish a performance gain. `LOCAL_BEST` and Online state are unchanged.
