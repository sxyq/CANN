# R-W4-4 V084 Partial Ranking Result

- `REVISION=V084`; exact comparison Parent is `R31B-V011`, SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`.
- Candidate SHA256: `7fa17c37cc520393cf361b318659307f7dd8a0a4203fafdb8a27f564c4f851b1`.
- Single threshold OFAT: `kSmallFp32BatchMaxWidth`, `4096 -> 1536`; source diff is only this constant.
- Exact-Parent C15 FP32 `1x32768` remains a known Parent correctness failure and was excluded; it was not run.
- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`.

## Build and Correctness

Route build and reference Parent/Candidate probe builds passed (`CMAKE_CONFIGURE_RC=0`, `ROUTE_BUILD_RC=0`, `PROBE_CONFIGURE_RC=0`, `REF_PROBE_BUILD_RC=0`). On host `hwnput3`, device 4, Parent and Candidate each returned `rc=0,bad=0` on all three FP32 widths.

| Shape | Parent max abs | Candidate max abs | Local |
|---|---:|---:|---|
| `128x1528` | `1.43051e-06` | `1.43051e-06` | yes |
| `128x1536` | `1.43051e-06` | `1.43051e-06` | yes |
| `128x1544` | `1.43051e-06` | `1.43051e-06` | yes |

## Partial Local

All three jointly correct shapes were measured on fixed device 4. Each shape used six interleaved P/C pairs, warmup 45, 31 samples/block, 2 blocks, batch 64, gap 0, with order `P,C; C,P; P,C; C,P; P,C; C,P`. All 36 calls returned `rc=0,bad=0`; all 2,232 device-event samples were retained (372/side/shape, 744/shape). Pair-level medians/MADs are in `v084-local-paired-summary.tsv`; pooled summaries are in `v084-local-pooled-summary.tsv`.

For each pair, `pair_speedup = Parent_median / Candidate_median` over its 62 raw `device_us` samples. Each shape score is the arithmetic mean of its six pair speedups. `PARTIAL_ROUTE_LOCAL_SCORE` is the equal-weight geometric mean of the three shape scores; delta is `(score - 1) * 100`.

| Shape | Mean pair speedup | Delta | Candidate-faster pairs | P/C pooled median us | P/C CV | P/C effective throughput Gelem/s |
|---|---:|---:|---:|---:|---:|---:|
| `128x1528` | `1.059195433343x` | `+5.919543334%` | `5/6` | `11.221600 / 10.693300` | `15.9813% / 33.0786%` | `17.429244 / 18.290331` |
| `128x1536` | `0.982179127168x` | `-1.782087283%` | `2/6` | `9.804845 / 10.048100` | `13.6053% / 14.0424%` | `20.052127 / 19.566684` |
| `128x1544` | `0.950597055127x` | `-4.940294487%` | `1/6` | `10.055800 / 10.496550` | `31.7211% / 16.8596%` | `19.653533 / 18.828282` |

- `PARTIAL_ROUTE_LOCAL_SCORE=0.996294550634x`; `PARTIAL_ROUTE_LOCAL_DELTA=-0.370544937%`.
- All 18/18 pair median differences were within their combined Parent+Candidate MAD. The aggregate is a noisy local ranking, not a resolved gain.
- `LOAD_QUALITY=NOISY`; `MEASUREMENT_QUALITY=NOISY`; `NEEDS_ONE_MORE_LOCAL=YES`. Candidate `128x1528` retains a `59.5719 us` maximum; Parent `128x1544` retains a `68.3563 us` maximum. No sample was removed.
- Device 4 pre-Local snapshot `2026-10-08T03:32:07.711401033Z`: AICore `58%`, HBM `59193/65536 MB` (`6343 MB` free). Post-Local snapshot `2026-10-08T03:37:45.045323741Z`: AICore `64%`, HBM `59195/65536 MB` (`6341 MB` free). Existing PID `2999855` (`VLLM::EngineCore`) was present before and after and left untouched.
- `DEVICE4_RESERVATION=RELEASED` at `2026-10-08T03:40:50.407211058Z`, after all raw/stat and post-Local evidence was captured and no active MODE probe process remained.

## Result Flags

- `COMPILE=PASS`; reference probe build `PASS`.
- `CORRECTNESS=PASS` on the three tested widths; `PARTIAL_CORRECTNESS=YES` because exact Parent C15 remains excluded with a known failure.
- `LOCAL_SCORE=0.996294550634x` (`-0.370544937%`), numeric partial route-local score.
- `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `ONLINE=NOT_ELIGIBLE`.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=NONE`; V084 is not promoted. Continue bounded threshold OFAT from exact `R31B-V011`; C15 remains excluded.
