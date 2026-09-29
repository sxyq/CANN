# R31B Handoff — V019（H3 删冗余 SyncVToMTE2）

Lane: M1-1 / Route R31B（Champion exploit 主线）
Worktree: `/Users/sunyiyang/Desktop/Project/cann-m1-r31b`，branch `m1/r31b-exploit`
日期：2026-09-29

---

## V019 收口结果（H3）

```text
REVISION=V019
DIRECT_PARENT=R31B-V017
PARENT_SOURCE_SHA=7c168eafde4c07d2a0667253a06925070e10349ccf08327a1b287302788180c4
PARENT_SCORE=LOCAL bf16-wide-d32768 -14.3% + fp16-wide-d32768 -6.7/-8.5% (V017 vs V016) / Official anchor 45.16 (V011)
SINGLE_HYPOTHESIS=H3 删除 ProcessWideLowPrecision 中两处冗余 SyncVToMTE2：
  (a) pass-2 每 (tile,row) Store 前的 V→MTE2 全同步
  (b) pass-1→pass-2 交界处的 V→MTE2 全同步
  SyncVToMTE3 与全部 event ID 逻辑保留；H6 未含（互斥）
SOURCE_SHA=ce5329da0976d5aac783eb51019a638dd8220dbf296601cba08d3391a53cc077
CONTEXT_CLASS=HISTORICAL_EXPLOIT
BUILD=PASS（RC=0；device.alink=e62cbf2e…，submission.alink=9986fd48…，
  paired_runner_v019=42f067e9…）
CORRECTNESS=PASS（CheckOutput 双 binary 12/12 两 attempt；BITWISE 12/12 IDENTICAL）
EXECUTABLE_IDENTITY=PASS（parent=V017 kernel / candidate=V019 kernel，SHA 对齐）
SAME_BINARY=PASS（d5，6/6 形状，MAD/med≤0.057，drift≤0.028，warmup=45）
LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL
LOCAL_BEST=V017（不变）
```

## 测量结果（device=5，parent=V017 / candidate=V019）

H3 变更域 = `ProcessWideLowPrecision` = 全部宽行形状（FP16/BF16 × tail/wide）；非宽 D=8192 为空白对照。

| 形状 | 变化域 | a1 delta | a2 delta | 干净对负占比 | 判定 |
|---|---|---|---|---|---|
| **bf16-tail-d12288** | 是 | **−0.36µs** | −0.04µs | 11/15、11/18 | a1 出带但不复现 |
| **bf16-wide-d32768** | 是 | −0.02µs | **−0.20µs** | 7/16、4/9 | a2 出带但不复现 |
| **fp16-wide-d32768** | 是 | −0.12µs | +0.10µs | 8/15、7/16 | 符号翻转，带内 |
| **fp16-tail-d12288** | 是 | +0.14µs | +0.10µs | 3/5、6/16 | 带内（对照同量级） |
| fp16-ctrl-d8192 | 否（非宽） | −0.06µs | +0.12µs | 8/16、8/20 | 空白对照 |
| bf16-ctrl-d8192 | 否（非宽） | −0.06µs | **+0.14µs** | 11/21、8/16 | 空白对照，本轮 \|median\| 上界 |

- **两关判定均 FAIL**：
  1. 噪声带：当轮空白对照 |median| ≤ **0.14µs**。变化域仅两个单次值出带（bf16-tail a1 −0.36、bf16-wide a2 −0.20），在同形状另一次 attempt 中均回到带内（−0.04、−0.02）。无复现出带信号。
  2. 机制幅度：预测 **0.2–0.8µs/行批**（全同步成本未标定，区间刻意放宽）。可复现分量约 **0.0–0.15µs** 且符号混杂，贴着或低于下界。
- **正确性结论（有价值的架构事实）**：BITWISE 12/12 IDENTICAL，两处被删的 `SyncVToMTE2` **不承重**——`rel0/rel1`（pass-1 staging 释放）与 `prel0/prel1`（pass-2 参数 staging 释放）事件已覆盖真实的 UB 复用顺序。预注册的失败模式（同步承重）未兑现，**无需** ARCHITECTURE_FINDING。
- PAIR_COUNT：21 交错对/形状/attempt × 2（42 对/形状）。4-block×11 仍 MISSING。
- NOISE：同码 MAD/median ≤0.057；配对 delta MAD 0.14–72.1µs，含 device-event 单点尖峰（−185~+236µs 级）；median 与干净对稳健，raw 全留。MAD>0.25µs 的块已标注受污染，不用于强结论。
- LOAD_QUALITY：d5，HBM 59899–59903/65536（~91.5%，<100%）pre/post，AICore 0%，VLLMWorker_TP 仅观察。d4 未用（V018 轮记录其上 VLLM 活跃推理）。lease 行未写入 `调度/服务器设备使用.tsv`（lane 边界禁改）——请 MAIN-1 补记 `R31B-V019-D5-PERF-20260929`。

## 累积机制结论（V018 + V019，喂回假设池）

同一函数 `ProcessWideLowPrecision` 内两次独立的 **V 侧工作删除**均测得 ≈0：

| Revision | 删除内容 | 量级 | 实测 |
|---|---|---|---|
| V018 (H2) | 每 unit 1 整 tile Muls + 1 中部 PB | 预测 0.3–1.0µs | ≈0（带内） |
| V019 (H3) | 每 (tile,row) 1 次 V→MTE2 全同步 + 交界 1 次 | 预测 0.2–0.8µs | ≈0（带内） |

两次都作用在 V 指令流 / V 侧同步上，两次都无效。**证据指向 pass-1/pass-2 的节奏由 MTE2 装载延迟与 reduce 链决定，不在 V 侧。**

对下一轮选型的含义：

1. **H5（pass-1 MTE2 staging 2→3-deep）优先级应升至 1**——它直接作用在被指认的瓶颈（MTE2 载入窗口），且 V011 的 2-deep 在本站点已证明有效（+1.25 Official）。前置条件：先做 UB 预算核对（xBuf_/residualBuf_ 各 +1 tile slot 可能把 `wideFullYRows_` 从 2 压到 1，有 COEFF-LOCALITY-X 的前车之鉴）。
2. **H4（pass-2 BF16 重复 widen 删除）建议降级或弃**——同为 V 侧算子删除，按 V018/V019 的连续否证，大概率同样落带内。
3. **H6（store 前 PB 删除）**——V018 已证中部 PB 在本站点近似免费，预期幅度应按 ~0.01µs/个 重算，很可能不值得单独一轮。与 H3 互斥约束在 H3 落地后自然解除，但优先级低。
4. **H3 的正确性结论可复用**：`rel0/rel1` + `prel0/prel1` 事件覆盖是完整的。后续任何 pass-1/pass-2 staging 复用改动（含 H5 加深到 3-deep）可以依赖这套事件模式，不必再加全同步。

## 状态与建议

```text
STATUS=needs_main_review（V019 NEEDS_ONE_MORE_LOCAL）
LOCAL_BEST=V017（不变，链条 V011 -> V016 -> V017）
CURRENT_CANDIDATE=V019（不推进 LOCAL_BEST）
```

- **不建议**第三次配对：42 对 / 2 次独立已覆盖；若 Main 要求可补 4-block×11 作协议完备。
- **不构成 LOCAL_REJECTED**：无复现退化（fp16-tail +0.10/+0.14 与未改代码的对照同量级）。
- 下一步建议（供 Main / Planning 选定，本 lane 不自行开 Revision）：
  1. H5 pass-1 MTE2 staging 2→3-deep（先 UB 预算核对）——与累积证据最对齐；
  2. 若 H5 UB 预算不成立，则回看 pass-1 的 MTE2 装载路径本身（非队列深度方向）是否有冗余往返；
  3. H4/H6 在当前证据下不建议优先。
- V016+V017 的 Online 候选评估（trigger condition B）仍待 Main 判定；V019 不推进 LOCAL_BEST，不改变 V017 累积幅度。

## 证据位置

```text
本地实验/R31B/V019/submission.asc            SOURCE_SHA=ce5329da…
本地实验/R31B/V019/submission.sha256
本地实验/R31B/V019/diff.patch
本地实验/R31B/V019/source-meta.json
本地实验/R31B/V019/local-result.json
本地实验/R31B/V019/support/results/         build / correctness / same-binary / paired-a1 / paired-a2 / snapshots
server3:/home/data4t2/lelinfeng/phase4-review-repro/R31B-V019/
```
