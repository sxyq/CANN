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

Host `hwnput3`, fixed device 4, `128x8192` FP32 on the threshold-selected path. Two interleaved P/C batches of six pairs each used warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0. Each side retained 62 device-event samples per pair (744/side; 1488 total); all invocations returned `rc=0,bad=0`. All samples were retained. The second batch is recorded in `v074-local-recheck02.log`; pair rows 07-12 are in `v074-local-recheck02-paired-summary.tsv`.

For each pair, `pair_speedup = Parent_median / Candidate_median`, using all 62 raw `device_us` values. The one-shape partial score is the arithmetic mean of all 12 pair speedups; `delta_pct = (score - 1) * 100`. Pair rows 01-06 are in `v074-local-paired-summary.tsv`, and rows 07-12 are in `v074-local-recheck02-paired-summary.tsv`.

- First six-pair batch: `1.004435421982x` (`+0.4435421982%`). Second six-pair batch: `1.056752425081x` (`+5.6752425081%`). Combined 12-pair receipt: `PARTIAL_ROUTE_LOCAL_SCORE=1.030593923531x`; `PARTIAL_ROUTE_LOCAL_DELTA=+3.0593923531%` (one jointly-correct shape).
- Median-based direction over all pairs: Candidate faster in 7/12 pairs and slower in 5/12. Eleven of twelve paired median deltas are within the corresponding Parent+Candidate MAD sum; pair 07 is outside.
- Pooled raw `device_us` summary (744 samples/side; unscaled MAD; P10/P90 use linear interpolation at `p*(n-1)`; CV uses population standard deviation / mean):

  | Metric | Parent | Candidate |
  |---|---:|---:|
  | Median | `21.424200` | `20.904050` |
  | MAD | `2.374200` | `2.556100` |
  | P10 | `17.852430` | `17.596760` |
  | P90 | `29.132480` | `35.268720` |
  | Min / max | `10.694400 / 151.047000` | `11.005600 / 157.731000` |
  | CV | `64.00857824%` | `74.24592616%` |

- Both pooled distributions are highly noisy; maxima are retained and no sample was classified as a protocol failure or removed. `LOAD_QUALITY=NOISY`, `MEASUREMENT_QUALITY=NOISY`, `NEEDS_ONE_MORE_LOCAL=YES`.
- First Local batch device 4 snapshot: AICore `68% -> 66%`; HBM `59193 -> 59195 MB`. Second batch: AICore `60% -> 58%`; HBM `59192 -> 59195 MB`. Existing PID `2999855` (`VLLMEngineCor`, `55666 MB`) was present before and after both batches, untouched.
- `CONTROL_ONLY=NO` for shape 8192, but this single noisy shape is only a partial local score and is not comparable to Official.

## Result Flags

- `COMPILE=PASS`; reference probe build `PASS`.
- `CORRECTNESS=FAIL_PARTIAL`: Candidate passed the threshold shape, failed 8184, and shared the Parent failure at 8200.
- `PARTIAL_CORRECTNESS=YES`.
- `LOCAL_SCORE=1.030593923531x` (12-pair partial, noisy; not a promotion).
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `CURRENT_LOCAL_BEST=NONE`; `NEEDS_ONE_MORE_LOCAL=YES`. Do not promote V074 from this noisy single-shape result; use exact `R31B-V011` as Parent for the next threshold OFAT.
