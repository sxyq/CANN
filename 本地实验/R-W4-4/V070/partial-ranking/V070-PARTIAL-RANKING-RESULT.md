# R-W4-4 V070 Partial Ranking Result

- `REVISION=V070`; exact comparison Parent is `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source SHA256: `58da38c602a16acf0358c0f07e90aa8913deea562a3d389a781482cd344ab638`.
- Single OFAT change: `kSmallFp32BatchMaxWidth`, `4096 -> 7680`.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact-Parent C15 FP32 `1x32768`; not rerun and not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route build and isolated Parent/Candidate probe builds passed (`ROUTE_BUILD_RC=0`, `PROBE_BUILD_RC=0`). The isolated probes used the exact V011 Parent. C15 was excluded.

| FP32 shape | Exact Parent | V070 Candidate | Candidate bad | Local |
|---|---|---|---:|---|
| `128x7672` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `981316` | not run |
| `128x7680` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `982223` | not run |
| `128x7688` | PASS, `rc=0`, `bad=0` | PASS, `rc=0` | `0` | control-only Local below |

Correctness raw files, identities, build logs, and device/process snapshots are retained alongside the Local evidence.

## Partial Local

Host `hwnput3`, fixed device 4, `128x7688` FP32. This width is above the V070 cutoff and exercises the unchanged control path. Six interleaved P/C pairs were run with warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0. Each side retained 62 device-event samples per pair (372/side total); all invocations returned `rc=0,bad=0`. No samples or pairs were dropped.

For each pair, `pair_speedup = Parent_median / Candidate_median`, using all 62 raw `device_us` values. The one-shape partial score is the arithmetic mean of the six pair speedups; `delta_pct = (score - 1) * 100`. Per-pair values are in `v070-local-paired-summary.tsv`.

- `PARTIAL_ROUTE_LOCAL_SCORE=0.987918611166x`; `PARTIAL_ROUTE_LOCAL_DELTA=-1.2081388834%` (one unchanged control shape only).
- The median-based pair ratios used for the score favor Candidate in 2/6 pairs and Parent in 4/6. By per-pair arithmetic means, direction is mixed 3/6 each. All six median-based paired deltas are within the corresponding Parent+Candidate MAD sum.
- Pooled raw device-event CV: Parent `11.4756239%`, Candidate `14.6294008%`; `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`, `NEEDS_ONE_MORE_LOCAL=YES`.
- Device 4 snapshot: AICore `61% -> 58%`; HBM `59192 -> 59194 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) remained untouched.
- `CONTROL_ONLY=YES`; this result does not rank or promote V070.

## Result Flags

- `COMPILE=PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: Candidate failed both tested threshold-path shapes and passed only the unchanged control; exact Parent passed all three.
- `PARTIAL_CORRECTNESS=YES`.
- `LOCAL_SCORE=0.987918611166x` (partial, control-only, noisy).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; V070 is not promoted. Use exact `R31B-V011` as the Parent for the next threshold OFAT.
