# V002 假设声明 — MULTIROW-DMA-CHAMPION-X（待 Main-2 确认，未写 Kernel）

依据：Main-2 V001 Review——V001 LOCAL_REJECTED 确认；V002 要求「保留 2-deep 流水的前提下减少 DMA 事务」；不要重跑 H1 同形态；先交声明确认再动手。父版本回退 FROZEN R31B-V011。

## 声明字段（推荐方案 C1）

| 字段 | 值 |
|---|---|
| ROUTE | MULTIROW-DMA-CHAMPION-X |
| REVISION | V002（待批后创建） |
| DIRECT_PARENT | R31B-V011（FROZEN；V001 LOCAL_REJECTED 不作父） |
| PARENT_SOURCE_SHA | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| PARENT_SCORE | 45.16（Official 15/15） |
| CONTEXT_CLASS | FROZEN_STRONG_BASELINE_TRANSPLANT |
| SINGLE_HYPOTHESIS | **C1 ALIGNED-DATACOPY-FORM**：`Load`/`Store` 在 `count*sizeof(T)` 为 32B 整数倍时改发非 Pad `AscendC::DataCopy`（`DataCopyParams` 形态），非对齐保持 `DataCopyPad`。事务条数、2-deep 流水、UB、所有权、dispatch 全部不动——只改单条搬运指令形态 |
| EXPECTED_SHAPES | 命令密集段全表（LP wide 主场 48×16384 FP16 / 48×12288 BF16 作 P/C 主探针；8×16384 FP16 阴性对照；33×100、2×4096 等小/中对齐形状回归） |

## 候选筛选（Main-2 给的两个方向 + 一个折中形态）

### C1 — ALIGNED-DATACOPY-FORM（推荐，READY）

- 机制：对齐单 burst 从 `DataCopyPad` 换 `DataCopy`（旧 `DataCopyParams` API；本工具链 `DataCopy` 无 `DataCopyExtParams` 重载，V001 Build Fix 已核实）。非 Pad 路径跳过 pad 边界处理，每命令固定成本可能更低。
- 保留 2-deep：完全不动（V011 unit 流、事件、暂存布局字节级不变）。
- UB 推演：0 字节变化。
- API 前置（实现第一步，build-only）：`DataCopyParams` 的 blockLen/gap 单位需实测确认（该结构字段为 uint16，经典语义是 32B 单位；`count*sizeof(T)%32==0` 时 blockLen=字节/32 精确无多读；单位若不符则 C1 降级为记 evidence 的 API 结论，不硬改）。
- 预期与风险：V001 已证明命令形态/条数是敏感面；但 Pad/非-Pad 可能同微码 → 中性。这是「不动流水的事务形态」里 diff 最小、风险最低的单变量。
- WHY_NOT_DUPLICATE：非 H1 stride 多行（不引入 nBursts>1）；非 R015/BATCH-RESIDENT/V003/MODE-X 四形态；不与 ASYNC lane 重叠（不改事件/流水）；不与 DTYPE 重叠（不改 dtype 计算路径）。

### C2 — STRIDE-MERGE-2DEEP（stride 合并且保流水的完整形态）— **BLOCKED，需 Main 决定 UB 计入**

- 机制构想：H1 的 stride 多行（nBursts=B, blockLen=tileBytes, srcStride=(rowWidth-tile)*elem）与 V011 2-deep 并存——暂存按 2×B tile 计（两个在飞窗口各持 B 行）。
- UB 数字推演（FP16，usable=184KB=188416B；`ChooseWideFullYRows` 预算 176KB）：

| 形状 | 现状占用 | 2-deep×B 需要 | 结果 |
|---|---|---|---|
| 16384 B=2 | 155776 | x/res 各 4 tile（+32768）→ 188544 | **超出 128B** |
| 12288 B=3 | ≈164032 | x/res 各 6 tile（+32768）→ 196800 | **超出 ≈8KB** |
| BF16 12288 B=2 | ≈164032 | +32768 → 196800 | **超出 ≈8KB** |

- 折中变体（3-slot 2+1）：x/res 各 3 tile（+16KB），首条 stride 命令覆盖 2 行、第三槽做下窗口单行预取——FP16 16384 B=2 实际 172544 ≤ 188416 放得下；但 `ChooseWideFullYRows` 的 io 计入项必须从 ioTiles=4 改 6，BF16 12288 B=2 校验式 180352 > 180224 会把 B 压到 1、机制失效。
- 结论：完整形态与折中变体都要改 UB 计入（ioTiles 4→6 或压缩 reduce/暂存），并可能改 `wideFullYRows_` 结果。按批准规则第 4 条「若需改 UB 计入方式则停下报 Main-2」——**不自行改，本声明上报决定**：
  - (a) 授权 ioTiles 4→6 并接受部分形状 B 下降 / BF16 主场失效；或
  - (b) 仅 FP16 路径启用 3-slot（BF16 保持现状）；或
  - (c) 放弃 C2，先做 C1。
- WHY_NOT_DUPLICATE：机制仍是 stride 多行事务（非三种历史形态），但与 V001 同机制家族——不重跑 H1 同形态（H1 是 1-deep 缩 B 版；C2 是保流水版，形态不同，触发条件是 Main 授权 UB 计入）。

### C3 — STRIDE-MERGE-PARTIAL（2-slot 不动 UB 的部分合并）— 备选，收益上限低

- 机制：保持 V011 unit 流与 2-deep 事件机；仅当两个 slot 同时空闲时，把同 tile 相邻两行的载入并成一条 nBursts=2 stride 命令（落 [A|B]），其余预取仍单 burst。窗口首对合并、后续单发。
- UB 推演：0 字节变化。命令削减约 10–25%（视 tile 数），流水完整保留。
- 风险：收益上限低（H1 命令减半尚且净负；此形态只减 1/8~1/4 命令且保流水，方向未知）；事件簿记复杂，弄坏预取顺序是正确性风险。
- WHY_NOT_DUPLICATE：非 H1（不全量多行、不 1-deep）、非 H3（仍含 nBursts>1）、非历史三形态。
- 分类：NEEDS_MORE_EVIDENCE（C1 结果出来后再定）。

## 推荐

**V002 = C1 ALIGNED-DATACOPY-FORM**（单变量、流水不动、UB 不动、可立即实现）。C2 等 Main 对 UB 计入的裁定；C3 视 C1 结果再议。

## OFAT 与流程承诺

- 一个概念变化（指令形态）；不动 2-deep/unit 流/暂存/所有权/dispatch/Store 语义。
- 流程：API build-only 探针 → 形态确认后 exact-source compile → link → correctness（全矩阵）→ same-binary（warmup=45，device-event）→ 交错 P/C（主战场 48×16384 FP16、48×12288 BF16、8×16384 FP16 对照）→ 本地结论 → handoff。
- 若 API 单位语义不允许精确非 Pad 传输，记 evidence 并停下报 Main，不硬改。

## PRECISION_RISK

无（同字节搬运；仅指令形态差异）。仍跑全 golden 矩阵确认边界 tile。
