# MAIN-REVIEW — SHAPE-TILING-CHAMPION-X V001

来源：MAIN-1 inbox，2026-09-29。本文件是该决定的落档，不改写 Review 原文语义。

## Review 结论

- `SINGLE_CHANGE_AUDIT=PASS`：仅 FP32 中宽 D（8192 < D ≤ 16384）seed tile 4096→8192，未触碰 reduction / store / dtype / scheduling。
- `BUILD=PASS`，`EXECUTABLE_IDENTITY=PASS`（local == remote）。
- `OFAT_NUMERIC_SAFETY=PASS`：未触碰格子（fp16/bf16 全 D、fp32 D=8192）parent vs candidate 逐位一致（delta 0）；变更域内 parent-vs-candidate delta 约 0.09 与 parent 自身 run-to-run 噪声同量级；D=32768（不在变更域）也有同样 delta，证明噪声来自主基线。
- `CORRECTNESS=COMMON_MODE_WITH_PARENT`（非 V001 回归）。共享基线限制：wide-FP32 invRms 跨 run 变动约 0.8%。

## 处置（照 MAIN-1 方向执行）

1. 全部 build / correctness / determinism 证据保留并 git commit + push，证据与 V001 状态分开提交。
2. `local-result.json` 明确记 `CORRECTNESS=COMMON_MODE_WITH_PARENT`、`OFAT_NUMERIC_SAFETY=PASS`、`SHARED_BASELINE_LIMITATION`。
3. **允许进入性能测量**：测量对象是 parent vs candidate 的延迟差，不是对 golden 的数值精度。same-binary 资格仍必须先过。
4. 优先形状：FP32 D=12288、D=16384（变更域）；对照 FP32 D=8192、D=32768。FP16/BF16 全 D 作空白对照。
5. 与 R31B lane 协调：非确定性证据写入 `研究/SHAPE-TILING-CHAMPION-X/WIDE-FP32-NONDETERMINISM.md`，由 Main 汇总；本路线不改其他 lane 文件。
6. V001 测量完成前不开 V002；测量后按 `LOCAL_ACCEPTED` / `LOCAL_REJECTED` / `NEEDS_ONE_MORE_LOCAL` / `MEASUREMENT_BLOCKED` 出结论并写 handoff。
