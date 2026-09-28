# HANDOFF-TO-MAIN — COEFF-LOCALITY-X

ROUTE=COEFF-LOCALITY-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-m2-coeff
BRANCH=m2/coeff-locality
FROM=Route Agent（LANE M2-3 = COEFF-LOCALITY-X）
TO=Main-2
DATE=2026-09-29
PHASE=第一阶段（只读研究）— 已完成，等待 Main-2 批准

---

## 1. 本轮做了什么

1. 按序完成启动必读（AGENTS.md、cann-mainline Skill、实验总则/执行约定/本地性能测试规范/服务器实验规范/Git工作流程、路线成绩表与全版本记录中 COEFF 行、研究/COEFF-LOCALITY-X 全部、本地实验 V001/V002、线上结果 V001、main2-r2-route-registry 的 COEFF/large-D 段落、MAIN-2 初始化报告 3.3/4.3）。
2. 判定 V001/V002 闭环。
3. 只读复查 FROZEN R31B-V011 的 UB budget 与 gamma/bias load-map。
4. 写 Track-B 下一轮假设（含 H3 专项判断）→ `研究/COEFF-LOCALITY-X/TRACK-B-HYPOTHESES-NEXT.md`。
5. 本 handoff。

未做（按第一阶段硬约束）：不改 Kernel、不建 V003、不跑正式 performance、不提交 Online、不动共享总账。

---

## 2. 闭环判断

| 版本 | verdict | 闭环？ | 备注 |
|---|---|---|---|
| V001 | LOCAL_REJECTED（1x32768 +19.2% favP 5/0）；Official 44.16 | 是 | 证据齐全；本地/线上 CONTRADICT 已入校准表（`LOCAL_REJECTED_BUT_OFFICIAL_DECENT`） |
| V002 | NO_UB_BUDGET | 是 | 按审批 stop rule 停止，无 SOURCE_SHA，无遗留 |

残留科学缺口：V001 的 prefetch 机制从未在常量 tileElems 下被干净证伪（staging 使 tileElems 4096→2560 混杂）。这正是 NH-2 要补的证据，不是重跑 H1。

---

## 3. UB / load-map 复查结论（要点）

8 个 gamma/bias 访问点逐点复查（详见 TRACK-B-HYPOTHESES-NEXT.md 第 1 节）：

| 位点 | 还有无合法 cache/reuse 空间（不抢 tile 预算） |
|---|---|
| Site 1 / 4 / 5 / 6（每核一次整行缓存类） | 无（已最优）；site 1 仅描述符合并微优化 |
| Site 2 generic `cacheParams=false`（localRows==1 多 tile） | **有**：`gammaBuf_`/`biasBuf_` 已按 8192 分配，整行预加载 UB 增量 0、tileElems 不变、MTE2 变少 |
| Site 3 NarrowMid | 仅发射顺序微调，无 cache 空间 |
| Site 7 wide FP32 pass 2 | **cache/驻留维度无空间**（见下） |
| Site 8 wide lowp（已有 2-deep prefetch） | 无（in-kernel 最优） |

Site 7 预算算术（tileElems=4096 钉死）：D=32768 leftover 16320 B、D=16384 leftover 16256 B，均 < K=1 stripe 32768 B，甚至 < 单 gamma tile 16384 B（差 64/128 B，正好是 reduce partial 占用）。host 启动 `blockCount = min(cores, rowCount)` → 测量形状 localRows=1 → 无跨 batch 命中窗口。**H2 的失败是预算与 host 约束的联合结果，不是实现问题。**

---

## 4. 下一轮 3–5 假设（均过 UB budget 与 tile-shrink 审查）

| 编号 | 一句话 | UB 增量 | tileElems | MTE2 | 成熟度 |
|---|---|---|---|---|---|
| NH-1（=H3 精化） | generic 单行多 tile 整行参数预加载 | 0 | 0 | 减少 | READY_FOR_MAIN_REVIEW |
| NH-2（推荐首选） | wide FP32 pass 2 split-phase 发射，现有两 slot 轮转，常量 tileElems | 0 | 0 | 次数不变 | READY_FOR_MAIN_REVIEW |
| NH-3 | NarrowMid 参数 MTE2 与输入并行发射 | 0 | 0 | 次数不变 | NEEDS_MORE_EVIDENCE |
| NH-4 | 预加载块描述符合并 | 0 | 0 | 减少 | DUPLICATE（并入 NH-1 实现） |

明确排除：H1 重跑（任何新增 staging slot）、H2 无预算 stripe、multi-row/rows-per-block、wide 分发切换、同步消除作为性能变量、store/epilogue/row-scheduling/reduction 拓扑。

---

## 5. H3 判断

**技术上成立，天花板确实小；不建议作主攻，建议作收口探针或 NH-2 之后的顺手项。**

- 能过审：site 2 是唯一还免费的 cache 空间（缓冲区已在，tileElems 不受预算函数支配）。
- 天花板小：只覆盖 D∈(4096, 8192] 且 localRows=1 的 generic 路径；Official case 14（约 4.4× 差距）走 wide FP32，H3 够不着主差距。
- 价值：便宜、零预算风险、可留本地小胜；若 NH-2 把 site 7 证伪，H3 是本轴仅存的正收益机制；不做则 generic 维度闭环不完整。

替代主攻：**NH-2 优先**（补常量 tileElems 证据，直接打 large-D），NH-1 次之。

---

## 6. 请求 Main-2 决定

1. 是否批准 **NH-2** 作为 V003 的单一假设（推荐）；或改批 **NH-1 / H3 精化**。
2. 若判定 NH-2 仍属 H1 范畴（尽管无 staging 增量），则按排除表处理，COEFF 轴在 large-D 维度收口，只留 NH-1 可选。
3. 批准前不建 V003、不改 Kernel、不跑正式 performance、不提交 Online。

详细依据：`研究/COEFF-LOCALITY-X/TRACK-B-HYPOTHESES-NEXT.md`。
