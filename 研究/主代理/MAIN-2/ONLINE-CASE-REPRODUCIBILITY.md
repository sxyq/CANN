# Online Case Reproducibility

## Classification

`CASE_REPRODUCIBILITY=PARTIAL` and `LOCAL_JUDGE_MODE=CALIBRATED_SURROGATE`.

The public problem endpoint and ranking endpoint expose the 15 ordered testcase IDs and current `tbest` values. The public problem description exposes supported dtype and dimension ranges and examples, but does not expose the exact per-case shape, dtype, row count, hidden input values, or runner configuration needed for 1:1 local reproduction.

Problem endpoint: https://cannjudge.cn/api/problems/name/addrmsnormbias
Ranking endpoint: https://cannjudge.cn/api/problems/6a9a9a99bf41025d6013eb85/ranking?page=1&size=1

## Formula Confirmation

The formula is present in `工具/cannjudge.py` and `工具/cannjudge-submit.mjs`, and is stated in the public problem description: s_i = 100 / (1 + log_1.5(time_i / best_time_i)); total = mean(s_i)

The current API vector is recorded in `ONLINE-BEST-TIMES.tsv`. Historical retained result payloads contain different best-time snapshots, so the vector is not treated as a timeless constant. Historical replay uses each submission's own recorded `best_time` when available.

## Exact vs Surrogate Boundary

Known exactly: testcase count, ordering, testcase IDs, formula, current public tbest snapshot, and historical result payload fields.
Known only partially: shape/dtype/workload mapping and local runner equivalence.
Unknown: hidden inputs and exact Official runner scheduling/configuration.

No local tool in the repository can currently emit the exact 15 hidden cases. The Local Judge therefore refuses to turn a single-shape local delta into a numeric predicted Official score.
