# SYNC-BARRIER-ELISION-X V055 Result

- Parent: exact R31B-V011, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Change: delete only the `PIPE_V` barrier after the scalar `Sqrt` loop and before `SyncVToS()` in `ProcessSmallLowPrecisionContiguousBatched`.
- Compile: PASS. Correctness: bitwise PASS on 7/7 FP16 cases.
- Local: 62 interleaved device-event pairs on device 3, FP16 128x128; four 31-pair same-binary qualification runs also retained.
- Primary score: `100 * (median(Parent device_us) / median(Candidate device_us) - 1) = -9.346365%`; pooled medians `14.84/16.37 us`, a `+1.53 us` Candidate median-latency increase.
- Reconciliation: ratio-of-means score `-6.411523%`; runner-style median-paired-delta score `+0.269542%`. Block ratio-of-medians scores are mildly positive (`+1.855895%`, `+3.516484%`), but both block ratio-of-means scores are negative (`-6.504465%`, `-6.280385%`). Candidate-faster pairs: 32/62.
- Quality: pooled device-event CV is `0.408600` Parent / `0.520575` Candidate; Candidate qualification run 2 median drift is `37.1%`. Preserve every sample and classify `LOCAL_REJECTED_NOISY`; do not promote.
- Local Best remains R31B-V011. This single-shape Local is not comparable to Official 45.16. No Online action was taken.
- Device 3 was explicitly released after numeric result capture; snapshots, process context, and release receipt are preserved.

Raw timing, throughput, wall latency, qualification, and load data are linked from `local-result.json`.
