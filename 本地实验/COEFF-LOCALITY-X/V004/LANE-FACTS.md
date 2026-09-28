# COEFF-LOCALITY-X — LANE 事实包

Date: 2026-09-29
ROUTE=COEFF-LOCALITY-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-m2-coeff
BRANCH=m2/coeff-locality
DIRECT_PARENT=FROZEN_R31B_V011 (SHA a8c19a19…, Official 45.16)
用途：供 Planning / Review Layer 决定 LANE_NEEDS_PLANNING_REVIEW（或继续）。
本包只陈述事实与建议，路线生命周期决定权在 Planning。

---

## 1. 本路线做过的全部实验

| 版本 | 假设 | 结果 | 关键证据 |
|---|---|---|---|
| V001 | H1 2-deep param MTE2 prefetch（新增 staging） | LOCAL_REJECTED，1x32768 +19.2% favP | tileElems 4096→2560，tileCount 8→13，staging 抢 tile 预算 |
| V002 | H2 跨 batch stripe residency | NO_UB_BUDGET，未建源码 | tileElems=4096 leftover ~16 KiB < 32 KiB；host clamp → localRows=1 |
| V003 | NH-2 split-phase 发射（零新 staging，tileElems 钉 4096） | LOCAL_REJECTED，large-D +2.1~+2.8% favP | 常量 tileElems 下 param 时序证伪；控制组显示 +1.47% 偏置底 |
| V004 | NH-1 generic 整行预加载（cacheParams 条件扩展） | NEEDS_ONE_MORE_LOCAL | 1x6144 三轮 15/18 favC 但超额仅 ~1.5%；1x8192 证伪；控制组同号偏置 |

## 2. 已证伪 / 已关闭的机制

1. **param MTE2 时序（prefetch / split-phase 发射）**：V001 混杂失败 + V003 在常量 tileElems 下 +2.1~+2.8% 回归方向 → **正式证伪**，large-D 维度收口。
2. **param 驻留（stripe / cross-batch reuse）**：V002 证明在 tileElems=4096 下无 UB 余量；host clamp 使 localRows=1，跨 batch 复用窗口结构性不存在。**关闭**。
3. **generic 整行预加载（NH-1/H3）**：机制真实生效（预加载块被启用），但收益幅度贴近测量装置偏置。1x8192 证伪；1x6144 残留 ~1-2% 不可干净分离。**边际**。

## 3. 合并瓶颈结论（跨路线，REDUCE×3 + COEFF×3）

large-D 主瓶颈 **不是**：reduction V/S（REDUCE-HIER×3）、param MTE2 时序（V003）、param 驻留（V002 预算分析）。
剩余嫌疑人（MAIN-2 范围内）：input x/residual DMA（ASYNC 轴，已 PARK）、output store（EPILOGUE，小）、compute apply 本身（VECTOR-MATH）、mode 选择（MAIN-1）。

## 4. 测量装置事实（供后续路线复用）

- 短 kernel（~5-6 µs，D∈(4096,8192] generic 带）same-binary 难合格：MAD/med 常 0.08–0.13，drift 可 >0.10。需 warmup≥60、samples≥81、sb blocks≥3 才能拿到双侧合格（V004 R3 实践）。
- **控制组必须带**（V003 起）：机制不触发的形状（1x4096 NarrowMid / 1x32768 wide）读出该场偏置底。V004 R3 中 1x32768 控制组显示 -2.77% favC（5/6），证明存在 binary-wide 候选侧偏置——判定任何小收益前必须减掉这个底。
- 三轮中控制组方向不稳（1x4096 从 -5.99% 到 +4.40%），单轮控制底不足为凭，需多轮或 null-binary 对照。

## 5. 资产与残留

- LOCAL_BEST = NONE（四次实验无一 LOCAL_ACCEPTED）。
- OFFICIAL_BEST = V001 = 44.16（ROUTE_OFFICIAL_BEST，FALSE_NEGATIVE 校准已记）。
- CURRENT_CANDIDATE = V004（NEEDS_ONE_MORE_LOCAL，不叠加新变化）。
- 全部失败/证伪版本证据保留于 `本地实验/COEFF-LOCALITY-X/V001..V004/`。
- 残留科学问题：1x6144 的 ~4% 收益是真实小胜还是装置偏置？唯一干净裁决法 = null-binary 对照（parent-rebuild vs parent）。若 Planning 认为不值得再测，该问题可随路线关闭。

## 6. 建议（非决定）

1. **主建议**：COEFF-LOCALITY-X → `LANE_NEEDS_PLANNING_REVIEW`。理由：目标轴（gamma/bias 参数访问局部性）的三个机制维度（时序、驻留、预加载）已全部触及，large-D 已证伪，mid-D 仅剩不可分离的 ~1-2% 残留。继续 Revision 的期望信息增量低。
2. **备选**：若 Planning 要彻底关账，批准一次 null-binary 对照测量（无 kernel 改动，parent-rebuild vs parent，同协议），裁决 1x6144 残留性质后关闭。
3. 不建议：继续在本轴开新假设（load-map 复查已确认 site 2/7 之外无合法空间）。
