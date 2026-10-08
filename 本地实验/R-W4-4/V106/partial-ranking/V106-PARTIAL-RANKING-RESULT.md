# MODE-DISPATCH-CUTOFF-X V106 Partial Local Result

- `DIRECT_PARENT=exact R31B-V011` (`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`). Candidate SHA256: `74d7884a78bd9b2e5654d9e459290eafac4a71ff2b5fd5f6e47a950213295092`.
- `SINGLE_CHANGE=kSmallFp32BatchMaxWidth: 4096 -> 952`; the only Parent-to-Candidate source difference is this constant. C15 `1x32768` remains excluded for its known exact-Parent failure and was not rerun.
- Parent and Candidate Correctness passed FP32 `128x948`, `128x952`, and `128x956` (`6/6`, all `rc=0,bad=0`).
- Local used device 3, six interleaved P/C pairs per shape, 45 warmups, 31 samples/block, two blocks, batch 64. All 36 invocations returned `rc=0,bad=0`; all 2,232 raw device-event samples are retained.
- Pair speedup is `median(Parent raw device_us) / median(Candidate raw device_us)` over 62 samples per invocation. Shape score is the arithmetic mean of its six pair speedups; route score is the equal-weight geomean of the three shape scores. Combined-MAD test is `abs(parent median - candidate median) <= parent MAD + candidate MAD`.

| Shape | Six pair speedups | Shape score / delta | Candidate faster / within combined MAD | P/C pooled median us | P/C pooled CV | P/C throughput Gelem/s |
|---|---|---:|---:|---:|---:|---:|
| 128x948 | 1.097723, 1.007968, 0.947711, 0.990585, 1.002413, 0.890768 | 0.989528041708x / -1.047196% | 3/6 / 3/6 | 7.512655 / 7.618595 | 4.989732% / 4.122846% | 16.151946 / 15.927346 |
| 128x952 | 0.972118, 0.975153, 0.974719, 0.558761, 0.996744, 0.967398 | 0.907482061036x / -9.251794% | 0/6 / 3/6 | 7.430155 / 7.651875 | 4.036491% / 26.672064% | 16.400196 / 15.924986 |
| 128x956 | 0.987338, 0.991635, 0.983738, 0.995255, 0.978309, 0.999710 | 0.989331147401x / -1.066885% | 0/6 / 6/6 | 7.499530 / 7.616090 | 6.588375% / 6.600262% | 16.316756 / 16.067037 |
| Equal-shape geomean | - | **0.961322880271x / -3.867712%** | **3/18 / 12/18** | - | - | - |

## Quality and Decision

- `PARTIAL_CORRECTNESS=YES`; `LOCAL_SCORE_COMPARABLE_TO_OFFICIAL=NO`; `OFFICIAL_SCORE=NONE`; `ONLINE=NOT_RUN`.
- `LOAD_QUALITY=STABLE_LOW_LOAD_DEVICE3`: 36 pre/post usage snapshots show HBM usage 5%, AICore 0%, AIVector 0%, CtrlCPU 2-17%; per-pair NPU snapshots show no device 3 process. Before/after Local snapshots indicate approximately 62,107 MB free HBM. All samples remain included.
- `MEASUREMENT_QUALITY=NOISY`: Candidate is faster in 3/18 pairs; 12/18 are within combined MAD. At 128x952, pair 04 speedup was 0.558761x and Candidate pooled CV was 26.672064%; no samples were excluded.
- Two post-Local awk summaries failed with a syntax error before reading input; the corrected read-only load aggregation succeeded. Details are retained in `v106-result-analysis-command-errors.txt`; experiment data was not changed.
- `LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL`; `CURRENT_LOCAL_BEST=exact R31B-V011`. V106 is not promoted.

## Timestamps

- Candidate edit: `2026-10-08T22:20:38.255631399Z`.
- Compile log final timestamp: `2026-10-08T22:21:47.328250711Z`; root `device/submission` and both route-bound reference probe targets passed.
- Correctness: `2026-10-08T22:23:08.185331988Z` to `2026-10-08T22:23:48.491139239Z`.
- Local capture: `2026-10-08T22:24:30.754220794Z` to `2026-10-08T22:29:48.274135972Z`; device 3 released at `2026-10-08T22:30:08.798492578Z`.
- `LOCAL_RESULT_TIMESTAMP=2026-10-08T22:31:52.151298269Z`; Local-capture-to-result gap `123.877162297 seconds`.
- `NEXT_EDIT_TIMESTAMP=PENDING_AFTER_V106_COMMIT`.
