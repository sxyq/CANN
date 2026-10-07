# R-W4-4 V034 partial ranking result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`
- REVISION: `V034`
- DIRECT_PARENT: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- CANDIDATE_SOURCE_SHA256: `dcb2eca042fee260f6b7418623f32cd0c0f3c3a69cfbdcf3b778cb50ea99827b`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4096 -> 64`; no other source delta.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for the previously observed route Parent C15 FP32 `1x32768` failure. C15 was not rerun; this is not evidence of a V034 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- ONLINE: `NOT_ELIGIBLE`.

## Build and correctness

The route `device` and `submission` targets compiled successfully. The isolated probe build compiled and linked both exact Parent and Candidate probes. Complete logs are `v034-build.log` and `probe-build.log`.

Exact Parent and Candidate correctness both passed on local host `hwnput3`, device 4, FP32. Every invocation returned `rc=0`, `bad=0`.

| Shape | Parent max_abs | Candidate max_abs |
|---|---:|---:|
| `128x64` | `7.15256e-07` | `7.15256e-07` |
| `128x72` | `7.15256e-07` | `7.15256e-07` |
| `128x80` | `7.15256e-07` | `7.15256e-07` |

C15 was excluded from every V034 command. The known route Parent failure is a scope limitation, not attributed to this Candidate.

## Local ranking

Measurements used device events with wall time retained secondarily, warmup 45, 31 samples per block, 2 blocks, batch 64, and six interleaved Parent/Candidate pairs per shape. Every invocation returned `rc=0`, `bad=0`. Each Parent and Candidate pair file has 62 device-event samples; raw samples, per-invocation stats/stdout, command order, and device snapshots are retained. No sample was discarded.

For every pair, `pair_speedup = median(62 Parent device_us) / median(62 Candidate device_us)`. Each shape aggregate is the arithmetic mean of its six pair speedups. The partial route-local score is the equal-weight geometric mean of the three shape aggregates; `delta_pct = (score - 1) * 100`.

| Shape | Mean paired speedup | Median paired delta (Candidate - Parent) | Delta range | Candidate faster/slower | Within P/C MAD sum |
|---|---:|---:|---:|---:|---:|
| `128x64` | `0.929631801169x` (`-7.036820%`) | `+0.54476500 us` | `+0.21187500 .. +1.21781000 us` | `0 / 6` | `6 / 6` |
| `128x72` | `0.981151690074x` (`-1.884831%`) | `+0.20132750 us` | `-0.14703000 .. +0.44906000 us` | `2 / 4` | `6 / 6` |
| `128x80` | `0.992449103888x` (`-0.755090%`) | `+0.19039250 us` | `-0.48328000 .. +0.62547000 us` | `2 / 4` | `6 / 6` |

- PARTIAL_ROUTE_LOCAL_SCORE: `0.967353314406x`.
- PARTIAL_ROUTE_LOCAL_DELTA: `-3.26466856%`.
- LOAD_QUALITY: `NOISY`.
- MEASUREMENT_QUALITY: `NOISY`.
- NEEDS_ONE_MORE_LOCAL: `YES`.
- All 18 paired deltas fall within their measured P/C MAD sums. `128x64` is consistently slower across these six pairs, while the other shapes have mixed direction. Combined Parent/Candidate CVs were `12.300% / 14.215%` (`128x64`), `14.149% / 15.306%` (`128x72`), and `13.999% / 12.859%` (`128x80`). The numeric score is retained, but these spreads leave the result inconclusive.

Device 4 snapshots show HBM usage `64887/65536 MB` before and `64888/65536 MB` after. AICore changed from `8%` to `0%`; pre-existing Python and VLLM processes were left untouched. These load facts reinforce the noisy classification and do not justify filtering samples.

## Result

- COMPILE: `PASS`.
- CORRECTNESS: `PASS` on the three jointly tested partial shapes; `PARTIAL_CORRECTNESS=YES` because C15 was not rerun.
- LOCAL_SCORE: `0.967353314406x` (partial route-local speedup geomean only).
- LOCAL_DELTA: `-3.26466856%`.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`.
- CURRENT_LOCAL_BEST: `NONE`; V029, V030, V031, V032, V033, and V034 are not promoted.
- NEXT_PARENT: use exact R31B-V011 sibling source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` unless a verified Candidate Local Best is established.
- OFFICIAL_COMPARISON: `NO`; this is not an Official candidate or score.
