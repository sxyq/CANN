# LANE-FACT-PACK-FOR-PLANNING — ASYNC-OVERLAP-CHAMPION-X（LANE M2-4）

```text
LANE           ASYNC-OVERLAP-CHAMPION-X
WORKTREE       /Users/sunyiyang/Desktop/Project/cann-m2-async
BRANCH         m2/async-overlap-champion
SEED           R31B-V011  SHA a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
ANCHOR         Official 45.16
STATUS         LANE_NEEDS_PLANNING_REVIEW（非 PARK）
本文件         事实包，供 Planning 裁定；不请求具体决定
```

---

## 1. V001–V004 对照表

| Rev | 机制 | 落点 | 正确性 | 本地结论 | Official | 根因 |
|---|---|---|---|---|---|---|
| **V001** | H1 inter-pass prologue param prefetch | `ProcessWideLowPrecision`：pass-2 tile-0 gamma/bias Load 提到 invRms 前；删冗余 SyncVToMTE2 | PASS_VS_PARENT（LP 形状 OUTHASH 逐位一致） | LOCAL_ACCEPTED（128×16384 fp16 3/3 干净对，median −1.3%~−2.2%；三形状 p10 12/12） | **44.17 REJECT**（Δ=−0.99） | 本地小胜未迁移；见 §2 |
| **V002** | W2 NarrowMid 行间发射重排 | `ProcessNarrowMidOverlap`：下一行 x/res Load 提前到 invRms 前；SyncMTE3ToV 下沉 | PASS_VS_PARENT（7 形状 OUTHASH 全等） | NEEDS_ONE_MORE_LOCAL（终局） | 未送 | 行间提前 Load 的收益窗口不成立；512×1024 甚至偏 parent |
| **V003** | W4 GetValue handoff 收敛 | `ProcessNarrowMidOverlap`：invRms 尾两轮 V-S 往返 → 一轮 | PASS_WITHIN_TOLERANCE（6/7 逐位一致；32×4096 差 1 bit，max_abs 3.58e-07） | NEEDS_ONE_MORE_LOCAL | 未送 | 砍一轮往返，但标量 sqrt 变更可能抵消同步收益 |
| **V004** | W1 FullCache inter-pass prologue | `ProcessWideFp32FullCacheRows`：tile-0 参数 Load 提到 invRms 尾 | PASS_WITH_NONDET_PATH（控制组逐位一致；FullCache 为 parent 非确定路径） | NEEDS_ONE_MORE_LOCAL（128×16384 fp32 3/3 对半开） | 未送 | xBuf_ 作 invRms scratch，prologue 窗口过小（见 §3） |

**四次实验的共同结构事实**：

- 调度类改动（发射时机 / sync 放置 / handoff 次数）在 Champion 热路径上都未给出稳定本地胜。
- 正确性全部 PASS_VS_PARENT（V003/V004 按裁定允许容差内或非确定路径差异）。
- SINGLE_CHANGE_AUDIT 全部 PASS；未加 buffer/queue/MTE3 stage。
- LOCAL_BEST 始终未推进（V001 虽 LOCAL_ACCEPTED 但 Official REJECT，链保持在 parent 根）。

---

## 2. Official 44.17 REJECT 归因（V001）

| 事实 | 推论 |
|---|---|
| V001 只改 `ProcessWideLowPrecision`（FP16/BF16 且 D>8192） | cases 1/2/3/5/4/6/7/8 **不在变更面** |
| Official Δ=−0.99（约需 case 分合计掉 ~15） | 受影响 case **净变慢**，或本地信号为假阳性 |
| 本地 p10 12/12 偏 V001，干净对 median 仅 8/11 | p10 可能反映抗宿主干扰，非 judge 干净机加速 |
| case 14（16.5ms）dtype/路径未知 | 若 FP32 wide（FullCache），V001 **完全没碰**最大缺口 case |
| 校准记录 | `FALSE_POSITIVE_small_local_win`（Main-2 已登记） |

**结论**：V001 的 1–3% 本地胜要么落在 Official 形状的噪声内，要么在实际
case 形状上反向。只优化 LP 天花板极低——Official 15 case 里能进 LP 的
是 wide FP16/BF16 子集，而最大缺口 case 14 可能是 FP32 wide 或其它路径。

---

## 3. 剩余假设与碰撞风险

### W3 — LP pass-2 store 延迟等待（per-slot MTE3_V）

| 项 | 内容 |
|---|---|
| 机制 | LP pass-2 内层每 batchRow 的 SyncMTE3ToV 解耦，store 源改 y 行区域 |
| 目标 case | 14/15（若为 LP 且 batchRows 大） |
| **碰撞风险** | **高**：与 STORE-EPILOGUE-X（MAIN-2 lane，STORE-H2B 单行驻留写回合并）同属 store 形态轴。STORE-EPILOGUE 已有 V001/V002 实验与 GATED 条件。若本 lane 做 W3，须先确认边界：STORE-EPILOGUE 管「store 合并/形态」，W3 管「store 完成可见性」——相邻但不同段；仍需 Planning 划界。 |
| 先验 | 中（FullCache 已有 per-slot 先例；但 LP 的 y 区域生命周期复杂） |

### W5 — H1 条件化发射

| 项 | 内容 |
|---|---|
| 机制 | 仅当 batchRows≥2 或 tileCount≥4 时提前参数 Load |
| 先验 | **低**：V001 的 Official 负收益若源自删 SyncVToMTE2 或 HBM 竞争，条件化不对症；若源自形状不对齐，条件化也不解决 |
| 建议 | 不优先 |

### invRms scratch 重定位（扩大 prologue 窗口）

| 项 | 内容 |
|---|---|
| 机制 | 把 FullCache invRms 的 Duplicate/Sqrt scratch 从 xBuf_ 挪到专用小缓冲，使 xBuf_ 在 invRms 期即空闲，参数 Load 可覆盖整个 invRms |
| **边界风险** | 触碰「不加 buffer」的解释——虽可争辩为「复用现有空闲槽」，但本质是 UB 用法变化，须 Planning 明示是否允许 |
| 先验 | 中（结构上能扩大 W1 窗口；但 W1 在 23 µs kernel 上已 3/3 对半开，更大窗口未必够） |

### 已排除（本轮不再提）

- 加 buffer / 加深 queue depth（R31B V006/V009 T14 proven-flat）。
- 只加 MTE3 stage（ASYNC-TRIPLE-X 已做）。
- 归约拓扑 / invRms 公式 / dtype 分裂 / row-group 调度 / UB 别名（他路线轴）。

---

## 4. 重启条件

若 Planning 认为本 lane 应继续，建议满足以下任一条件再重启：

| 条件 | 理由 |
|---|---|
| **A. Official case 14/7/6/4/8 的 dtype 与宽度路径映射落地** | 当前最大盲区；不知道缺口 case 走哪条 process 路径，机制选择是盲打 |
| **B. 干净测量窗口**（VLLM 空闲或专用设备） | 本 lane 全部小信号（1–5%）都在带载 d4/d5 的噪声带边缘；p10 vs median 分歧反复出现 |
| **C. Planning 划清 W3 与 STORE-EPILOGUE 的边界** | 否则 W3 有撞车风险 |
| **D. 明示 scratch 重定位是否触碰「不加 buffer」** | 否则扩大 prologue 窗口的方案无法声明 |

无上述条件时，建议保持 `LANE_NEEDS_PLANNING_REVIEW`，不烧测量预算。

---

## 5. 测量装置偏置对本路线小信号的影响

本 lane 研究的是 pipeline scheduling 类微改，预期收益 0.3–2 µs
（相对 10–25 µs kernel 约 1–5%）。测量装置的偏置刚好落在同一量级：

| 偏置源 | 观察 | 对小信号的影响 |
|---|---|---|
| **VLLM / se_candidate_pr 驻留** | d4/d5 全程有 VLLMEngineCor 或 VLLMWorker_TP | 宿主离群点 60–340 µs，CV>1；median 被污染，p10 快簇成唯一可用统计量 |
| **B2 系统性漂移** | 多形状 B2/B1 = 1.12–1.68（warmup=45 不够；w55 反而更差） | same-binary 频繁不合格；合格窗口稀少 |
| **kernel 过短** | NarrowMid 10–14 µs、FullCache 15–25 µs | 1% 效应 = 0.1–0.25 µs，接近 device-event 分辨率下限 |
| **FullCache 非确定路径** | UB-GAP-CLUE：parent 自身 OUTHASH 逐次不同 | 该路径逐位对照不可用；只能比 max_abs 量级 |
| **p10 vs median 分歧** | V001 p10 12/12 偏 candidate，median 8/11；V004 p10 3/3、median 3/3 | 两个统计量方向不一致时无法定 LOCAL_ACCEPTED；V001 的 Official REJECT 证明 p10 偏置会给出假阳性 |

**定量感受**：V001 在带载窗口下 p10 一致偏 candidate（~2%），Official 却 −0.99。
说明带载窗口的 p10 对本类小信号有 **系统性乐观偏置**（candidate 的指令时序
可能恰好更耐宿主干扰）。后续任何 1–3% 的「胜」都必须在干净窗口复核才可信。

---

## 附：证据索引

```text
研究/ASYNC-OVERLAP-CHAMPION-X/
  ROUTE-DECLARATION.md          路线声明与去重
  TRACK-B-HYPOTHESES.md         第一轮假设（H1–H4）
  TRACK-B-HYPOTHESES-V002.md    REJECT 归因 + W1–W5
  HANDOFF-TO-MAIN.md            第一阶段 handoff
  HANDOFF-V001.md … V004.md     各 Revision handoff

本地实验/ASYNC-OVERLAP-CHAMPION-X/
  V001/  声明、源码、correctness、LOCAL_ACCEPTED、results+results2+results3+results4
  V002/  声明、源码、correctness、results+results2
  V003/  声明、源码、PRECISION-STOP/DECISION、results
  V004/  声明、源码、bugfix、correctness、results

线上结果/ASYNC-OVERLAP-CHAMPION-X/V001/
  submission.asc / source-meta.json / result.json   Official 44.17 REJECT
  ONLINE-HANDOFF.md

技术线 commit 链（m2/async-overlap-champion）:
  188e065a Track-B → c2633581 V001 声明 → 772b754e V001 源码 → bd964e8e 构建正确性
  → 18735d63 V001 性能 → 9b86f234 补测 → 53332808 选项 B → 7ea5e481 形状扩展
  → 279dd221 Online package → 48bbc3a5 V002 Track-B → 08d73217 Official 记录
  → 39ac4fdd V002 声明 → 444dedda V002 源码 → fb7a383d V002 正确性
  → c42772d9 V002 性能 → f24dc4ec V002 补测 → 632d5ec4 V003 声明
  → 0c5c90ad V003 源码 → 538f3c1e 精度 STOP → 75f2251e 精度裁定
  → ae35ce2b V003 性能 → 48a52df1 V004 声明 → b94f3809 V004 源码
  → db0a0276 V004 bugfix → b4d55e72 V004 正确性 → 9bc7f40e V004 性能
  → 本文件
```
