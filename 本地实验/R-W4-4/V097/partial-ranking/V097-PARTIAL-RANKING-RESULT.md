# V097 Partial Local Result

- `ROUTE=MODE-DISPATCH-CUTOFF-X`; `DIRECT_PARENT=exact R31B-V011` (SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`).
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 1004`; Candidate SHA256 `3123a6ab012525957314b6c63b09ac61b905d12ee4559ab0b7de831124ba33eb`.
- `COMPILE=PASS`; `CORRECTNESS=PASS` for Parent and Candidate on FP32 `128x1000/1004/1008`; six checks returned `rc=0,bad=0`.
- Interleaved Local on device 2: six P/C pairs per shape, 45 warmups, 31 event samples per block, two blocks, batch 64. All 36 invocations returned `rc=0,bad=0`; all 2,232 raw event samples are retained (372 per side per shape). No sample was removed.
- Formula: each pair is `median(Parent raw device_us) / median(Candidate raw device_us)`; shape score is the arithmetic mean of six pair speedups; route score is the equal-weight geometric mean of the three shape scores.

| Shape | Six pair speedups | Shape score / delta | Faster / within MAD | Parent/Candidate pooled median us | Parent/Candidate CV | Parent/Candidate throughput Gelem/s |
|---|---|---:|---:|---:|---:|---:|
| 128x1000 | 0.985646, 0.933607, 1.016061, 0.925981, 0.924239, 0.997070 | 0.963768x / -3.623248% | 1/6 / 3/6 | 7.545780 / 7.818125 | 4.930% / 5.827% | 16.963124 / 16.372212 |
| 128x1004 | 0.980860, 0.976324, 1.001135, 0.987612, 0.984887, 1.038735 | 0.994925x / -0.507465% | 2/6 / 5/6 | 7.785465 / 7.852655 | 26.785% / 14.199% | 16.506657 / 16.365420 |
| 128x1008 | 0.980060, 0.900256, 0.990924, 0.908272, 0.998782, 1.000124 | 0.963070x / -3.693034% | 1/6 / 4/6 | 7.490780 / 7.741875 | 10.048% / 5.775% | 17.224374 / 16.665730 |
| Equal-shape geomean | - | **0.973808348572x / -2.619165143%** | **4/18 / 12/18** | - | - | - |

## Quality and Decision

- `LOCAL_SCORE=0.973808348572x`; `LOCAL_DELTA=-2.619165143%` (partial route-local score only).
- `LOAD_QUALITY=PROCESS_PRESENT`; across 36 pair snapshots, FREE_HBM was 59,097-59,101 MB and AICore 0-2%. Existing PID 2055832 was present and untouched.
- `MEASUREMENT_QUALITY=NOISY/INCONCLUSIVE`: pooled CV ranged 4.930%-26.785%; only 4/18 pair medians favored Candidate, while 12/18 shifts were within combined MAD. The numeric result is retained without excluding samples.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=exact R31B-V011`; V097 is not promoted.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- Exact Parent C15 FP32 `1x32768` remains excluded due to its known Parent correctness failure; not rerun and not a Candidate regression.
- An initial local resource-gate parser failed before any Local invocation. It is preserved in `v097-pre-local-resource-gate.txt`; the same fresh snapshot was correctly parsed in `v097-pre-local-resource-gate-retry1.txt` as 59,100 MB FREE_HBM, and the full Local capture then completed.
- Device 2 was released after capture at `2026-10-08T18:41:13.121190129Z`.

## Timestamps

- `COMPILE_START=2026-10-08T18:23:10.063607141Z`; `COMPILE_PASS=2026-10-08T18:23:20.815638116Z`.
- `CORRECTNESS_START=2026-10-08T18:29:53.817304316Z`; `CORRECTNESS_END=2026-10-08T18:30:36.394597319Z`.
- `LOCAL_START=2026-10-08T18:36:00.692976273Z`; `LOCAL_CAPTURE_END=2026-10-08T18:41:11.238834429Z`.
- `DEVICE_RELEASE=2026-10-08T18:41:13.121190129Z`; `LOCAL_RESULT=2026-10-08T18:42:50.198636961Z`.
