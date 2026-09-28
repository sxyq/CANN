# V005 N3 timing SUMMARY — REDUCE-HIER-X

Device: d4 (lease R2-REDUCE-V005-TIMING). method=DEVICE_EVENT_PRIMARY+HOST_WALL_SECONDARY
warmup=45, samples=41, same-binary blocks=2 gap=2s, interleaved P/C pairs=6

## Same-binary qualification (device-event)

| shape | variant | n | B1_med | B2_med | MAD/med | drift | verdict |
|---|---|---|---|---|---|---|---|
| 1x32768_fp32 | P | 82 | 12.200 | 12.500 | 0.0416 | 0.0243 | PASS |
| 1x32768_fp32 | C | 82 | 12.840 | 13.260 | 0.0354 | 0.0322 | PASS |
| 1x16384_fp32 | P | 82 | 8.100 | 8.180 | 0.0465 | 0.0098 | PASS |
| 1x16384_fp32 | C | 82 | 8.520 | 8.480 | 0.0212 | 0.0047 | PASS |
| 8x8192_fp32 | P | 82 | 6.380 | 6.420 | 0.0485 | 0.0063 | PASS |
| 8x8192_fp32 | C | 82 | 58.120 | 7.080 | 0.1173 | 1.5656 | MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE |
| 1x4096_fp32 | P | 82 | 6.120 | 4.420 | 0.1705 | 0.3226 | MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE |
| 1x4096_fp32 | C | 82 | 6.300 | 19.460 | 0.6415 | 1.0217 | MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE |

## Interleaved P/C paired deltas (device-event block medians)

| shape | pair | P_med | C_med | delta% | fav |
|---|---|---|---|---|---|
| 1x32768_fp32 | 1 | 34.500 | 12.900 | -62.61 | C |
| 1x32768_fp32 | 2 | 11.960 | 13.060 | +9.20 | P |
| 1x32768_fp32 | 3 | 11.900 | 12.820 | +7.73 | P |
| 1x32768_fp32 | 4 | 11.940 | 12.400 | +3.85 | P |
| 1x32768_fp32 | 5 | 11.500 | 39.460 | +243.13 | P |
| 1x32768_fp32 | 6 | 12.120 | 13.000 | +7.26 | P |
| **1x32768_fp32 median** | n=6 | | | **+7.50** | favC=1 favP=5 |
| 1x16384_fp32 | 1 | 8.100 | 8.620 | +6.42 | P |
| 1x16384_fp32 | 2 | 7.900 | 8.200 | +3.80 | P |
| 1x16384_fp32 | 3 | 8.100 | 8.460 | +4.44 | P |
| 1x16384_fp32 | 4 | 7.600 | 8.540 | +12.37 | P |
| 1x16384_fp32 | 5 | 7.760 | 8.520 | +9.79 | P |
| 1x16384_fp32 | 6 | 7.660 | 8.480 | +10.70 | P |
| **1x16384_fp32 median** | n=6 | | | **+8.11** | favC=0 favP=6 |
| 8x8192_fp32 | 1 | 6.700 | 6.680 | -0.30 | C |
| 8x8192_fp32 | 2 | 7.040 | 6.820 | -3.12 | C |
| 8x8192_fp32 | 3 | 7.200 | 7.040 | -2.22 | C |
| 8x8192_fp32 | 4 | 6.620 | 16.740 | +152.87 | P |
| 8x8192_fp32 | 5 | 15.340 | 6.720 | -56.19 | C |
| 8x8192_fp32 | 6 | 6.520 | 6.760 | +3.68 | P |
| **8x8192_fp32 median** | n=6 | | | **-1.26** | favC=4 favP=2 |
| 1x4096_fp32 | 1 | 5.660 | 6.000 | +6.01 | P |
| 1x4096_fp32 | 2 | 4.780 | 5.720 | +19.67 | P |
| 1x4096_fp32 | 3 | 5.920 | 18.440 | +211.49 | P |
| 1x4096_fp32 | 4 | 6.880 | 6.380 | -7.27 | C |
| 1x4096_fp32 | 5 | 5.980 | 19.880 | +232.44 | P |
| 1x4096_fp32 | 6 | 5.060 | 6.760 | +33.60 | P |
| **1x4096_fp32 median** | n=6 | | | **+26.63** | favC=1 favP=5 |

## Notes

- PRIMARY (both same-binary PASS): 1x32768 median +7.50% (5/6 favP; clean pairs 2/3/4/6 +3.85..+9.20% all favP); 1x16384 median +8.11% (6/6 favP, +3.80..+12.37%).
- 8x8192: candidate same-binary BLOCKED (B1 58.12 polluted); clean P/C pairs 1/2/3/6 near-neutral (-3.12..+3.68%).
- 1x4096 control (S5 untouched): both sides BLOCKED, no signal.
- Verdict direction: N3 two-step WholeReduceSum is a small consistent regression on large-D primary shapes. The 2-vcadds-per-tile adapter costs more than the per-call V/S+get_acc_val+S_MTE3 chain it removes.
