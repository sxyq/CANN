# R-W4-4 V046

- ROUTE: `R-W4-4 / MODE-DISPATCH-CUTOFF-X`.
- REVISION: `V046`.
- DIRECT_PARENT: exact sibling `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- SINGLE_CHANGE: `kSmallFp32BatchMaxWidth`, `4480 -> 4608`.
- CANDIDATE_SOURCE_SHA256: `7ee85626738e5bf792731103bad358862d521b262bb0c35b2dcd096400aec285`.
- PARENT_KNOWN_CORRECTNESS_FAILURE: `YES` for exact Parent C15 FP32 `1x32768`; do not rerun and do not conflate with any Candidate-only failure.
- PARTIAL_CORRECTNESS: `YES`; Candidate failed at widths 4600 and 4608, and Parent/Candidate both passed only at 4616.
- LOCAL_SCORE_COMPARABLE_TO_OFFICIAL: `NO`.
- CURRENT_LOCAL_BEST: `NONE`; prior noisy/partial revisions are not promoted.
- LOCAL: `0.965123947268x` at the jointly passing `128x4616` control shape, delta `-3.487605273%`; `NOISY`, `NEEDS_ONE_MORE_LOCAL`.
- OFFICIAL: `NOT_ELIGIBLE`; the Local result is partial and not comparable to Official.

## V046 Partial Ranking Result

- Compile: route `device`/`submission` and isolated `clx_ref_parent_probe`/`clx_ref_candidate_probe` targets passed. The first shell invocation failed before CMake because nounset exposed an unset `LD_LIBRARY_PATH`; the exact failure and successful environment retry are retained in `v046-compile.log`.
- Correctness used the exact staged Parent above on local host `hwnput3`, device 4, FP32. C15 was explicitly excluded because exact Parent C15 is already a known failure.

| Shape | Parent | Candidate | Candidate bad | Candidate max_abs | Local |
|---|---|---|---:|---:|---|
| `128x4600` | PASS, `bad=0` | FAIL, `rc=3` | `586818` | `4.85819` | not run |
| `128x4608` | PASS, `bad=0` | FAIL, `rc=3` | `588033` | `4.83657` | not run |
| `128x4616` | PASS, `bad=0` | PASS, `rc=0` | `0` | `3.57628e-06` | six interleaved pairs |

- Local used fixed device 4, FP32, warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0. Odd pairs ran Parent/Candidate and even pairs Candidate/Parent. All 12 invocations returned `rc=0`, `bad=0`; each side retained 372 device-event samples (six raw files x 62 events).
- For each pair, `speedup = median(62 Parent device_us) / median(62 Candidate device_us)`. The one eligible shape score is the arithmetic mean of the six pair speedups; since only one shape jointly passed, the partial route score is that shape score and is not a full-route score.

| Pair | Parent median us | Candidate median us | P/C MAD us | Speedup | Delta us | Within MAD sum |
|---:|---:|---:|---:|---:|---:|---|
| 1 | 17.2437 | 17.4277 | 1.61312 / 1.77813 | 0.989442095056x | -0.1840 | YES |
| 2 | 18.2300 | 18.3783 | 2.13672 / 1.61359 | 0.991930700881x | -0.1483 | YES |
| 3 | 16.6662 | 18.5906 | 1.28406 / 2.20922 | 0.896485320538x | -1.9244 | YES |
| 4 | 18.0305 | 17.7117 | 1.60437 / 1.99781 | 1.017999401526x | +0.3188 | YES |
| 5 | 17.5361 | 17.9661 | 1.75406 / 1.51125 | 0.976066035478x | -0.4300 | YES |
| 6 | 16.9175 | 18.4122 | 2.00422 / 2.07625 | 0.918820130131x | -1.4947 | YES |

- `PARTIAL_ROUTE_LOCAL_SCORE=0.965123947268x`; `PARTIAL_ROUTE_LOCAL_DELTA=-3.487605273%` on the sole eligible shape.
- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. Candidate was faster in 1/6 pairs; all 6/6 paired deltas were within the corresponding Parent-plus-Candidate MAD sum. Pooled raw CV was 15.694914% Parent and 17.049630% Candidate. The Candidate maximum event value was 53.9122 us (Parent maximum 24.4894 us); it remains included in the raw data.
- Width 4616 is greater than the 4608 cutoff, so this is an unchanged-path control and does not estimate the changed small-batch path. Its numeric Local result is retained as required but is not evidence of a threshold-path gain.
- Device 4 AICore load was 0% before correctness and 44% before Local (58% in the post-Local snapshot); HBM was 59192 MB before and 59194 MB after Local. Existing VLLM PID `2999855` remained at 55666 MB and was untouched.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `CURRENT_LOCAL_BEST=NONE`. V046 is not promoted.

Raw correctness, timing, stdout/stderr, run-order index, compile log, and device/process snapshots are retained under `partial-ranking/`. The staged harness sources are under `partial-ranking/stage/`; generated build directories are excluded from the evidence commit.
