# R-W4-4 V031 partial ranking result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`
- REVISION: `V031`
- DIRECT_PARENT: exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- CANDIDATE_SOURCE_SHA256: `16a7778ee5dc27e251bfd820da0f5b45111c68079faa584d43ab151f42d712e5`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4096 -> 192`; no other source delta.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for the route's previously observed exact V027 Parent C15 FP32 `1x32768` failure. C15 was not rerun; this is not evidence of a V031 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- ONLINE: `NOT_ELIGIBLE`.

## Build and correctness

The route `device` and `submission` targets passed. The first environment initialization attempt failed because `set_env.sh` reads unset `LD_LIBRARY_PATH` under `set -u`; that log is retained, and the retry passed. The isolated Parent/Candidate probe build also retains two setup failures (missing unused CMake wrapper sources, then CCE host plugin `vector` include); retry with the existing HCC C++ include path built both probes successfully. See `v031-build*.log` and `probe-build*.log`.

Exact Parent and Candidate correctness both passed on local host `hwnput3`, device 4, FP32. Each invocation returned `rc=0`, `bad=0`.

| Shape | Parent max_abs | Candidate max_abs |
|---|---:|---:|
| `128x136` | `9.53674e-07` | `9.53674e-07` |
| `128x192` | `9.53674e-07` | `9.53674e-07` |
| `128x200` | `7.15256e-07` | `7.15256e-07` |

C15 was excluded from all V031 commands. The exact route Parent failure is carried forward as a scope limitation, not attributed to this Candidate.

## Local ranking

Measurements used device events with wall time retained secondarily, warmup 45, 31 samples per block, 2 blocks, batch 64, and six interleaved Parent/Candidate pairs per shape. Every invocation returned `rc=0`, `bad=0`. Each Parent and Candidate pair file has 62 device-event samples; all raw samples, per-invocation stats/stdout, command order, and device snapshots are retained. No sample was discarded.

For every pair, `pair_speedup = median(62 Parent device_us) / median(62 Candidate device_us)`. Each shape aggregate is the arithmetic mean of its six pair speedups. The partial route-local score is the equal-weight geometric mean of the three shape aggregates; `delta_pct = (score - 1) * 100`.

| Shape | Mean paired speedup | Median paired delta (Candidate - Parent) | Delta range | Candidate faster/slower | Within P/C MAD sum |
|---|---:|---:|---:|---:|---:|
| `128x136` | `0.9838604581x` (`-1.613954%`) | `+0.19086250 us` | `-0.42703000 .. +0.48047000 us` | `1 / 5` | `6 / 6` |
| `128x192` | `0.9956407491x` (`-0.435925%`) | `+0.17891000 us` | `-0.74922000 .. +0.39000000 us` | `2 / 4` | `6 / 6` |
| `128x200` | `0.9987133755x` (`-0.128662%`) | `-0.00945250 us` | `-0.12984500 .. +0.25922000 us` | `3 / 3` | `6 / 6` |

- PARTIAL_ROUTE_LOCAL_SCORE: `0.992717501551x`.
- PARTIAL_ROUTE_LOCAL_DELTA: `-0.72824984%`.
- LOAD_QUALITY: `NOISY`.
- MEASUREMENT_QUALITY: `NOISY`.
- NEEDS_ONE_MORE_LOCAL: `YES`.
- Per-shape combined Parent/Candidate CVs were `17.453% / 10.883%` (`128x136`), `12.903% / 11.536%` (`128x192`), and `11.055% / 9.857%` (`128x200`). The `128x136` Parent distribution had a `26.0241 us` maximum. All paired deltas remain within the measured P/C MAD sums and directions are mixed; retain the numeric score but treat it as inconclusive.

Device 4 snapshots show AICore `0%` and HBM usage `59192/65536 MB` before and after. Existing PID `2999855` was untouched. The snapshots do not establish load changes on other devices as a cause of device-4 samples.

## Result

- COMPILE: `PASS`.
- CORRECTNESS: `PASS` on the three jointly tested partial shapes; `PARTIAL_CORRECTNESS=YES` because C15 was not rerun.
- LOCAL_SCORE: `0.992717501551x` (partial route-local speedup geomean only).
- LOCAL_DELTA: `-0.72824984%`.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`.
- CURRENT_LOCAL_BEST: `NONE`; V029, V030, and V031 are not promoted.
- NEXT_PARENT: use exact R31B-V011 sibling source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` unless a verified Candidate Local Best is established.
- OFFICIAL_COMPARISON: `NO`; this is not an Official candidate or score.
