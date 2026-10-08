# R-W4-4 V082 Partial Ranking Result

- `REVISION=V082`; comparison Parent is exact `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `58da38c602a16acf0358c0f07e90aa8913deea562a3d389a781482cd344ab638`.
- Single threshold OFAT: `kSmallFp32BatchMaxWidth`, `4096 -> 7680`.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact-Parent C15 FP32 `1x32768`; C15 was excluded and not rerun.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route build and reference-probe build passed (`ROUTE_BUILD_RC=0`, `REF_PROBE_BUILD_RC=0`). Correctness used the established C++ include/runtime paths and fixed device 4.

| FP32 shape | Exact Parent | V082 Candidate | Candidate bad / max abs | Local |
|---|---|---|---:|---|
| `128x7672` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `981306 / 4.78556` | not run |
| `128x7680` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `982222 / 4.49627` | not run |
| `128x7688` | PASS, `rc=0`, `bad=0` | PASS, `rc=0`, `bad=0` | `0 / 1.90735e-06` | control-only Local below |

## Partial Local

Host `hwnput3`, fixed device 4, `128x7688` FP32. This width is above the V082 cutoff and uses the unchanged control path. Six interleaved P/C pairs used warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0, order `P,C; C,P; P,C; C,P; P,C; C,P`. Each side retained 62 device-event samples per pair (372/side; 744 total). Every invocation returned `rc=0,bad=0`; all samples were retained.

For each pair, `pair_speedup = Parent_median / Candidate_median` over all 62 raw `device_us` samples. The one-shape partial score is the arithmetic mean of six pair speedups; `delta_pct = (score - 1) * 100`. Pair values are in `v082-local-paired-summary.tsv`.

- `PARTIAL_ROUTE_LOCAL_SCORE=0.995391008935x`; `PARTIAL_ROUTE_LOCAL_DELTA=-0.4608991065%` (one unchanged control shape only).
- Candidate was faster in 2/6 pairs and slower in 4/6. Every pair's median difference is within the sum of its Parent and Candidate MADs.
- Pooled raw `device_us` summary (372 samples/side; unscaled MAD; P10/P90 linear interpolation at `p*(n-1)`; CV uses population standard deviation / mean):

  | Metric | Parent | Candidate |
  |---|---:|---:|
  | Mean | `21.421566` | `21.541130` |
  | Median | `21.370150` | `21.453600` |
  | MAD | `1.095300` | `0.973750` |
  | P10 | `19.341390` | `19.388000` |
  | P90 | `23.502080` | `23.721780` |
  | Min / max | `12.208700 / 65.609700` | `15.574700 / 28.900600` |
  | CV | `14.05547452%` | `7.45974886%` |

- The small pair differences are inside combined MAD, with mixed pair direction and a retained Parent maximum of `65.6097 us`. `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`, `NEEDS_ONE_MORE_LOCAL=YES`; no sample was removed.
- Device 4 snapshot: AICore `64% -> 64%`; HBM `59193 -> 59195 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) was present before and after and left untouched.
- `DEVICE4_RESERVATION=RELEASED` at `2026-10-08T02:48:27Z`, after all correctness/Local raw files and post-Local snapshots were captured and no active V082 probe command remained.
- `CONTROL_ONLY=YES`; this Local does not measure the threshold-selected path.

## Result Flags

- `COMPILE=PASS`; reference probe build `PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: Parent passed all three tested widths; Candidate failed both threshold-path shapes and passed only the unchanged control.
- `PARTIAL_CORRECTNESS=YES`.
- `LOCAL_SCORE=0.995391008935x` (partial, control-only, noisy).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; V082 is not promoted. Continue the bounded threshold OFAT from exact `R31B-V011`; C15 remains excluded.
