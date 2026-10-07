# R-W4-4 V030 partial ranking result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`
- REVISION: `V030`
- DIRECT_PARENT: V029 exact source SHA256 `9e9a4a17e0079b77e8f9b41f26e57653e70128081da27c5ffff0b064375bb978`
- CANDIDATE_SOURCE_SHA256: `ff5543d4b4748953ad9f43880f9f3d21d562771440f6e2cb514ab11ce099f168`
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `128 -> 192`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact V027 Parent C15 FP32 `1x32768`; do not rerun and do not classify it as a V030 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- ONLINE: `NOT_ELIGIBLE`.

## Build

The local route Candidate build passed for `device` and `submission` (exit 0). The isolated Parent/Candidate reference probe build also passed (exit 0). Logs are `v030-build.log` and `partial-ranking/probe-build.log`. Source and executable SHA256 values are in `partial-ranking/identity.sha256`.

## Correctness

The first correctness command was a tooling invocation error: it constructed executable names `clx_ref_ref_parent_probe` and `clx_ref_ref_candidate_probe`, which do not exist. All six failed attempts returned `rc=127` before device launch; they are retained in `correctness.log` and are not correctness failures. The corrected retry used the exact direct V029 Parent and V030 Candidate binaries. Both passed all three FP32 widths on local device 4 (`rc=0`, `bad=0`):

| Shape | Parent max_abs | Candidate max_abs |
|---|---:|---:|
| `128x136` | `9.53674e-07` | `9.53674e-07` |
| `128x192` | `9.53674e-07` | `9.53674e-07` |
| `128x200` | `7.15256e-07` | `7.15256e-07` |

The first two widths enter the V030 cutoff path; `128x200` is the just-outside control. No C15 case was included. Corrected command order and outputs are in `correctness-retry1.log`; per-invocation stats are in the `*-correctness-*-retry1-stats.txt` files.

## Local ranking

Measurements ran on local host `hwnput3`, device 4, using device events, warmup 45, 31 samples per block, 2 blocks, batch 64, and six interleaved P/C pairs for each shape. Parent and Candidate returned `rc=0`, `bad=0` for every invocation. Raw samples, per-run stats and ordering logs are retained. Negative delta means Candidate was faster.

| Shape | Parent same-binary median / MAD / CV | Candidate faster / slower pairs | Median paired delta | Range | Noise classification |
|---|---:|---:|---:|---:|---|
| `128x136` | `8.70469 us / 0.595313 us / 11.201%` | `2 / 4` | `+0.206640 us` | `-0.986870 .. +0.330780 us` | All six deltas are within paired P/C MAD sums; mixed, no repeatable gain |
| `128x192` | `8.46797 us / 0.522031 us / 8.871%` | `3 / 3` | `+0.009220 us` | `-0.461570 .. +0.651250 us` | All six deltas are within paired P/C MAD sums; mixed, noise-level |
| `128x200` | `8.48078 us / 0.562344 us / 15.261%` | `2 / 4` | `+0.069685 us` | `-0.091410 .. +0.475310 us` | All six deltas are within paired P/C MAD sums; mixed, Parent control has high spread and a `15.9219 us` max |

Per-pair medians, percentages, MADs and p90 values are in `batch64-w136-paired-summary.tsv`, `batch64-w192-paired-summary.tsv`, and `batch64-w200-paired-summary.tsv`. Every raw sample remains; no outlier was removed. V029's six-shape numeric/noise review is in `../V029/partial-ranking/V029-PARTIAL-RANKING-RESULT.md`.

### Numeric partial route-local receipt

The score is recomputed from the retained raw `device_us` samples, not from rounded table values. Each paired Parent and Candidate raw file has 62 samples (2 blocks x 31). For each pair, take the median of its 62 Parent samples and Candidate samples, then `pair_speedup = Parent_median / Candidate_median`. For each shape, take the arithmetic mean of its six paired speedups. The partial route-local score is the equal-weight geometric mean of the three shape aggregates; `delta_pct = (score - 1) * 100`.

| Shape | Mean of six paired speedups | Shape delta |
|---|---:|---:|
| `128x136` | `1.0035155266x` | `+0.351553%` |
| `128x192` | `0.9905325864x` | `-0.946741%` |
| `128x200` | `0.9854798739x` | `-1.452013%` |

- PARTIAL_ROUTE_LOCAL_SCORE: `0.993147013644x`.
- PARTIAL_ROUTE_LOCAL_DELTA: `-0.68529864%`.
- LOAD_QUALITY: `NOISY`.
- MEASUREMENT_QUALITY: `NOISY`.
- NEEDS_ONE_MORE_LOCAL: `YES`.
- The numeric receipt includes all six pairs for each shape; no sample or unfavorable direction was excluded. This is only a partial local ranking, not an Official-comparable score.

Device 4 pre/post snapshots show HBM usage 90% (conservative FREE_HBM lower bound 5898 MB), AICore 0%, AIVector 0%. Existing VLLM PID `2999855` was left untouched, and no route probe remained after the run. The device-4 snapshot does not support interpreting noise on other devices as a load change on device 4.

## Result

- COMPILE: `PASS`.
- CORRECTNESS: `PASS` on three jointly tested partial FP32 shapes; `PARTIAL_CORRECTNESS=YES` because the known exact-Parent C15 baseline still fails and was not repeated.
- LOCAL_SCORE: `0.993147013644x` (partial route-local speedup geomean; not Official-comparable).
- LOCAL_DELTA: `-0.68529864%`.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY` (all paired deltas are within measured MAD envelopes; direction is mixed).
- CURRENT_LOCAL_BEST: `NONE`; no verified `LOCAL_ACCEPTED` revision exists in the current route evidence. Neither V029 nor V030 is promoted.
- NEXT_PARENT: use the exact R31B-V011 sibling baseline, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` (`线上结果/R31B/V011/submission.asc`), not V029 or V030. The retained V011 sidecar and champion record match this source SHA.
- OFFICIAL_COMPARISON: `NO`; this partial ranking is not an Official candidate or score.
