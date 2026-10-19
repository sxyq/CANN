# V001 Historical Evidence

## Review scope

- `phase4/control/idea-pool-29-routes.md` consolidates the historical route summaries for R001-R029.
- `归档/phase3-before-reset-20260920/管理/路线状态/R001.json` through `R029.json` were parsed as 29 records; their `local_reference` and `next_action` fields were reviewed.
- `归档/phase3-before-reset-20260920/提交/版本实验记录.md` provides the experiment-level result summary.

## Dtype selection

- **R004 / low-precision middle:** `归档/phase3-before-reset-20260920/管理/路线状态/R004.json` records the CPU counterexamples: FP16 square overflow and BF16 large-D accumulation mismatch. The archived `phase4/archive/historical-branches-20260924/fast__lane-a/files/实验/local/FAST-BREADTH/lane-a/R004/V001/status.md` records compile failure in the native low-precision path and no online readiness. This argues against choosing an FP16/BF16 arithmetic-path experiment without range guards.
- **R017 / FP32 middle:** `归档/phase3-before-reset-20260920/管理/路线状态/R017.json` identifies the CPU main matrix and H001/V005 three-dtype NPU evidence. `归档/phase3-before-reset-20260920/提交/版本实验记录.md` lines 110 and 265 record the 324-case CPU matrix (FP32 strict pass) and H001/V005's 40 three-dtype NPU cases passing. The archived isolated R017 work also has a CANN compile/link and minimum-run record at `phase4/archive/historical-branches-20260924/exp__full-r011-r017-v001/files/实验/server/FULL-R017-FP32-FULL-INTERMEDIATE/V001/L001/compile.log` and `minimum-run.log`; these are supporting, not full-matrix, evidence.
- **R029 / dtype-specific wide-cache evidence:** the archived BF16 wide cached-y result in `归档/phase3-before-reset-20260920/提交/外部轨道-ChatGPT编译并修复/源码/R029-V003-WIDE-BF16-CACHED_沙箱下载件/结果.md` reports 15/15 and score 28.68, specifically for a wide-row cached-y specialization. The FP32 wide cached-row result at `归档/phase3-before-reset-20260920/实验/online/FULL-R029-WIDE-CACHED-ROW/V001/6aad7849b0477ec41e7fd3b1/result.json` is Runtime Error, 4/15, with no score. R029 does not support importing its wide cached-y mechanism into this route.

## Decision

FP32 has the strongest applicable evidence for V001: the middle arithmetic already has a passing FP32 reference path, while the proposed change preserves numeric values and alters only same-type FP32 UB copies. R004's failures concern changed low-precision arithmetic; R029's successful dtype evidence concerns a different wide-row cache architecture.
