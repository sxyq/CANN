# ADDR V001 / H3 — Online Eligibility After Fresh V011 Calibration

```text
OFFICIAL_ANCHOR=R31B V011 / 45.16
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_SHA256=26aa65a2e1313e0681ca7ad29f85ca20f667d33ede1a4d2d9e7e1557d6192602
FRESH_V011_TARGET_AVG_US=32.133000
HISTORICAL_CANDIDATE_AVG_US=46.470000
NORMALIZED_LATENCY_DELTA=+44.617683%
NORMALIZED_THROUGHPUT_DELTA=-30.852163%
QUALITY=POOR
ONLINE_ELIGIBLE=NO
ONLINE_READY_PACKAGE=NO
READY_FOR_EXTERNAL_JUDGE_OWNER=NO
DIRECT_ONLINE_SUBMISSION=0
```

## Gate review

| Gate | Finding |
|---|---|
| Exact parent/candidate source | PASS at source level. Candidate digest, sidecar, source commit, and V011 parent digest are verified in `ADDR-V001-ONLINE-PACKAGE-AUDIT-20261004.md`. The historical timing metadata does not retain a separate executable digest. |
| Target correctness | INCOMPLETE. BF16-D32768 reduced-tail passed Parent/Candidate correctness. BF16-D40960 still returns shared Parent/Candidate `507035`; this is not Candidate-specific, but the required domain is not closed. |
| Fresh comparable Champion baseline | Measurements exist on exact BF16 [9,32768], device 7, device-event runner. Only 1/3 independent Parent processes passed the shape floor; aggregate quality is POOR. |
| Candidate direction | Historical same-window evidence shows Candidate faster in only 1/3 runs, below the 2/3 requirement. The fresh Parent-only runs are separate time windows, so their 3-way values must not be treated as new paired directions. |
| Average latency / throughput | The reused Candidate average is slower than the fresh Parent aggregate (+44.62% latency; −30.85% throughput). The cross-window comparison is calibration-only and POOR. |
| Extreme sample sensitivity | FAIL. The historical score was strongly influenced by one extreme Parent timing; raw samples show large tails on both sides. |

## Decision

Do not prepare or label an Online-ready package and do not hand this Candidate
to the external Judge Owner. The official submission identity audit is
preserved, but it does not repair the incomplete D40960 correctness domain or
the poor/one-of-three local performance direction. No fresh Candidate timing
was started because the fresh Parent same-binary floor did not provide a
stable comparison window.
