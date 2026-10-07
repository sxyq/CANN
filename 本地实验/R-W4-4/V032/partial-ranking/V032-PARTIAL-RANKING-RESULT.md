# R-W4-4 V032 partial ranking result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`
- REVISION: `V032`
- DIRECT_PARENT: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- CANDIDATE_SOURCE_SHA256: `ceee5781d272fdbd1b80528c3301be8919fed418f56a63f326de16d144fcbaee`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4096 -> 256`; no other source delta.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for the previously observed route Parent C15 FP32 `1x32768` failure. C15 was not rerun; this is not evidence of a V032 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- ONLINE: `NOT_ELIGIBLE`.

## Build and correctness

The route `device` and `submission` targets compiled successfully. The isolated probe build also compiled and linked both exact Parent and Candidate probes. Complete logs are `v032-build.log` and `probe-build.log`.

Exact Parent and Candidate correctness both passed on local host `hwnput3`, device 4, FP32. Every invocation returned `rc=0`, `bad=0`.

| Shape | Parent max_abs | Candidate max_abs |
|---|---:|---:|
| `128x248` | `1.19209e-06` | `1.19209e-06` |
| `128x256` | `1.19209e-06` | `1.19209e-06` |
| `128x264` | `9.53674e-07` | `9.53674e-07` |

C15 was excluded from all V032 commands. The known route Parent failure is a scope limitation, not attributed to this Candidate.

## Local ranking

Measurements used device events with wall time retained secondarily, warmup 45, 31 samples per block, 2 blocks, batch 64, and six interleaved Parent/Candidate pairs per shape. Every invocation returned `rc=0`, `bad=0`. Each Parent and Candidate pair file has 62 device-event samples; raw samples, per-invocation stats/stdout, command order, and device snapshots are retained. No sample was discarded.

For every pair, `pair_speedup = median(62 Parent device_us) / median(62 Candidate device_us)`. Each shape aggregate is the arithmetic mean of its six pair speedups. The partial route-local score is the equal-weight geometric mean of the three shape aggregates; `delta_pct = (score - 1) * 100`.

| Shape | Mean paired speedup | Median paired delta (Candidate - Parent) | Delta range | Candidate faster/slower | Within P/C MAD sum |
|---|---:|---:|---:|---:|---:|
| `128x248` | `0.986741744856x` (`-1.325826%`) | `+0.06656250 us` | `-0.19156500 .. +0.39890500 us` | `1 / 5` | `6 / 6` |
| `128x256` | `1.001980384297x` (`+0.198038%`) | `-0.01101250 us` | `-0.28656500 .. +0.19188000 us` | `3 / 3` | `6 / 6` |
| `128x264` | `0.989344021474x` (`-1.065598%`) | `+0.05937500 us` | `-0.30094000 .. +0.54640500 us` | `3 / 3` | `6 / 6` |

- PARTIAL_ROUTE_LOCAL_SCORE: `0.992666467680x`.
- PARTIAL_ROUTE_LOCAL_DELTA: `-0.73335323%`.
- LOAD_QUALITY: `NOISY`.
- MEASUREMENT_QUALITY: `NOISY`.
- NEEDS_ONE_MORE_LOCAL: `YES`.
- All 18 paired deltas fall within their measured P/C MAD sums and directions are mixed. Combined Parent/Candidate CVs were `11.081% / 16.291%` (`128x248`), `12.692% / 45.295%` (`128x256`), and `12.754% / 11.045%` (`128x264`). Candidate `128x256` contains a retained `80.8384 us` sample (372 total samples); it was not removed. This score is numeric but inconclusive.

Device 4 snapshots show HBM usage `64885/65536 MB` before and `64886/65536 MB` after. AICore changed from `0%` to `8%`; a pre-existing Python process on device 4 grew from about `1.4 GB` to `5.7 GB` resident. Existing VLLM PID `2999855` and all other processes were left untouched. These load facts reinforce the noisy classification; they do not justify filtering samples.

## Result

- COMPILE: `PASS`.
- CORRECTNESS: `PASS` on the three jointly tested partial shapes; `PARTIAL_CORRECTNESS=YES` because C15 was not rerun.
- LOCAL_SCORE: `0.992666467680x` (partial route-local speedup geomean only).
- LOCAL_DELTA: `-0.73335323%`.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`.
- CURRENT_LOCAL_BEST: `NONE`; V029, V030, V031, and V032 are not promoted.
- NEXT_PARENT: use exact R31B-V011 sibling source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` unless a verified Candidate Local Best is established.
- OFFICIAL_COMPARISON: `NO`; this is not an Official candidate or score.
