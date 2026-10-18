# w5ctl historical Canonical validation receipt

Scope: published Git commit `6d3ad0b30b21d2462b0fd8716c884f0ff6bdaabb`,
`MAIN2-CANONICAL-V1` / `VECTOR-MATH-X__V003`. This is a Tooling validation
input, not the W5-R01 `CROSSROW-EPI-PROLOGUE-X/V001` input, not an R2
Register-Spill Kernel Revision, and not an R2 performance result.

## Identity and published facts

- Candidate source: `submission.asc`, SHA-256
  `f019d099973b8ab4df07741b494376d9807a6d6a0b1686cfc0aca63ee4245bc8`.
- Published candidate executable identity: `EXE_SHA`
  `da52970a469da6cc5e317b36c7048e17412ddf9d5266f7f5a9d94d415e4e48b3`.
  The executable bytes are not present in the pinned public commit; w5ctl uses
  `build.json` as a `PUBLIC_BUILD_RECEIPT`, never as executable bytes.
- Parent source SHA: `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Parent artifact SHA: `eed03216f3ad02a7c5dde0978f50edff218619286e4b6a479c79b845cd843c71`.
- Build receipt SHA-256: `b028871965a42cb1c15425696a885b5a8fd68e13c0b3543344078029b039719c`.
- Correctness receipt SHA-256: `f48fe23ec1212f68ea0c0bcc8b8707e1db1660529e1ca3337febe0f805e38451`.
- Measurements SHA-256: `cc82e753ea8f1f956e6333b799c1bd72d065cdcd8735f796f54fdc66f7a35c2d`.
- Route-local manifest SHA-256:
  `e5994474b76f31a1de9834256935552c95ced2b6b5667a7b99c05fd2d711d1a7`.

## Actual validation results

- Published Build receipt: PASS / RC 0.
- Published Correctness receipt: PASS / 7 of 7 cells, all RC 0/BAD 0.
- w5ctl Canonical Local stats-only: RC 0; 48 raw/stats identities,
  C12/C13/C14/C16 complete, published scorer result `60.477249`, quality
  `POOR`, `LOCAL_NEUTRAL`, reliable `NO`, promotion `NO`.
- Route-local record: `RECORD_STATUS=PASS`,
  `BACKEND_STATUS=NOT_REQUESTED`; manifest contains source/artifact/raw and
  evidence SHA values plus the Canonical score and stage receipts.
- w5ctl report: `IDENTITY_STATUS=PASS`, `BUILD_STATUS=PASS`,
  `CORRECTNESS_STATUS=PASS`, `LOCAL_SCORE_COMPLETENESS=COMPLETE`,
  `REPORT_STATUS=PASS`, `FULL_LOCAL_SCORE=NOT_COMPUTABLE`.
- `report --route-summary` excludes this historical scope; no R2 Kernel
  Revision, no shared scoreboard write, no Online submission.

## Boundary

`W5CTL_E2E=INCOMPLETE`: this validation consumes the already-published Build
and Correctness receipts and public four-Core measurements. It does not rerun
Build/Correctness, does not use the dirty shared tooling-runner, and does not
start NPU timing. A future true E2E receipt requires an Owner-approved existing
Build/Correctness/Local backend invocation with executable/event provenance;
this route does not invent that backend.

The public materials currently available for W5-R01 CROSSROW V001 still do
not provide the complete four-Core C12/C13/C14/C16, three-run paired
measurement bundle plus verified artifact/scorer metadata required for a
Canonical E2E receipt. The older R2 `crossrow-historical-tool-validation`
record remains one-shape `UNCLASSIFIED` evidence, and the separate
`r1-public-build-w5ctl.log` is WIDE-X-FRESH4; neither is used here.

Evidence logs:

- `w5ctl-local-validation.stdout.log`
- `w5ctl-record-validation-v2.stdout.log`
- `w5ctl-report-validation-v2.stdout.log`
