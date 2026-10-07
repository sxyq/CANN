# R-W4-4 V033 partial ranking result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`
- REVISION: `V033`
- DIRECT_PARENT: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- CANDIDATE_SOURCE_SHA256: `edb580f3f0fb1d4b654e90188bbe29811b2f3ea4b54bbc141c47fb8897bdc556`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4096 -> 128`; no other source delta.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for the previously observed route Parent C15 FP32 `1x32768` failure. C15 was not rerun; this is not evidence of a V033 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- ONLINE: `NOT_ELIGIBLE`.

## Build and correctness

The route `device` and `submission` targets compiled successfully. The isolated probe build compiled and linked both exact Parent and Candidate probes. Complete logs are `v033-build.log` and `probe-build.log`.

Exact Parent and Candidate correctness both passed on local host `hwnput3`, device 4, FP32. Every invocation returned `rc=0`, `bad=0`.

| Shape | Parent max_abs | Candidate max_abs |
|---|---:|---:|
| `128x120` | `9.53674e-07` | `9.53674e-07` |
| `128x128` | `7.15256e-07` | `7.15256e-07` |
| `128x136` | `9.53674e-07` | `9.53674e-07` |

C15 was excluded from every V033 command. The known route Parent failure is a scope limitation, not attributed to this Candidate.

## Local ranking

Measurements used device events with wall time retained secondarily, warmup 45, 31 samples per block, 2 blocks, batch 64, and six interleaved Parent/Candidate pairs per shape. Every invocation returned `rc=0`, `bad=0`. Each Parent and Candidate pair file has 62 device-event samples; raw samples, per-invocation stats/stdout, command order, and device snapshots are retained. No sample was discarded.

For every pair, `pair_speedup = median(62 Parent device_us) / median(62 Candidate device_us)`. Each shape aggregate is the arithmetic mean of its six pair speedups. The partial route-local score is the equal-weight geometric mean of the three shape aggregates; `delta_pct = (score - 1) * 100`.

| Shape | Mean paired speedup | Median paired delta (Candidate - Parent) | Delta range | Candidate faster/slower | Within P/C MAD sum |
|---|---:|---:|---:|---:|---:|
| `128x120` | `1.013635962626x` (`+1.363596%`) | `-0.05625000 us` | `-0.79093500 .. +0.20765500 us` | `4 / 2` | `6 / 6` |
| `128x128` | `0.967368082832x` (`-3.263192%`) | `+0.09375000 us` | `-0.28953000 .. +1.03125000 us` | `2 / 4` | `6 / 6` |
| `128x136` | `0.984695665813x` (`-1.530433%`) | `+0.07898250 us` | `-0.65187000 .. +0.99312500 us` | `2 / 4` | `6 / 6` |

- PARTIAL_ROUTE_LOCAL_SCORE: `0.988382992404x`.
- PARTIAL_ROUTE_LOCAL_DELTA: `-1.16170076%`.
- LOAD_QUALITY: `NOISY`.
- MEASUREMENT_QUALITY: `NOISY`.
- NEEDS_ONE_MORE_LOCAL: `YES`.
- All 18 paired deltas fall within their measured P/C MAD sums and directions are mixed. Combined Parent/Candidate CVs were `14.373% / 44.126%` (`128x120`), `12.562% / 14.770%` (`128x128`), and `43.627% / 13.094%` (`128x136`). Candidate `128x120` has a retained `82.1075 us` sample, and Parent `128x136` has a retained `59.8972 us` sample (372 samples per side and shape); no samples were removed. This score is numeric but inconclusive.

Device 4 snapshots show HBM usage `64885/65536 MB` before and `64887/65536 MB` after. AICore changed from `16%` to `0%`; pre-existing Python and VLLM processes were left untouched. These load facts reinforce the noisy classification and do not justify filtering samples.

## Result

- COMPILE: `PASS`.
- CORRECTNESS: `PASS` on the three jointly tested partial shapes; `PARTIAL_CORRECTNESS=YES` because C15 was not rerun.
- LOCAL_SCORE: `0.988382992404x` (partial route-local speedup geomean only).
- LOCAL_DELTA: `-1.16170076%`.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`.
- CURRENT_LOCAL_BEST: `NONE`; V029, V030, V031, V032, and V033 are not promoted.
- NEXT_PARENT: use exact R31B-V011 sibling source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` unless a verified Candidate Local Best is established.
- OFFICIAL_COMPARISON: `NO`; this is not an Official candidate or score.
