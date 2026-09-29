# R31B Handoff — V018（H2 pass-1 retained-y 直写）

Lane: M1-1 / Route R31B（Champion exploit 主线）
Worktree: `/Users/sunyiyang/Desktop/Project/cann-m1-r31b`，branch `m1/r31b-exploit`
日期：2026-09-29

---

## V018 收口结果（H2）

```text
REVISION=V018
DIRECT_PARENT=R31B-V017
PARENT_SOURCE_SHA=7c168eafde4c07d2a0667253a06925070e10349ccf08327a1b287302788180c4
PARENT_SCORE=LOCAL bf16-wide-d32768 -14.3% + fp16-wide-d32768 -6.7/-8.5% (V017 vs V016) / Official anchor 45.16 (V011)
SINGLE_HYPOTHESIS=H2 pass-1 retained-y 直写：ProcessWideLowPrecision pass-1 FP16 分支
  Add 结果直接写入 retained y 槽，删除 Muls(xLocal, 1.0) 拷贝与其中部 PipeBarrier，
  后续 ToFloat 改读 y 槽
SOURCE_SHA=31241ca2ccb6a7e705ac0bdc60f67647c68f4e4d4faa5c924d34c492e245b203
CONTEXT_CLASS=HISTORICAL_EXPLOIT
BUILD=PASS（RC=0；device.alink=cecbc206…，submission.alink=48001786…，
  paired_runner_v018=06576df5…）
CORRECTNESS=PASS（CheckOutput 双 binary 12/12 两 attempt；BITWISE 12/12 IDENTICAL）
EXECUTABLE_IDENTITY=PASS（parent=V017 kernel / candidate=V018 kernel，SHA 对齐）
SAME_BINARY=PASS（d5，6/6 形状，MAD/med≤0.045，drift≤0.053，warmup=45）
LOCAL_VERDICT=NEEDS_ONE_MORE_LOCAL
LOCAL_BEST=V017（不变）
```

## 测量结果（device=5，parent=V017 / candidate=V018）

| 形状 | 变化域 | a1 delta | a2 delta | 干净对负占比 | 判定 |
|---|---|---|---|---|---|
| **fp16-wide-d32768** | 是（主探针） | **−0.16µs** | **−0.02µs** | 11/15、10/15 | 带内，幅度不匹配 |
| **fp16-tail-d12288** | 是 | +0.00µs | +0.06µs | 10/19、10/21 | 带内 |
| bf16-wide-d32768 | 否（BF16 路径未改） | −0.04µs | +0.06µs | 9/16、7/15 | 对照，带内 |
| bf16-tail-d12288 | 否 | +0.14µs | −0.02µs | 7/18、6/11 | 对照，带内 |
| fp16-ctrl-d8192 | 否（非宽） | −0.06µs | **−0.24µs** | 8/16、11/14 | 空白对照，本轮最大 \|median\| |
| bf16-ctrl-d8192 | 否（非宽） | +0.04µs | +0.04µs | 8/20、8/16 | 空白对照，带内 |

- **两关判定均 FAIL**：
  1. 噪声带：当轮空白对照（bf16 未改路径 + 非宽 D=8192）配对 delta 中位最大 |median| = **0.24µs**（出现在代码未改的 fp16-ctrl a2）。变化域实测 |median| ≤ 0.16µs，落在带内。
  2. 机制幅度：预测 **0.3–1.0µs/行批**（中部 barrier + 整 tile Muls/unit）；实测 **−0.16~+0.06µs**，低于下界 5–10 倍且符号混杂。按 H2 预注册规则（<0.1µs 判幅度不匹配），a2 与两个 tail 已在 0.0–0.1µs 级。
- 逐位同值**已证实**：BITWISE parent_vs_candidate=IDENTICAL，6 形状 × 2 attempt 共 12/12，diff_elements=0。`half*1.0` 确为精确拷贝，直写不改位型。
- PAIR_COUNT：21 交错对/形状/attempt × 2 attempt（42 对/形状）。4-block×11 仍缺，记 MISSING。
- NOISE：同码 MAD/median ≤0.045；配对 delta MAD 0.16–2.48µs，偶发单点 device-event 尖峰（−162~+199µs 级），median 与干净对（双侧 device_us<20µs）对其稳健，raw 全留。MAD>0.25µs 的块已标注为受污染，不用于强结论。
- LOAD_QUALITY：d5，HBM 59901–59903/65536（~91.5%，<100%）pre/post，AICore 0%，VLLMWorker_TP 仅观察。d4 首轮 same-binary 被 VLLMEngineCor 活跃推理污染（AICore 65%，MAD/median 0.22–0.41），未在其上跑 P/C，日志保留为污染证据。d6 有 MAIN-2 lease，未触碰。lease 行未写入 `调度/服务器设备使用.tsv`（lane 边界禁改共享文件）——请 MAIN-1 补记 `R31B-V018-D5-PERF-20260929`。

## Build Fix 记录（非性能改动）

1. CMake 将 `CMAKE_ASC_COMPILER` 解析到 `ascend-toolkit/latest/.../bisheng`，嵌套 host 编译 `asc_plugin_binary_register_code.c` 找不到 `<vector>`；显式指定 `8.5.0.alpha002/.../bisheng` 后仍失败。
2. 补 `CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11` 后编译通过。
3. 运行需 `LD_LIBRARY_PATH=/usr/lib/aarch64-linux-gnu` 优先，以系统 libstdc++ 提供 `GLIBCXX_3.4.29`。

失败与成功日志均保留（`paired-build-failed-nocpath.log` / `build2.log` / `run-env.txt`）。

## 机制结论（喂回假设池）

预注册的失败模式兑现：**pass-1 在该处不被 V 吞吐卡住**。删掉每 unit 一个整 tile Muls + 一个中部 PipeBarrier 后无可测窗口，说明 pass-1 节奏由 MTE2 装载 / reduce 链决定，不在 V 指令流。

对 `研究/R31B/next-hypotheses.md` §0.1 的含义：

- 「中部 barrier ≈0.07µs/unit」的 V024 外推**不能**迁移到本 FP16 pass-1 位点；此处 barrier 为 ~0.01µs 级或免费。
- **H4**（pass-2 BF16 重复 widen 删除，2 Cast + 1 PB/tile，定价 0.1–0.4µs）同为算子删除类机制，开跑前应按本发现重核幅度；若按同一比例缩水，可能同样落带内。
- **H3**（删冗余 V→MTE2 全同步）与 **H5**（pass-1 MTE2 staging 2→3-deep）不受影响——它们作用在 DMA/同步侧，正是本轮指向的疑似瓶颈。
- **H6**（barrier 清账）的标定价值仍在，但预期幅度应按 ~0.01µs/个 重算。

## 状态与建议

```text
STATUS=needs_main_review（V018 NEEDS_ONE_MORE_LOCAL）
LOCAL_BEST=V017（不变，链条 V011 -> V016 -> V017）
CURRENT_CANDIDATE=V018（不推进 LOCAL_BEST）
```

- **不建议**对同一对再跑第三次：42 对 / 2 次独立试验已在带内，第三轮不改变判定。若 Main 要求，可补 4-block×11 布局作协议完备，但不预期改写结论。
- **不构成 LOCAL_REJECTED**：无稳定退化，输出逐位一致。
- 下一步建议（供 Main / Planning 选定，本 lane 不自行开 Revision）：
  1. H3 删冗余 V→MTE2 全同步 —— 直接打本轮指向的 DMA/同步瓶颈，优先级升至 1；
  2. H5 pass-1 MTE2 staging 2→3-deep（需先做 UB 预算核对）；
  3. H4 在幅度重核后再决定是否跑。
- V016+V017 的 Online 候选评估（trigger condition B）仍待 Main 判定，与 V018 结果无关（V018 不推进 LOCAL_BEST，不改变 V017 的累积幅度）。

## 证据位置

```text
本地实验/R31B/V018/submission.asc            SOURCE_SHA=31241ca2…
本地实验/R31B/V018/submission.sha256
本地实验/R31B/V018/diff.patch
本地实验/R31B/V018/source-meta.json
本地实验/R31B/V018/local-result.json
本地实验/R31B/V018/support/results/         build / correctness / same-binary / paired-a1 / paired-a2 / snapshots
server3:/home/data4t2/lelinfeng/phase4-review-repro/R31B-V018/
```
