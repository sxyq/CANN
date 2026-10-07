# V040 Local Result

- Parent: V026 (`7c2569d9e3830c845f2b5dc4587a73a05a0bdf041168614cd55ad84204e063f9`)
- Change: In `ProcessSmallFp32FullTileBatched`, move the FP32 per-row row-scale `Muls` after gamma `Mul`, before bias `Add`.
- Compile: PASS; `device` and `submission` targets.
- Correctness: PASS for Parent and Candidate, FP32 `[128,4096]`, matched ratio 1.0, maximum absolute error `4.05311584e-06`.
- Local: `MEASUREMENT_BLOCKED`; descriptive score `99.3211540293`, delta `+0.6834857864%` (Candidate slower).
- Pooled medians: Parent `19.0200005 us`, Candidate `19.1499995 us`; 96 samples per arm; pooled CV `0.351848` / `0.284473`.
- Paired block medians (Parent / Candidate, us): P1/C1 `16.6900005 / 21.4200005` (`+28.340323%`); P2/C2 `18.24 / 16.759998` (`-8.114046%`); P3/C3 `21.39 / 16.870001` (`-21.131365%`). Candidate direction was mixed across blocks.
- Parent stability medians were `19.4000005` and `18.46 us`; host load was high and shared activity on other devices was observed. Preserve the raw values as noisy evidence; do not promote.
- `CURRENT_LOCAL_BEST=V026`; Official score absent; Online not run.

Evidence: `compile-result.json`, `correctness-result.json`, `local-result.json`, `submission.sha256`, `source-meta.json`, and `diff.patch`; raw compile, correctness, paired samples, stability samples, and device/host snapshots are retained alongside them.
