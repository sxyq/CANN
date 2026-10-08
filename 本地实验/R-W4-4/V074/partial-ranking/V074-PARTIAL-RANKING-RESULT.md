# R-W4-4 V074 Partial Ranking Result

- `REVISION=V074`; exact comparison Parent is `R31B-V011`, source SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate source SHA256: `d3a471ccd4e82bb1244ecb0b991f693d52e5e674cd2def36a19f4b62a34d8592`.
- Single OFAT change: `kSmallFp32BatchMaxWidth`, `4096 -> 8192`; the Candidate differs from exact V011 only at this constant.
- `PARENT_KNOWN_CORRECTNESS_FAILURE=YES` for exact-Parent C15 FP32 `1x32768`; excluded and not a Candidate regression.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Compile and Correctness

Route build passed (`ROUTE_BUILD_RC=0`) and the `clx_ref_parent_probe` / `clx_ref_candidate_probe` reference-harness targets built successfully (`REF_PROBE_BUILD_RC=0`). Correctness was run on fixed device 4 for only the authorized FP32 shapes; C15 was not run.

| FP32 shape | Exact Parent | V074 Candidate | Candidate bad | Local |
|---|---|---|---:|---|
| `128x8184` | PASS, `rc=0`, `bad=0` | FAIL, `rc=3` | `1046888` | not run |
| `128x8192` | PASS, `rc=0`, `bad=0` | PASS, `rc=0` | `0` | Local below |
| `128x8200` | FAIL, `rc=3` | FAIL, `rc=3` | `525571` (Parent bad `521206`) | not run |

The 8200 result is an exact Parent failure, not a Candidate regression. Local was run only on the jointly passing 8192 shape.

## Partial Local

Host `hwnput3`, fixed device 4, `128x8192` FP32 on the threshold-selected path. Six interleaved P/C pairs used warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0. Each side retained 62 device-event samples per pair (372/side; 744 total); all invocations returned `rc=0,bad=0`. All samples were retained.

For each pair, `pair_speedup = Parent_median / Candidate_median`, using all 62 raw `device_us` values. The one-shape partial score is the arithmetic mean of the six pair speedups; `delta_pct = (score - 1) * 100`. Per-pair values are in `v074-local-paired-summary.tsv`.

- `PARTIAL_ROUTE_LOCAL_SCORE=1.004435421982x`; `PARTIAL_ROUTE_LOCAL_DELTA=+0.4435421982%` (one jointly-correct shape).
- Median-based direction: Candidate faster in 3/6 pairs and slower in 3/6. All six paired median deltas are within the corresponding Parent+Candidate MAD sum.
- Pooled raw `device_us` summary (372 samples/side; unscaled MAD; P10/P90 nearest-rank; CV uses population standard deviation / mean):

  | Metric | Parent | Candidate |
  |---|---:|---:|
  | Median | `20.690800` | `20.388400` |
  | MAD | `2.752050` | `2.524700` |
  | P10 | `17.564700` | `17.655000` |
  | P90 | `43.535900` | `43.685300` |
  | Min / max | `11.010900 / 151.047000` | `14.324100 / 157.731000` |
  | CV | `78.44684312%` | `83.29540044%` |

- Both pooled distributions are highly noisy; maxima are retained and no sample was classified as a protocol failure or removed. `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`, `NEEDS_ONE_MORE_LOCAL=YES`.
- Device 4 snapshot: AICore `68% -> 66%`; HBM `59193 -> 59195 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) was present before and after, untouched.
- `CONTROL_ONLY=NO` for shape 8192, but this single noisy shape is only a partial local score and is not comparable to Official.

## Result Flags

- `COMPILE=PASS`; reference probe build `PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: Candidate passed the threshold shape, failed 8184, and shared the Parent failure at 8200.
- `PARTIAL_CORRECTNESS=YES`.
- `LOCAL_SCORE=1.004435421982x` (partial, noisy; not a promotion).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; `NEEDS_ONE_MORE_LOCAL=YES`. Do not promote V074 from this noisy single-shape result; use exact `R31B-V011` as Parent for the next threshold OFAT.
