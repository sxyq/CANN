# R-W4-4 V029 partial ranking result

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`
- REVISION: `V029`
- DIRECT_PARENT: `V028`, source SHA256 `06384465fe0e3831048fc15903807ec3aeae55cdcb1852f5d85d60fbb9154ffb`
- CANDIDATE: source SHA256 `9e9a4a17e0079b77e8f9b41f26e57653e70128081da27c5ffff0b064375bb978`
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `256 -> 128`
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` (exact V027 Parent SHA256 `f0ab43545e13d943e4c5bb5ff22400426e18f560ab313c1c6856b39fa904f033`, FP32 `1x32768`, C15). Do not repeat. This is a Parent baseline failure, not a V029 Candidate regression.
- PARTIAL_CORRECTNESS: `YES`
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`
- ONLINE: `NOT_ELIGIBLE`

## Build

The first isolated build failed before device launch with `fatal error: 'vector' file not found`; the full original log is `cmake-build.log`. Sourcing the installed CANN environment and exporting the four existing HCC C++ include directories fixed the host plugin compile without editing Candidate or harness source. The retry built `clx_ref_parent_probe` and `clx_ref_candidate_probe` successfully (exit 0); see `cmake-build-retry-include-path.log`.

Probe executable SHA256 values are `8e14fe999de9646567f9ad28ef9a14c72e5fd678d3d3ee45bf1a03d0c0301eef` (Parent) and `e8a747a7a5e93f4a9ead55ce811a8060fe6301acc2184e521686fdda5d6649e7` (Candidate). Source, wrapper, and runner hashes are preserved in `identity.sha256` and `executable-identity.txt`.

## Correctness

The exact Parent and V029 Candidate each passed the six route-local FP32 shapes below on local device 4. Every invocation returned `rc=0`, `bad=0`.

| Shape | Parent max_abs | Candidate max_abs | Cutoff relevance |
|---|---:|---:|---|
| `128x128` | `7.15256e-07` | `7.15256e-07` | unchanged control, at Candidate cutoff |
| `128x256` | `1.19209e-06` | `1.19209e-06` | inclusive Parent upper boundary |
| `32x136` | `9.53674e-07` | `9.53674e-07` | row-count control for the `localRows > 1` dispatch condition |
| `64x136` | `9.53674e-07` | `9.53674e-07` | row-group activation case |
| `128x136` | `9.53674e-07` | `9.53674e-07` | first aligned width above Candidate cutoff |
| `128x248` | `1.19209e-06` | `1.19209e-06` | upper part of the changed dispatch interval |

Raw correctness outputs, stats, stdout, and invocation order are in the shape-specific `*-r*-w*-fp32` files, `extended-cutoff-correctness.log`, and the initial `parent-r128-w128-fp32*` / `candidate-r128-w128-fp32*` and `parent-r128-w256-fp32*` / `candidate-r128-w256-fp32*` files. C15 was excluded from every V029 command.

## Per-shape Parent-valid review

The exact V028 Parent and V029 Candidate both passed all six listed FP32 cases (`rc=0`, `bad=0`); these are the complete Parent-valid shape set used for this partial ranking. Delta is Candidate minus Parent, so negative favors Candidate. Timing configurations are kept separate because batching changed the observed spread.

| Shape | Parent/Candidate correctness max_abs | Local measurements and direction | Noise classification |
|---|---:|---|---|
| `128x128` | `7.15256e-07 / 7.15256e-07` | `batch_n=1`, 4 pairs: median delta `+0.445 us`, range `-1.370 .. +25.280 us`; Candidate faster/slower `1/3` | Parent same-binary CV `135.2%`, max `129.7 us`; severe outliers, not rankable |
| `128x256` | `1.19209e-06 / 1.19209e-06` | `batch_n=1`, 4 pairs: median `-8.300 us`, range `-43.900 .. +7.850` (`3/1` faster/slower); `batch_n=16`, 2 pairs: `+1.714 us`, range `-2.847 .. +6.276` (`1/1`); `batch_n=64`, 6 pairs: `-0.211 us`, range `-0.886 .. +0.808` (`4/2`) | Parent same-binary CV `191.97%` (`n=1`) and `36.0%` (`batch_n=16`); batch64 CV `10.39%`; direction depends on batch regime, inconclusive |
| `32x136` | `9.53674e-07 / 9.53674e-07` | `batch_n=16`, 3 pairs: median delta `+0.308 us`, range `-2.730 .. +2.741`; faster/slower `1/2` | Parent same-binary CV `61.64%`; pair spread dominates, noisy |
| `64x136` | `9.53674e-07 / 9.53674e-07` | `batch_n=16`, 3 pairs: median `+0.969 us`, range `-0.970 .. +4.979`; faster/slower `1/2` | Parent same-binary CV `40.04%`; pair spread dominates, noisy |
| `128x136` | `9.53674e-07 / 9.53674e-07` | `batch_n=16`, initial 3: median `-6.256 us`, range `-14.514 .. -5.174` (`3/0`); confirmation 2: median `+3.871 us`, range `+2.865 .. +4.878` (`0/2`); `batch_n=64`, 6: median `+0.083 us`, range `-1.598 .. +1.364` (`3/3`) | Parent same-binary CV `40.4%–57.8%` at batch16 and `12.90%` at batch64; direction reverses across runs, inconclusive |
| `128x248` | `1.19209e-06 / 1.19209e-06` | `batch_n=16`, initial 3: median `+4.955 us`, range `-12.193 .. +6.134` (`1/2`); confirmation 2: median `-2.268 us`, range `-3.592 .. -0.944` (`2/0`); `batch_n=64`, 6: median `+0.116 us`, range `-0.187 .. +0.448` (`2/4`) | Parent same-binary CV `27.5%–39.6%` at batch16 and `9.06%` at batch64; mixed direction and noisy |

Every raw sample remains in the shape-specific `*-raw.tsv`; no outlier was removed. No shape establishes a repeatable Candidate advantage. The per-pair batch64 values are in the batch64 summary TSVs below.

## Partial local ranking

Device-event measurements used identical Parent/Candidate runners, warmup, shape, dtype, device, and alternating P/C order. Raw `*-raw.tsv` files, per-run stats, and command-order logs are retained. `paired-summary.tsv` records the first four `batch_n=1` pairs; `width-confirm-summary.tsv` records supplemental `batch_n=16` pairs. The completed `batch_n=64` six-pair results are in `batch64-paired-summary.tsv`, `batch64-w248-paired-summary.tsv`, and `batch64-w256-paired-summary.tsv`.

For the completed batch64 set, each invocation used warmup 45 and 31 measured samples; every Parent and Candidate invocation returned `rc=0`, `bad=0`. Pair deltas below are Candidate minus Parent, so a negative value favors Candidate.

| Shape | Parent same-binary median / MAD / CV | Candidate faster / slower pairs | Median paired delta | Delta range | Classification |
|---|---:|---:|---:|---:|---|
| `128x136` | `8.54766 us / 0.754687 us / 12.895%` | `3 / 3` | `+0.082970 us` | `-1.597500 .. +1.364060 us` | Mixed; one pair exceeds the P/C MAD sum by `0.1053 us`; no stable direction |
| `128x248` | `9.17469 us / 0.556251 us / 9.056%` | `2 / 4` | `+0.116255 us` | `-0.186560 .. +0.448440 us` | Mixed; all pair deltas are within the P/C MAD sum |
| `128x256` | `8.02766 us / 0.510469 us / 10.388%` | `4 / 2` | `-0.211410 us` | `-0.886250 .. +0.807510 us` | Mixed; all pair deltas are within the P/C MAD sum |

Exact per-pair parent/candidate medians, deltas, MADs, and p90 values are in those TSVs. Same-binary controls show substantial spread (CV `9.1%` to `12.9%`); the small median deltas change sign by shape and do not establish a repeatable ranking. Earlier batch-16/individual samples are retained and untrimmed; no sample was removed or reclassified.

Device 4 pre/post snapshots show 90% HBM usage (conservative FREE_HBM lower bound `5898 MB`), AICore `0%`, AIVector `0%`, and the existing VLLM PID `2999855`; that process was untouched. The post-128x256 snapshot at `2026-10-07T12:09:09.066985104Z` also shows no route probe process. The broad pre-correctness process dump had unrelated VS Code `--connection-token` command-line values redacted before commit; the NPU process table is retained. These measurements do not support a numeric Local score or delta and are not Official-comparable.

## Result and next action

- COMPILE: `PASS` after environment-only include-path support fix; original failure retained.
- CORRECTNESS: `PASS` on six jointly tested partial shapes; `PARTIAL_CORRECTNESS=YES` because the known exact-Parent C15 baseline remains failing and was not rerun.
- LOCAL_SCORE: `NONE`
- LOCAL_DELTA: `NONE`
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL` (current interpretation is inconclusive: mixed direction and noisy Parent same-binary distributions).
- CURRENT_LOCAL_BEST: `NONE`; the route evidence contains no verified `LOCAL_ACCEPTED` revision. V029 is not a Local Best.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`; this is not an Official candidate.
- RANKING_GATE: `COMPLETE_FOR_SIX_TESTED_PARENT-VALID_SHAPES`; the result remains inconclusive and does not establish full correctness or an Official-comparable score.
- NEXT_ACTION: V030 has been started as the single threshold OFAT `128 -> 192`, with the exact V029 source as its direct Parent. Do not promote V029 automatically. For any later Parent selection, use only a verified Local Best; current value is `NONE`. Keep C15 excluded and retain partial-correctness/non-comparability flags.
