# V098 Partial Local Result

- `ROUTE=MODE-DISPATCH-CUTOFF-X`; `DIRECT_PARENT=exact R31B-V011` (SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`).
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1000`; Candidate SHA256 `064e28930a69c6c921952a554765061f5df780df536ee52eb6f68f177d72a38c`.
- `COMPILE=PASS`; Parent/Candidate Correctness PASS for FP32 `128x996/1000/1004`; all six checks returned `rc=0,bad=0`.
- Interleaved device-event Local on device 2: six P/C pairs per shape, 45 warmups, 31 samples per block, two blocks, batch 64. All 36 invocations returned `rc=0,bad=0`; all 2,232 event samples are retained (372 per side per shape). No samples were excluded.
- Formula: each pair is `median(Parent raw device_us) / median(Candidate raw device_us)`; shape score is the arithmetic mean of six pair speedups; route score is the equal-weight geometric mean of the three shape scores.

| Shape | Six pair speedups | Shape score / delta | Faster / within MAD | Parent/Candidate pooled median us | Parent/Candidate CV | Parent/Candidate throughput Gelem/s |
|---|---|---:|---:|---:|---:|---:|
| 128x996 | 0.978177, 1.013295, 0.936237, 0.935659, 0.983856, 0.959289 | 0.967752x / -3.224765% | 1/6 / 3/6 | 7.546095 / 7.777660 | 3.335% / 4.147% | 16.894566 / 16.391563 |
| 128x1000 | 0.969575, 0.989204, 0.984056, 0.938936, 1.003332, 0.931687 | 0.969465x / -3.053495% | 1/6 / 4/6 | 7.566720 / 7.842970 | 5.405% / 12.041% | 16.916180 / 16.320348 |
| 128x1004 | 0.940592, 0.991289, 1.002477, 0.977876, 1.019667, 0.944771 | 0.979445x / -2.055461% | 2/6 / 4/6 | 7.830000 / 7.933280 | 4.688% / 7.777% | 16.412771 / 16.199100 |
| Equal-shape geomean | - | **0.972207290914x / -2.779270909%** | **4/18 / 11/18** | - | - | - |

## Quality and Decision

- `LOCAL_SCORE=0.972207290914x`; `LOCAL_DELTA=-2.779270909%` (partial route-local score only).
- `LOAD_QUALITY=PROCESS_PRESENT`; across 36 pair snapshots, FREE_HBM was 59,098-59,100 MB and AICore 0-15%. Existing PID 2055832 was present and untouched.
- `MEASUREMENT_QUALITY=NOISY/MIXED`: pooled CV ranged 3.335%-12.041%; 4/18 pair medians favored Candidate and 11/18 shifts were within combined MAD. All three shape aggregates favored Parent; the numeric result retains all raw samples.
- `LOCAL_VERDICT=LOCAL_REJECTED`; `CURRENT_LOCAL_BEST=exact R31B-V011`; V098 is not promoted.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- Exact Parent C15 FP32 `1x32768` remains excluded due to its known Parent correctness failure; not rerun and not a Candidate regression.
- Device 2 was released after capture at `2026-10-08T19:02:56.069376305Z`.

## Timestamps

- `COMPILE_START=2026-10-08T18:54:37.046720173Z`; `COMPILE_PASS=2026-10-08T18:54:48.452139266Z`.
- `CORRECTNESS_START=2026-10-08T18:56:18.681922561Z`; `CORRECTNESS_END=2026-10-08T18:56:59.038296193Z`.
- `LOCAL_START=2026-10-08T18:57:34.778706135Z`; `LOCAL_CAPTURE_END=2026-10-08T19:02:54.124260444Z`.
- `DEVICE_RELEASE=2026-10-08T19:02:56.069376305Z`; `LOCAL_RESULT=2026-10-08T19:04:01.935653863Z`.
