# V090 Partial Ranking Result

- `ROUTE=MODE-DISPATCH-CUTOFF-X`; `REVISION=V090`
- `CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1032`; direct Parent is exact `R31B-V011` (`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`).
- Candidate source SHA256: `57de1e6e90ccbb0627f98200b7bc3ca7b29f4bd6a206f9a614c7969210b0ddba`.
- Compile and reference-probe build: PASS. Parent and Candidate correctness: PASS on FP32 128x1024, 128x1032, and 128x1040; each invocation `rc=0`, `bad=0`. C15 FP32 1x32768 remains excluded as an exact-Parent known failure, not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.

## Local Result

Device 4, FP32, six interleaved Parent/Candidate pairs per shape in alternating P,C / C,P order; 45 warmups, 31 samples per block, two blocks, `batch_n=64`, and no discarded samples. All 36 Local invocations returned `rc=0`, `bad=0`. The raw event and wall samples, per-invocation stats, command logs, and pre/post device/process snapshots are retained in this directory.

The route-local score is `0.979930246590x` (`LOCAL_DELTA=-2.006975341%`). Per-pair speedup is `median(Parent device_us) / median(Candidate device_us)`; each shape aggregates its six pair speedups arithmetically, and the three shapes are combined with an equal-weight geometric mean. All 18/18 paired median shifts are inside combined MAD; Candidate is faster in 6/18 pairs. The numeric result is retained, but the mixed direction and noise-floor overlap make it non-promotable.

`LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`. `CURRENT_LOCAL_BEST=R31B-V011` is unchanged; V090 is not promoted. The captured pre/post snapshots show sustained device load; the inference/process inventories are retained and existing processes were not disturbed.

- Local capture ended `2026-10-08T12:20:08.216139406Z`.
- Device 4 was released `2026-10-08T12:25:05.809056739Z`.
- No V091 device use or timing is included in this result.
