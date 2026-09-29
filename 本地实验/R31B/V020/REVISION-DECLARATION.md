# R31B V020 REVISION-DECLARATION — H5 UB 预算核对

Lane: M1-1 / Route R31B
Worktree: `/Users/sunyiyang/Desktop/Project/cann-m1-r31b`，branch `m1/r31b-exploit`
日期：2026-09-29
MAIN_SELECTION: 2026-09-29 选 H5（pass-1 MTE2 staging 2→3-deep）为 V020

```text
ROUTE=R31B
REVISION=V020（声明后终止于前置核对，未写 Candidate 源码）
DIRECT_PARENT=R31B-V017
PARENT_SOURCE_SHA=7c168eafde4c07d2a0667253a06925070e10349ccf08327a1b287302788180c4
SINGLE_HYPOTHESIS=H5 pass-1 MTE2 staging 2→3-deep（xBuf_/residualBuf_ 各加深 1 slot）
CONTEXT_CLASS=HISTORICAL_EXPLOIT
PREREQUISITE=UB 预算核对（MAIN-1 指令：必做，写入本文件）
OUTCOME=INFEASIBLE（见下）
```

---

## 1. 核对对象

`ProcessWideLowPrecision` pass-1 的 2-deep MTE2 staging（V011 引入，`rd0/rd1` + `rel0/rel1` 两组事件）。
H5 要求 `xBuf_` / `residualBuf_` 各从 2 slot 加深到 3 slot，事件 2 组→3 组，载入窗覆盖 u+1 与 u+2。

相关分配（`submission.asc` FP16 / BF16 宽行分支，V017 一致）：

| 缓冲 | FP16 分支 | BF16 分支 |
|---|---|---|
| `xBuf_` | `2 × tileElems × 2B` | `2 × tileElems × 2B` |
| `residualBuf_` | `2 × tileElems × 2B` | `2 × tileElems × 2B` |
| `outputBuf_` | `tileElems × 2B` | `tileElems × 2B` |
| `gammaBuf_` / y 驻留 | `rowWidth × rows × 2B` | `rowWidth × rows × 4B`（y 存 float） |
| `valueFp32Buf_` | `tileElems × 4B` | `rowWidth × rows × 4B` |
| `xFp32Buf_` / `residualFp32Buf_` | 各 `tileElems × 4B` | 各 `tileElems × 4B` |
| `reduceFp32Buf_` | `rows × 16 × 4B` | `rows × 16 × 4B` |

预算函数 `ChooseWideFullYRows(rowWidth, yTypeBytes, ioTiles, ioTypeBytes, workTiles, tileElems)`：
- FP16 调用：`(rowWidth, 2, 4, 2, 3, tileElems)`，ioTiles=4 即 xBuf_+residualBuf_ 共 4 个 half slot
- BF16 调用：`(rowWidth, 4, 4, 2, 2, tileElems)`
- 预算门限 `kWideFullYBudgetBytes = 176 KiB = 180224 B`
- 先试 `rows = 8 … 2`（用当前 tileElems）；都不够则退到 `rows = 1` 并把 tileElems 按 512 递减到放下为止
- **注意**：该函数的 `need` 只计 `y*rows + ioBytes + workBytes + reduceBytes`，**不含 `outputBuf_`**（各分支均如此）

## 2. 预算表（实算，与实测 tileCount 对账）

单位：字节。`rows` 为 `ChooseWideFullYRows` 返回值；`tile` 为该函数实际落定的 `tileElems`；`tileCount = ceil(D / tile)`。

### 2.1 现状（2-deep，ioTiles=4）

| 形状 | rows | tile | tileCount | 公式 need | 实际 alloc |
|---|---|---|---|---|---|
| FP16 D=12288 | 1 | 7680 | 2 | 178240 | 193600 |
| FP16 D=16384 | 1 | 7168 | 3 | 176192 | 190528 |
| FP16 D=32768 | 1 | 5632 | **6** | 178240 | 189504 |
| BF16 D=12288 | 1 | 7680 | 2 | 172096 | 187456 |
| BF16 D=16384 | 1 | 6656 | 3 | 172096 | 185408 |
| BF16 D=32768 | 1 | 2560 | **13** | 172096 | 177216 |

**对账**：V017 实测 tileCount 估计值 fp16-wide≈6、bf16-wide≈13、两个 tail≈2，与上表完全一致 → 预算模型可信。

### 2.2 H5（3-deep，ioTiles=6）

| 形状 | rows | tile | tileCount | 公式 need | 实际 alloc | 对比现状 |
|---|---|---|---|---|---|---|
| FP16 D=12288 | 1 | 6144 | 2 | 172096 | 184384 | tile −20% |
| FP16 D=16384 | 1 | 5632 | 3 | 168000 | 179264 | tile −21% |
| FP16 D=32768 | 1 | 4608 | **8** | 176192 | 185408 | tile −18%，**tileCount 6→8** |
| BF16 D=12288 | 1 | 6144 | 2 | 172096 | 184384 | tile −20% |
| BF16 D=16384 | 1 | 5632 | 3 | 178240 | 189504 | tile −15% |
| BF16 D=32768 | 1 | 2048 | **16** | 172096* | 176192 | tile −20%，**tileCount 13→16** |

\* BF16 D=32768 在 tile=2048 处落到 `while (tileElems > 2048)` 循环出口，以 fallback 返回 1。

### 2.3 关键判据

**判据 A — `wideFullYRows_` 是否从 2 压到 1？**
**否。** 全部 6 个宽行形状在**现状**就已经是 `wideFullYRows_ = 1`。FP16 D=32768 的 y 驻留是 `32768×2 = 65536 B/行`，加上 io+work 163840 B，rows=2 需要 295040 B，远超 180224 B 门限。H5 预注册担心的「驻留行数 2→1 净亏」**不会发生**，因为已经没有第 2 行可压。

**判据 B — UB 放得下吗（当前 tileElems 不变，只 +1 slot）？**
**放不下。** 910B3 UB 总量 192 KiB = 196608 B。按现状实际 alloc 反推，可用上限就是 196608 B（代码里 176 KiB 门限比真实 UB 松 8 KiB，而 `outputBuf_` 恰好落在这 8 KiB 缺口里——现状 FP16 D=12288 实际 alloc 193600 B 已高于 184 KiB=188416 B，说明 8 KB reservation 不从 UB 总量中扣，或扣得比注释少）。

| 形状 | 现状 alloc | headroom | 3-deep@同 tile | 超出 |
|---|---|---|---|---|
| FP16 D=12288 | 193600 | 3008 | 224320 | **−27712** |
| FP16 D=32768 | 189504 | 7104 | 212032 | **−15424** |
| BF16 D=12288 | 187456 | 9152 | 218176 | **−21568** |
| BF16 D=32768 | 177216 | 19392 | 187456 | +10240（唯一放得下） |

主探针 fp16/bf16-wide-d32768 中，fp16-wide **放不下**（超 15424 B）；bf16-wide 仅因其 tile 已被 y=float 预算压到 2560 才勉强放下。三个主形状（fp16 tail/wide、bf16 tail）都放不下。**连 +1 个 slot 都放不下**（fp16 D=32768 单边 +11264 B > headroom 7104 B）。

**判据 C — 若强行用预算函数的 tile 收缩来塞下 3-deep？**
可行，但会把 tileElems 压掉 15–21%，主探针 tileCount 上升（fp16-wide 6→8，bf16-wide 13→16）。这把 **V016 的 tile 加宽轴**回退约一半——V016 当初靠 tile 4096→5632（有效）拿到 fp16-wide −0.68µs，回退一半约合 −0.3µs 代价，而 H5 预期收益 0.2–0.6µs，净值落在噪声带内且**与 V016 轴混杂**，违背 OFAT 单变量原则。

## 3. 结论

```text
OUTCOME=INFEASIBLE
```

按 MAIN-1 指令的停止规则：**「若 UB 放不下 → INFEASIBLE，存档不重试」**。

- 干净 3-deep（同 tileElems 只加深 staging）在主探针形状上 **UB 放不下**（超 15–28 KiB）。
- 唯一能放下的实现必须收缩 tileElems 15–21%，那是 **V016 的 tile 宽度轴**，不是 H5 的 staging 深度轴；做成一版会引入第二变量，结果无法归因。
- 预注册的失败模式（`wideFullYRows_` 2→1 净亏）**未兑现**——因为 rows 本就是 1；真正的代价落在 **tileElems 收缩**上，同样是 UB 挤占（COEFF-LOCALITY-X H2 的前车之鉴同属此类），只是压的是 tile 不是行数。

**不写 Candidate 源码，不做编译/正确性/测量，不创建 submission.asc。** 本文件即存档。

## 4. 建议（供 Main / Planning 选定，本 lane 不自行开 Revision）

1. **H5 按原规格关闭**（INFEASIBLE，不重试同规格）。
2. 若仍想验证「MTE2 装载窗加深」这个机制本身，可行的替代是 **只对 headroom 富余的形状做**（如 bf16-wide-d32768 单形状 3-deep），但那样覆盖面窄、且要接受形状间不一致，信息增量有限；不建议单独占一轮。
3. 更对齐累积证据的方向：V018/V019 已两次否证 V 侧删除，本次 H5 又被 UB 挡住。`ProcessWideLowPrecision` 的 pass-1 节奏指认在 MTE2 装载/reduce 链，但**队列加深走不通**。可考虑：
   - 从 pass-1 的 MTE2 装载路径本身找冗余（非队列深度方向，H5 假设原文已提及这条退路）；
   - 或按 MAIN-1 指令走 `LANE_NEEDS_PLANNING_REVIEW`，把「V 侧删除 2×null + 队列加深 UB-blocked」打包上报，请求规划层给新方向。
4. H4/H6 保持降级（V018 已证中部 PB 近似免费，V 侧删除连续否证）。

## 5. 证据

本文件为该前置核对的完整记录，计算过程可由 `ChooseWideFullYRows`（`submission.asc` L1297）与各分支 `InitBuffer`（L88–99 / L101–114）逐行复算。

```text
本地实验/R31B/V020/REVISION-DECLARATION.md   （本文件）
参照：本地实验/R31B/V017/submission.asc      （预算函数与分配的源）
参照：研究/R31B/next-hypotheses.md H5        （假设原文与 UB 失败模式）
```
