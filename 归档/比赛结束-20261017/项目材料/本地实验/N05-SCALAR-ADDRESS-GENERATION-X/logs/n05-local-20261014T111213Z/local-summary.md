# N05 V014 Local Summary

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V014
PARENT_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256=3346b12b7f14e88e5188080f5810ec6327cd25338354f709435e4d676e58d817
PARENT_OBJECT_SHA256=a8d7738508bf4d6a61eb6651104396ce85be36a8834e7505aaf8fbb77d200b14
CANDIDATE_OBJECT_SHA256=c75fcd8eebd10c1c93ddc7d5ca68e772fd5efce2baace404441341cef578f5ed
PARENT_LIBRARY_SHA256=89a86ee883bb96259de41595e3deb05a148c22bc9d59576a4fb780fb04dea47f
CANDIDATE_LIBRARY_SHA256=d1c9a5ebb1c537f3cd4f88044eab230766042fbcef9643cdf7d03b74302bc1e0
RUNNER_SHA256=40e16bede8f55e2cf0fb09b35488bf3f3bc9cd210db3d1f4cf11bb4bab9fa6ae

Shape was rows=128,width=12416, device=2, input generator R4_MULTIROW_PROXY_128XWIDTH_SEED0. Official Shape mapping remains UNKNOWN. Prior Parent and Candidate Correctness passed for FP16 and BF16 at this diagnostic shape. No rebuild was needed. All four timing stages returned RC=0, using 45 warmups, 21 samples, 2 same-binary blocks, and 4 paired blocks per dtype. Device-event time is the primary metric.

## Same-Binary Parent

| Dtype | n | Median us | Mean us | SD us | MAD us | P10-P90 us | Min-Max us | Block medians us |
|---|---:|---:|---:|---:|---:|---:|---:|---:|
| FP16 | 42 | 20.50 | 29.20 | 31.58 | 1.35 | 19.78-36.90 | 18.56-224.72 | 20.16 / 27.30 |
| BF16 | 42 | 30.77 | 32.08 | 11.50 | 9.29 | 20.72-45.36 | 19.38-61.40 | 26.96 / 32.80 |

The Parent same-binary block medians drifted by about 35.4% for FP16 and 21.7% for BF16. FP16 also had one 224.72 us sample.

## Paired Parent/Candidate

Delta is Candidate minus Parent; negative is faster.

| Dtype | Parent median us | Candidate median us | Paired delta median us | Paired delta median % | Delta P10-P90 us | Block delta medians us |
|---|---:|---:|---:|---:|---:|---|
| FP16 | 29.47 | 25.92 | -0.63 | -2.61% | -13.68 to +9.20 | +2.08, -3.20, -0.16, -0.88 |
| BF16 | 26.04 | 29.13 | +0.88 | +4.07% | -7.97 to +10.44 | +0.64, +1.00, +0.44, +3.12 |

FP16 block directions were inconsistent. BF16 block medians were slower for Candidate in all four blocks. Both paired distributions had large outliers and broad spread; neither gives a stable performance result.

## Load and Decision

Before: HBM 96%, AICore 8%, AIVector 6%, HBM bandwidth 1%. After: HBM 96%, AICore 9%, AIVector 9%, HBM bandwidth 12%. Python PID 278936, owned by penghao+, remained active before and after; it was not changed. No N05 Runner was active before the run, and it was absent after.

LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
LOAD_QUALITY=CONTAMINATED_SHARED_DEVICE
OFFICIAL_CASE_MAPPING=UNKNOWN
DECISION=INVESTIGATE_NEXT
PROMOTION=NO

Raw samples, stage stdout/stderr/status, identity, load snapshots, and the Main SSH transcript are stored in this directory. The diagnostic proxy does not establish an Official-case gain.
