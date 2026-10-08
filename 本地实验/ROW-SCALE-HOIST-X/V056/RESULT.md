# ROW-SCALE-HOIST-X V056 Result

- Direct parent: V026, exact source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Candidate source SHA-256: `9bfd5afed03e679ffbc5849b97164de9224e15d784ac4407b19519f344050096`.
- Change: BF16 full-tile gamma-scratch row-scale placement in `ProcessBf16FullTileBatchedOutputPipelined` only.
- Compile: PASS for `device` and `submission`; fresh build `/tmp/cann-row-scale-hoist-x-v056-compile-20261008T151354Z`.
- Correctness: Parent and Candidate PASS for BF16 `[128,4096]`; each matched ratio `1.0`, max absolute error `0.00781393051`. Dispatch audits confirm the full-tile batched branch and Candidate delta execution.
- Local: device 0; 45 warmups and 32 event-timed samples per invocation; 32-sample Parent stability plus four interleaved P/C blocks. All nine invocations exited 0; 128 raw samples per arm are preserved in `local/`.

## Numeric Result

- Parent pooled median/mean: `16.7699995 / 17.0849999 us`.
- Candidate pooled median/mean: `19.1999995 / 21.6173437 us`.
- Descriptive pooled-median score index: `87.3437497`.
- Candidate median delta: `+14.4901614%` slower. Pooled-mean delta: `+26.5282049%` slower.
- Paired block score indices: `82.7483695`, `89.5157289`, `84.1524620`, `84.7963330`; Candidate was slower in `4/4` blocks. Paired geometric score index: `85.2659576`.
- Pooled Parent/Candidate CV: `0.350964 / 0.801675`. Parent stability CV: `0.538813`, MAD/median: `0.538947`. Candidate maximum: `155.259995 us`.

## Verdict

`MEASUREMENT_BLOCKED`, numeric result retained as descriptive only. The Parent stability run was highly variable, and the Candidate distribution had substantial jitter. All paired block medians and both pooled aggregates favor Parent, so there is no Local Best promotion. `CURRENT_LOCAL_BEST=V026`.

The pre-Local snapshot recorded HBM usage at 13% of 65,536 MB, AICore usage at 16%, and an existing Python process using 5,752 MB. The process was left untouched. Post-capture device snapshot and explicit release receipt are recorded separately.

The measurement is Local-only and is not Official-comparable. No shared records or Online state were changed.
