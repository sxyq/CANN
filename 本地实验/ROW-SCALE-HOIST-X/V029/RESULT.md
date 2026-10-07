# V029 Result

## Outcome

- Compile: PASS. Correctness: PASS.
- Local score: `96.979171875` route-local speedup index, using `100 * parent_pooled_median / candidate_pooled_median`.
- Local delta: `+3.114924645%` candidate slower; pooled device-event medians are `9.3100005 us` Parent and `9.6000000 us` Candidate.
- Current Local Best remains V026. V029 is a negative Local result and is not Official-comparable. Online was not run.

## Primary Local Window

Run `20261007T031411Z`, device 0, 910B3, FP16 `[128, 2048]`, 40 vector cores, 20 warmups and 32 measured device-event samples per process. Order was `P-C-P-C-P-C`; all six process return codes were 0. The command ran from `2026-10-07T03:14:11Z` through `2026-10-07T03:14:42Z`. The parent and candidate executable paths and full runner output are retained in `local/paired-command-20261007T031411Z.log` and the six corresponding `local/{P1,C1,P2,C2,P3,C3}-20261007T031411Z.stdout` files.

All 96 raw samples per arm are embedded in `local-result.json`; none are filtered. Paired-block median deltas (candidate vs parent) were `-4.017862%`, `+5.731832%`, and `+17.346926%`. Candidate was slower in two of three blocks. Pooled jitter was high: Parent mean/stdev/CV `26.148542 us / 40.603056 us / 1.552785`; Candidate `28.690417 us / 61.171414 us / 2.132120`. Runner-style mean absolute deviation from pooled median was `17.618958 us` Parent and `20.181667 us` Candidate; p10/p90 were `8.230001/79.070004 us` Parent and `8.180000/62.219999 us` Candidate. Long-tail samples remain included.

The runner reports `3,162,112` full logical I/O bytes. Estimated throughput at pooled median is `339.646813 GB/s` Parent and `329.386667 GB/s` Candidate. This is a logical-traffic estimate, not a measured HBM bandwidth.

## Device Load

Before/after snapshots are retained at `local/device-load-before-20261007T031411Z.txt` and `local/device-load-after-20261007T031411Z.txt`. Device 0 HBM was `60,221/65,536 MB` before and `60,224/65,536 MB` after (free `5,315` to `5,312 MB`); device 0 AICore was 0% in both snapshots. VLLM workers occupied devices 0-3 and 5-6 at about `56.7 GB` per device; device 4 had a VLLM engine process using `55,666 MB`; device 7 had a Python process using `21,260 MB`. The after snapshot showed device 7 AICore at 9%. Host process activity was also high. Treat the Local comparison as contention-affected and noisy; do not discard any timing samples.

## Other Attempts

The earlier successful `20261007T0221Z` P-C-P-C-P-C window is retained separately in the JSON and raw files; its samples are not pooled into the primary score. The `00:38Z` launch attempt is also retained: all runner launches exited 127 because `libmsprofiler.so` could not be loaded, so it produced no timing samples and is not a measurement window.

## Receipt

```text
ROUTE_EVENT
ROUTE = ROW-SCALE-HOIST-X
REVISION = V029
LAST_ACTION = RESULT
NEXT_ACTION = COMMIT
CHANGE = Move per-row inverse-RMS multiply ahead of gamma multiplication in the changed output paths
COMPILE = PASS
CORRECTNESS = PASS
LOCAL_SCORE = 96.979171875
LOCAL_DELTA = +3.114924645%
CURRENT_LOCAL_BEST = V026
GIT_COMMIT = PENDING
PUSH = NO
BLOCKER = NONE
```
