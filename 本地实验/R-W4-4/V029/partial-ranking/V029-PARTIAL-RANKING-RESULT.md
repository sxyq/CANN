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

## Partial local ranking

Device-event measurements used identical Parent/Candidate runners, warmup, shape, dtype, device, and alternating P/C order. Raw `*-raw.tsv` files and per-run stats are retained. `paired-summary.tsv` records the first four pairs at `128x128` and `128x256` (`batch_n=1`); `width-confirm-summary.tsv` records two supplemental pairs at `128x136`, `128x248`, and `128x256` (`batch_n=16`). `interleaved-extended-local-batch16.log` and `interleaved-width-confirm-batch16.log` preserve exact command order and return codes.

The direction is not repeatable. At `128x136`, three initial batch-16 pairs favored Candidate by `5.17`, `6.26`, and `14.51 us`, while the two supplemental pairs favored Parent by `2.87` and `4.88 us`. At `128x248`, initial and supplemental pairs are mixed. At `128x256`, batch-16 confirmation also changes direction across two pairs. The unbatched `128x128` and `128x256` pairs contain large latency outliers. Parent same-binary block medians and spread are preserved in `parent-samebinary-*`; rechecks did not establish a consistently quiet distribution. No samples were removed or trimmed.

Device 4 snapshots show 90% HBM usage (conservative FREE_HBM lower bound `5898 MB`), AICore `0%`, AIVector `0%`, and existing VLLM PID `2999855`; that process was untouched. Before/after device snapshots and all run logs are preserved. The broad pre-correctness process dump had unrelated VS Code `--connection-token` command-line values redacted before commit; the NPU process table is retained. The mixed deltas and unstable Parent distribution do not support a numeric Local score or delta.

## Result and next action

- COMPILE: `PASS` after environment-only include-path support fix; original failure retained.
- CORRECTNESS: `PASS` on six jointly tested partial shapes; `PARTIAL_CORRECTNESS=YES` because the known exact-Parent C15 baseline remains failing and was not rerun.
- LOCAL_SCORE: `NONE`
- LOCAL_DELTA: `NONE`
- LOCAL_VERDICT: `NEEDS_ONE_MORE_LOCAL` (direction mixed and Parent same-binary distribution noisy).
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`; this is not an Official candidate.
- V030: `BLOCKED` until V029 has a comparable, repeatable local ranking; keep the current Parent/Candidate and evidence unchanged.
- NEXT_ACTION: same-Candidate Parent requalification and interleaved local sampling on the jointly passing cutoff shapes, with every raw sample retained. Do not rerun C15, use the repaired Parent, edit Candidate/source, write shared records, push, or submit Online.
