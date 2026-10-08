# SYNC-BARRIER-ELISION-X V054 Result

- Parent: exact R31B-V011, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Change: delete only the `SyncSToV()` after scalar `meanSquares` extraction and before `Duplicate` in `ProcessSmallLowPrecisionContiguousBatched`.
- Compile: PASS. Correctness: bitwise PASS on 7/7 FP16 cases.
- Local: 62 interleaved device-event pairs on device 3, FP16 128x128; qualification and raw timing samples are retained.
- Primary score: `100 * (median(Parent device_us) / median(Candidate device_us) - 1)`, giving pooled medians `11.69/9.11 us` and `+28.320527%`.
- Reconciliation: pooled ratio-of-means is `+14.499597%`; median paired delta is `-0.56 us`, yielding runner-style `+4.790419%`. Per-block ratio-of-medians scores reverse from `-24.835165%` to `+109.429825%`.
- Quality: pooled device-event CV is `0.6553` Parent and `0.5870` Candidate, with strongly drifting qualification medians. This is `LOCAL_REJECTED_NOISY`; no sample was filtered and no promotion was made.
- Local Best remains R31B-V011. Single-shape Local is not comparable to Official 45.16. No Online action was taken.
- Device 3 was released after numeric capture; snapshots and release receipt are preserved.
