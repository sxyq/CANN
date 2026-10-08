# ROW-SCALE-HOIST-X V055 Result

- Parent: V026, source SHA-256 `7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`.
- Candidate: source SHA-256 `07ef84374e1668f7f57dd9e2a6b20b99f4588c25b27c8f3d9d3ed9a1fa157228`.
- Change: BF16 full-row gamma/scale placement in `ProcessBf16FullRowOutputPipelined` only.
- Compile: PASS. Parent and Candidate correctness: PASS on BF16 `[128,8192]`; matched ratio `1.0`, max absolute error `0.00781869888` for each. Candidate dispatch audit confirmed the changed branch executed.
- Local protocol: device 0, 45 warmups and 32 event-timed samples per invocation; 32-sample Parent stability run and four interleaved Parent/Candidate blocks. All nine invocations exited 0; all 1,024 timed samples are retained with no exclusions.

## Numeric Result

- Parent pooled median/mean: `16.7000005 / 19.0801563 us`.
- Candidate pooled median/mean: `19.8400000 / 21.6442188 us`.
- Descriptive pooled-median speedup index: `84.1733896`.
- Candidate median delta: `+18.8023917%` slower. Pooled-mean delta: `+13.4383731%` slower.
- Paired block speedup indices: `113.6505917`, `76.3492782`, `109.1432094`, `61.5740741`; Candidate faster in `2/4` blocks. Paired block geometric-mean index: `87.3861914`.
- Pooled Parent/Candidate CV: `0.376724 / 0.431210`. Parent stability CV: `0.189575`; Candidate MAD/median: `0.256048`.

## Verdict

`MEASUREMENT_BLOCKED`, descriptive only; no Local Best promotion. The canonical protocol completed, but pooled variability is high, paired direction is split, and both pooled median and mean favor Parent. `CURRENT_LOCAL_BEST=V026`.

Device 0 was released after numeric capture. The pre-Local snapshot recorded `9,133/65,536 MB` HBM used; the post-capture snapshot recorded `9,134/65,536 MB`. No shared records or Online state were changed.

Raw samples, device snapshots, executable hashes, compile/correctness records, and the full result are retained in this V055 directory.
