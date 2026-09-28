# V004 N1 timing SUMMARY — REDUCE-HIER-X

Device: d4 (lease R2-REDUCE-V004-TIMING). method=DEVICE_EVENT_PRIMARY+HOST_WALL_SECONDARY
warmup=45, samples=41, same-binary blocks=2 gap=2s, interleaved P/C pairs=6

## Same-binary qualification (device-event)

| shape | variant | n | B1_med | B2_med | MAD/med | drift | verdict |
|---|---|---|---|---|---|---|---|
| 1x32768_fp32 | P | 82 | 11.960 | 12.020 | 0.0234 | 0.0050 | PASS |
| 1x32768_fp32 | C | 82 | 15.000 | 15.860 | 0.0385 | 0.0557 | PASS |
| 1x16384_fp32 | P | 82 | 8.160 | 8.660 | 0.0475 | 0.0595 | PASS |
| 1x16384_fp32 | C | 82 | 9.400 | 9.440 | 0.0445 | 0.0042 | PASS |
| 8x8192_fp32 | P | 82 | 6.480 | 6.840 | 0.0418 | 0.0541 | PASS |
| 8x8192_fp32 | C | 82 | 118.600 | 13.380 | 0.7433 | 1.5945 | MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE |
| 1x4096_fp32 | P | 82 | 12.040 | 5.920 | 0.2387 | 0.6815 | MEASUREMENT_PROTOCOL_BLOCKED_FOR_SHAPE |
| 1x4096_fp32 | C | 82 | 4.400 | 5.260 | 0.1654 | 0.1781 | NEEDS_VALIDATION |

## Interleaved P/C paired deltas (device-event block medians)

| shape | pair | P_med | C_med | delta% | fav |
|---|---|---|---|---|---|
| 1x32768_fp32 | 1 | 12.200 | 14.620 | +19.84 | P |
| 1x32768_fp32 | 2 | 11.820 | 17.200 | +45.52 | P |
| 1x32768_fp32 | 3 | 11.760 | 15.100 | +28.40 | P |
| 1x32768_fp32 | 4 | 12.180 | 14.900 | +22.33 | P |
| 1x32768_fp32 | 5 | 11.940 | 14.800 | +23.95 | P |
| 1x32768_fp32 | 6 | 11.920 | 14.760 | +23.83 | P |
| **1x32768_fp32 median** | n=6 | | | **+23.89** | favC=0 favP=6 |
| 1x16384_fp32 | 1 | 7.840 | 9.540 | +21.68 | P |
| 1x16384_fp32 | 2 | 8.180 | 9.400 | +14.91 | P |
| 1x16384_fp32 | 3 | 7.720 | 9.500 | +23.06 | P |
| 1x16384_fp32 | 4 | 7.760 | 14.560 | +87.63 | P |
| 1x16384_fp32 | 5 | 8.180 | 9.500 | +16.14 | P |
| 1x16384_fp32 | 6 | 7.840 | 9.460 | +20.66 | P |
| **1x16384_fp32 median** | n=6 | | | **+21.17** | favC=0 favP=6 |
| 8x8192_fp32 | 1 | 6.620 | 16.640 | +151.36 | P |
| 8x8192_fp32 | 2 | 6.520 | 7.360 | +12.88 | P |
| 8x8192_fp32 | 3 | 6.520 | 15.900 | +143.87 | P |
| 8x8192_fp32 | 4 | 6.840 | 7.620 | +11.40 | P |
| 8x8192_fp32 | 5 | 6.460 | 7.600 | +17.65 | P |
| 8x8192_fp32 | 6 | 6.580 | 7.260 | +10.33 | P |
| **8x8192_fp32 median** | n=6 | | | **+15.27** | favC=0 favP=6 |
| 1x4096_fp32 | 1 | 6.000 | 5.560 | -7.33 | C |
| 1x4096_fp32 | 2 | 16.060 | 5.340 | -66.75 | C |
| 1x4096_fp32 | 3 | 5.280 | 6.620 | +25.38 | P |
| 1x4096_fp32 | 4 | 4.960 | 5.420 | +9.27 | P |
| 1x4096_fp32 | 5 | 5.420 | 6.040 | +11.44 | P |
| 1x4096_fp32 | 6 | 5.200 | 45.900 | +782.69 | P |
| **1x4096_fp32 median** | n=6 | | | **+10.36** | favC=2 favP=4 |

## Notes

- rc=3 on 1x32768/1x16384 raw runs is the parent-preexisting wide-FP32 golden mismatch (bad>0), not a timing failure; device-event samples are valid.
- 8x8192 C same-binary B1_med=118.600 is a polluted block (B2_med=13.380); its P/C pairs 2/4/5/6 (C ~7.3-7.6us) still show +10..18% favP.
- 1x4096 is the untouched S5 negative control; both sides noisy (P BLOCKED, C NEEDS_VALIDATION).
- PRIMARY SHAPES both sides qualified: 1x32768 +23.89% (6/0 favP), 1x16384 +21.17% (6/0 favP). N1 is a stable regression.
