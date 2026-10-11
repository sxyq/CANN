# R1 canonical Local summary

All three declared R1 revisions were measured with the published MAIN2-CANONICAL-V1 harness and scored locally with the published pure `scoreVectors` function. No Main2 scoreboard/controller was run or modified.

| Revision | Source SHA | Canonical score | Delta vs anchor | Paired diagnostic | Quality/status | Promotion |
|---|---|---:|---:|---:|---|---|
| V001 | `3698c229dfb6a4b336e7ee0506eb1ceaf898c1c37c937c0af7d3eef71fbc685d` | 58.935569 | -41.064431 | 88.248022 | POOR / LOCAL_NEUTRAL | NO |
| V002 | `426489bfdc63441f2fd179fdff176875b24d815e6e0420343aeed0b0dab7933f` | 72.153409 | -27.846591 | 130.700485 | POOR / LOCAL_NEUTRAL | NO |
| V003 | `c6d8da88a9ba03e6c730733c52bed051b54ebc94629196636ea415599b96eb8c` | 75.778905 | -24.221095 | 156.141781 | POOR / LOCAL_NEUTRAL | NO |

All versions had Build PASS and passed the prior 7-case correctness preflight (7/7, RC=0, bad=0). V001 and V002 now have 24 successful paired raw files each; V003 already had 24. Each scored case used 45 warmups and 31 device-event samples per side, with P-C/C-P/P-C interleave. Every version retains per-side raw, stats, RC, identity, and HBM/AICore/process snapshots. V001/V002 NPU7 HBM stayed above 40 GB free; external PID 439848 (`python3`, about 21,260 MB) was present in all 48 snapshots per version and is recorded, not treated as a stop condition. V001 also retains its initial no-environment Parent RC=127 failure (`libgraph.so` missing); that failure was not used in scoring.

The score is a route-local diagnostic, not an Official result and not a Local Best. The common observed NPU7 external PID 439848 and broad run dispersion make all three measurements `QUALITY=POOR`; no stable gain is claimed.
