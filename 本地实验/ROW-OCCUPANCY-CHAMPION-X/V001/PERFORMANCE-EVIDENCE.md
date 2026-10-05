# ROW-OCC-H1 V001 Canonical Performance Evidence

ROUTE=ROW-OCCUPANCY-CHAMPION-X
REVISION=V001
DIRECT_PARENT=R31B/V011
PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA=7757274fe51507a2f8bf599c5771a9f1e9b21dce68c66a0b2b97934d7f4d1315
SCORER_VERSION=MAIN2-CANONICAL-V1
SUITE_VERSION=CANONICAL_LOCAL_SUITE_V1
DEVICE=4
FORMAL_LEASE=M2-ROW-OCC-H1-V001-D4-20261005T142600Z
FORMAL_CELLS=21/21
VALID_RUNS=42/42
WARMUP=45
SAMPLES_PER_RUN=31
ORDER=run1 P-C; run2 C-P; run3 P-C
TIMING_PRIMARY=DEVICE_EVENT

## Canonical result

CANONICAL_LOCAL_SCORE=94.801850
DELTA_VS_R31B_V011=-5.198150
SCORER_STATUS=LOCAL_NEUTRAL
LOCAL_VERDICT=LOCAL_REJECTED
NUMERICAL_POSITIVE=NO
RELIABLE_LOCAL_POSITIVE=NO
USER_ONLINE_CANDIDATE=NO
PAIRED_DIAGNOSTIC_SCORE=95.797631
CORE_GAIN_COUNT=0/4
CORE_DIRECTION=all four candidate medians slower than the fresh anchor

The scorer's overall quality is `POOR` because the frozen fresh anchor's C16
cell is POOR. This is a quality classification, not a discarded measurement:
all 21 formal cells completed with `RC=0`, `bad=0`, and 31 positive device
samples per invocation. The complete raw data remains under `formal/`.

## Core cases

| Case | Shape | Parent median us | Candidate median us | Candidate/anchor | Latency delta | Parent throughput | Candidate throughput | Throughput delta | Quality |
|---|---|---:|---:|---:|---:|---:|---:|---:|---|
| C12 | 8x8192 BF16 | 6.600000 | 6.840000 | 1.039514 | +3.951368% | 9929696969.696970 | 9581286549.707603 | -3.801170% | GOOD |
| C13 | 128x16384 FP16 | 21.280000 | 22.500000 | 1.081731 | +8.173077% | 98550375939.849625 | 93206755555.555557 | -7.555556% | GOOD |
| C14 | 128x16384 BF16 | 26.100000 | 27.140000 | 1.045455 | +4.545455% | 80350651340.996170 | 77271628592.483414 | -4.347826% | GOOD |
| C16 | 9x32768 BF16 | 15.220000 | 15.860000 | 1.053121 | +5.312085% | 19376609724.047306 | 18594703656.998741 | -5.044136% | GOOD |

The frozen anchor quality for C16 is POOR, so the aggregate quality is POOR;
the candidate-side C16 run-to-run quality itself is GOOD. Diagnostic vectors
are retained in `canonical-vector.tsv` and do not affect the four-case score.

## Interpretation

The H1 launch-width change did not show a positive signal. C13 and C14 are
the cases where the two-logical-block ceiling is active and both regress.
C12 and C16 clamp to the row count under both policies and are controls; they
also do not improve. Throughput follows latency in every core case.

No Online submission or recommendation was made. The global Official anchor
remains R31B V011 at 45.16 and the route's next revision, if later selected,
must use an explicit sibling declaration from the same R31B V011 parent.
