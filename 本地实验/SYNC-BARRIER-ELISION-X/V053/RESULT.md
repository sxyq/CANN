# SYNC-BARRIER-ELISION-X V053 Result

- Parent: exact R31B-V011, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Change: delete the single `SyncSToV()` after `invRmsValues` extraction in `ProcessSmallLowPrecisionContiguousBatched`.
- Compile: PASS. Correctness: bitwise PASS on 7/7 FP16 cases.
- Local: 62 interleaved device-event pairs on device 3, FP16 128x128. All 124 Parent/Candidate same-binary qualification pair entries (248 individual timed observations) and all raw Local pairs are retained.
- Primary formula: `100 * (median(Parent device_us) / median(Candidate device_us) - 1)`. Pooled medians are `15.47/15.82 us`; score `-2.212389%` (Candidate median latency 2.2624% higher).
- Reconciliation: ratio-of-means score is `+4.392048%`, while median paired delta is `+0.65 us` and runner-style score is `-4.201681%`. Block ratio-of-medians scores reverse from `-9.704142%` to `+2.777778%`. Candidate-faster pairs are 29/62.
- Quality: pooled device-event CV is 0.3482 Parent / 0.3094 Candidate; qualification medians shift markedly between repeat runs. The disagreement among pooled means, medians, paired deltas, and blocks makes the result `LOCAL_REJECTED_NOISY`; no sample was removed and no Local Best promotion is made.
- Local Best remains R31B-V011. This single-shape Local result is not comparable to Official 45.16. No Online action was taken.
- Device 3 was explicitly released after numeric capture; post-capture snapshot and release receipt are preserved in the `logs/` directory and `DEVICE_RELEASE_RECEIPT.md`.

Raw samples, throughput, wall latency, qualification output, and device/load snapshots are linked from `local-result.json`.
