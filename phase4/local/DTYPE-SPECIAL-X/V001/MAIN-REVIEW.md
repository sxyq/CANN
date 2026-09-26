# MAIN Review: DTYPE-SPECIAL-X V001

Date: 2026-09-27

## Current Evidence

- Direct parent: R31B-V011, source SHA-256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source SHA-256: `e2717055199f541d98d85ceef2e011e52e2749d932ca954dc887868de7db880f`; server3 copy matches.
- Server3 Parent, Candidate, and unified-runner targets compiled and linked with RC 0 using the existing GCC 11 include setup.
- Executable SHA-256 values: Parent `11e3c07c4e35181153b20f5acd90cad700261af9d309d52724cb0713a1646434`; Candidate `14cdc6cf6f986fd49f92662b4ff180266d0265cafdc0145706e37153b984a90b`; unified runner `b14b57e32d3b9029bf01f1dca339599063b6be05bda750e189de01c7c2a81306`.
- Existing exact-source NPU correctness remains 39/39 PASS, max absolute error `2.6226044e-06`.

## Disposition

No local performance verdict has been formed. `BUILD=PASS`; `CORRECTNESS=PASS`; `EXECUTABLE_IDENTITY=PASS`; `READY_FOR_SAME_BINARY=true`. `TIMING=MEASUREMENT_BLOCKED` is the timing-stage state only: allowed devices d4-d6 currently have heavy VLLM HBM use, and d7 is excluded from performance timing. No Local Best change and no Online submission.

Next action: with a fresh exclusive performance lease, run Parent same-binary qualification for each selected exact shape before paired samples.
