# REVERT + LANE_NEEDS_PLANNING_REVIEW 事实包 — REDUCE-HIER-X V005

Date: 2026-09-29
Trigger: V005 LOCAL_REJECTED（N3 ReduceSum→WholeReduceSum）
Per: MAIN-2 指示 —「LOCAL_REJECTED 则证据保留 + 显式回退 FROZEN_R31B_V011，并写
LANE_NEEDS_PLANNING_REVIEW 事实包」

---

## 1. 回退状态

```text
CURRENT_SOURCE      = FROZEN_R31B_V011
CURRENT_SOURCE_SHA  = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
V005_SOURCE_SHA     = 5fa325263d18fc1aa49a7ac0c1130e7cceb5948e4423bba2ee10e6e2f6bfdd0d（失败证据保留）
V005_LOCAL_VERDICT  = LOCAL_REJECTED
```

- 失败证据全部保留于 `本地实验/REDUCE-HIER-X/V005/`（声明、源码、diff、build、
  correctness、完整 raw timing 172 文件、API-usage 修复痕迹）。
- server3 工作区 `submission.asc` 已覆盖回 parent（SHA 核对 = a8c19a19…）。
- 未改共享成绩记录（由 Main-2 登记）。lease 已释放。

## 2. LANE_NEEDS_PLANNING_REVIEW 事实包

### 2.1 归约结构轴五次独立证伪（完整对照）

| Rev | 假设 | 单变量 | 主探针结果（device-event 配对中位数） | 本地结论 | 根因 |
|---|---|---|---|---|---|
| V001 | H1 eager fold | partial 折叠时机（去 collapse） | 8x8192 -3.8%（n=2） | NEEDS_ONE_MORE_LOCAL | collapse 非主要成本 |
| V002 | H2 手写 Add 树 | per-tile 归约 primitive | 1x32768 **+6.6%**（5/0 favP） | LOCAL_REJECTED | per-level PipeBarrier 链 > 省下的 V/S |
| V003 | H3 短 span | 归约 span（折到 128） | 1x32768 -0.5% 中性 | NEEDS_ONE_MORE_LOCAL（Official 44.24） | 短 span 无大胜 |
| V004 | N1 分箱单级 | 归约结构（两级→单级） | 1x32768 **+23.9%**、1x16384 **+21.2%**（各 6/0 favP） | LOCAL_REJECTED | 手工 Add 指令数 > 省下的调用 |
| V005 | N3 primitive 替换 | 归约 primitive + 后处理链 | 1x32768 **+7.5%**（5/6 favP）、1x16384 **+8.1%**（6/6 favP） | LOCAL_REJECTED | 两段式 WRS 的额外 vcadd > 省下的同步链 |

共同结论：**reduction structure / span / primitive 在 FROZEN_R31B_V011 谱系已无赢面。
父版的 `ReduceSum`（一条 vcadd 进硬件 acc + 标量回路）在 dav-2201 上接近最优形态。**

### 2.2 V005 新增的硬件事实（可复用）

1. **`vcadd` mode=0 输出粒度 = 每 natural repeat（64 FP32）一个和**，不是任意 count
   一个总和。mode=1（acc）才能跨 repeat 累到 `get_acc_val`。因此「一次调用换一次调用」
   的纯 primitive 交换在 span>64 时不可实现；必须两段式（先 64 元粒度 partial，再收尾）。
2. **`ReduceSum`（dav-2201 `ReduceSumImpl`）每次调用 = vcadd(mode=1) + V→S 排空 +
   `get_acc_val` + S→V + S→MTE3**（3 硬同步 + 标量寄存器读 + 与 store 管线耦合）——
   工具链源码实锤。
3. **两段式 `WholeReduceSum`（生产 rms_norm `ReduceSumHalfInterval` 模式）净收益为负**：
   每 tile 2 条 vcadd（无同步）vs 父版 1 条 vcadd + 3 同步 + 标量回路，在主探针上稳定
   慢 7–8%。同步链的代价低于额外 vcadd 的代价。
4. 各机制净效果：手工 Add 分解（V002/V004）大败；缩短 span（V003）中性；两段式向量
   归约（V005）小败。没有第四条路能在「每 tile 一次归约」的结构下击败父版。

### 2.3 对 Planning 的事实陈述（非建议决定）

- 本 Lane（REDUCE-HIER-X）的归约拓扑轴已用尽：5 个 Revision、3 个轴（fold 时机/
  span/primitive）全部测过，2 胜（V003 中性 + V001 弱信号）3 败（V002/V004/V005），
  无一超过噪声成为 LOCAL_ACCEPTED。
- 该路线唯一留下的实测痕迹是 V003 的 Official 44.24（低于 anchor 45.16）。
- Track-B 残余假设 N2（group-span）与 N4（多行同时归约）先验已极低：
  N2 与 V003 同轴（span）且方向相反但 V005 显示指令数是主导项；N4 的形状带
  （短行多行）与主探针不同，但其结构（多行一次归约）在 N4 立项前仍无正面证据。
- 五次实验共同指向：large-D 主瓶颈不在 reduction 侧（与 registry 既有结论一致）。

### 2.4 请 Planning 决定（本 Lane 状态 = LANE_NEEDS_PLANNING_REVIEW）

1. REDUCE-HIER-X 的路线生命周期（KEEP / PARK / CLOSE / 合并）。
2. 若 KEEP，剩余可走方向只有 N4（短行多行带，未测）或与其他路线组合使用本路线的
   donor 知识；两者都需要 Planning 明确授权。
3. 本 Lane 的 5 个 Revision 证据包（V001–V005）与 E3 工具链契约记录完整保留，供
   后续路线作 donor（尤其 V005 的 vcadd mode 语义与 ReduceSumImpl 同步链事实）。
