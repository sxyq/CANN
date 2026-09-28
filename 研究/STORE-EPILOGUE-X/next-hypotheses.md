# STORE-EPILOGUE-X — 下一轮假设（TRACK-B，只读研究）

Date: 2026-09-29
Route: STORE-EPILOGUE-X · Worktree `cann-m1-store` · Branch `m1/store-epilogue`
Parent of record: V002（LOCAL_BEST，SOURCE_SHA `59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839`）
组合基点可选：FROZEN_R31B_V011（SHA `a8c19a19…b15e3`）+ V002 叠加后的输出侧现状。
本轮无 kernel 改动、无新 Revision、无设备计时、无共享总账改动。

## 0. 现状（只从证据读取）

V002 变更域只有一个函数的输出遍：`ProcessWideFp32FullCacheRows`（源码约 `:2082-2295`）。

- 判定条件 `mergeRowRuns = tileCount>=4 && (rowWidth % 8 == 0)`（`submission.asc:2179`）。
- 条件成立：每行一次 `Store`，长度 `rowWidth`（`:2266-2277`），2-deep ring 每行占一格。
- 条件不成立：父版按 tile 写回块原样保留（`:2215-2246`）。
- 该函数只在 wide FP32（rowWidth > 8192）到达；tileWidth = 4096，`tileCount = ceil(rowWidth/4096)`。
- 实测（V002 support）：合并条件命中仅 1x32768（t=8）→ −5.62% 6/0；其余形状逐字节同父版，差值即窗口噪声。

已知空档（喂给下面假设，不属 V002 收口范围）：

| 空档 | 说明 |
|---|---|
| t=4 边界未测 | 1x16384 FP32 命中合并但从未 P/C 计时（测过的是 1x16384 FP16，走另一条路径） |
| 多行 wide 未测 | 2x16384 / 8x16384 / 2x32768 等命中合并且 batchRows>1，未计时 |
| case 14 | Official 最大缺口（4.4x，16443 vs 3750 µs），推断为 wide-D 多行、store 流量最大 |
| 同父版两次 max_abs 不一致 | 1x32768 FP32 父版 V001 轮 1.203099 / V002 轮 1.260115，bad 计数也漂 → 参照 golden 比较本身有非确定性，属 MAIN-1 开放项（参照/runner 质量），非候选缺陷 |

写回侧仍然存在的可改形态（均在允许边界内）：

1. 整行合并把所有 store 推迟到输出遍 tile 循环结束之后，失去了父版「store 与后续 tile 计算重叠」的时间窗（V001 中带 t=2 退化正是这个代价；t=8 时描述符收益更大所以净赚）。
2. 输出遍每个 tile 开头对两个 ring 槽做 `WaitFlag<MTE3_V>`（`:2191-2201`）再把 xBuf_/residualBuf_ 当 gamma/bias 用——但本函数 `Store` 的源是 `valueLocal`（valueFp32Buf_），不是这对 staging，此处是假依赖。
3. 每个 batch 开头 `AllocEventID` ×4、结尾 `ReleaseEventID` ×4（`:2154-2167`、`:2288-2291`）；batchRows=2 时多行大形状的 batch 次数多，这笔逐批开销反复支付。
4. `rowWidth%8==0` 是全有或全无：行首 32B 对齐只与 `batchRow*rowWidth*4` 有关，rowWidth 非 8 倍数时首行仍可合并。

---

## H1 — 分块合并写回（拆分+输出遍内提早发出）

**Classification: READY_FOR_MAIN_REVIEW**（首选）

```text
MECHANISM
  把一行的连续输出 run 拆成 2 个（或 K 个）块；输出遍的算术按块完成：
  先算完块 0 的 tiles（Muls/Mul/Add 不动），立刻为块 0 发一次合并 Store，
  同时继续算块 1 的 tiles，再发块 1 的 Store。仍用现有 2-deep ring，
  每次 Store 的 SetFlag/WaitFlag 形态与 V002 相同；块内仍是「多 tile → 一次写回」。
  块边界取 tile 的整数倍（例如 4 tiles = 16 KB 一块），不跨行。

BOTTLENECK
  V002 整行合并把 MTE3 全部推到算术结束之后：输出遍期间 MTE3 空转，
  store 时长（大行 128 KB）完全暴露；父版按 tile 写回反而有 ring 重叠。
  t=8 时描述符收益（8→1）盖过了这个暴露所以净赢，但 case 14 量级下
  store 字节巨大，暴露时长是主项之一。

EXPECTED_SHAPES
  合并条件命中的 wide FP32：1x32768（主）、1x16384、2x16384、8x16384、2x32768 等；
  行越长、行数越多收益越大。t<4 不受影响（仍走父版块）。

WHY_IT_MAY_HELP
  每行描述符从 tileCount 降到 K（32768 行 K=2 时 8→2，仍省 75%），
  同时恢复 store 与后半段算术的重叠窗。两个收益来源相加，
  优于 V002 的「只省描述符、丢掉重叠」单边。

WHY_IT_MAY_FAIL
  块 0 的算术部分缩短了可重叠对象（只剩块 1 的 tiles）；若块 1 计算太快，
  store 还是没跑完就到下一批的 drain，重叠窗不够。
  或描述符已不是瓶颈（问题在算术/MTE2），则 K=2 与 K=1 差值落在噪声内。
  另外块 0 store 未完成时不能动 valueLocal 对应区——分区活性要按块管，
  实现上比整行合并多一层边界推理。

WHY_NOT_DUPLICATE
  vs V002：V002 是整行 1 次写回、全在 tile 循环之后；本假设改的是写回形态
  （拆分粒度）与发出时机，算术、归约、ring 深度/旗帜/等待点数量都不动。
  vs V001：V001 是「不拆分的合并 + 多站点铺开」，且 t=2 也合并（中带已退化）。
  vs R31A V021：V021 是按 tile 写回不变、把每 tile 的 MTE3→V 完成等待推迟到
  整行输出 tiles 结束（同步点后移）；本假设不移动任何等待，只改 run 的
  切分与 Store 发出位置（描述符数与写回形态轴）。
  vs ASYNC-TRIPLE-X：不改 ring 深度、不改旗帜语义、不改每格 in-flight 数量。
  vs R015 多行合并 DMA：块严格留在行内，绝不跨行。
  vs ALIGN-TAIL-X：不改 DataCopy/DataCopyPad 选型，不拆 bulk/tail 路径。
  vs EPILOGUE-ARITH-CHAMPION-X：Muls/Mul/Add 每 tile 的次数与顺序不变。

MINIMAL_OFAT_DIFF
  仅在 mergeRowRuns 分支：把「tile 循环全部结束后按行 Store」改成
  「tile 循环内按块边界发块级合并 Store」。一个概念变量 = 写回块粒度与发出点。

ASCEND_FEASIBILITY
  高。Store 助手与事件 API 不变；块起点 = tile 起点 = 4096 元素倍数，
  行首对齐条件（rowWidth%8==0）继续保证每块起点 32B 对齐。
  参照父版已有单次 32 KB 写回先例（ProcessSmallFp32ContiguousBatched）。

UB/DMA/SYNC_IMPACT
  UB 不变（仍整行驻留 valueFp32Buf_，总字节不变）。
  DMA 同字节数，描述符数 tileCount→K，每块 1 次 SetFlag/WaitFlag 对，
  ring 仍 2-deep。同步点数量随 K 线性，K=2 时比 V002 多 1 对、比父版少得多。
  块活性：块 0 的 UB 区在块 0 Store 完成前不得被覆写——输出遍内只追加算术
  不覆写他块，天然满足；批次边界 drain 保持 V002 原样。

PRECISION_RISK
  无。同值、同算术顺序、同 pad 语义；只是写回次数与时机变化。
```

预期量级（先登记再测）：1x32768 在 V002 的 −5.62% 之上再吃一部分暴露 store 时长；
是否超噪声要靠配对测量判定，不设固定百分比。

---

## H2 — tileCount 断点重定（条件常数调优）

**Classification: NEEDS_MORE_EVIDENCE**（先补边界测量，再改常数）

```text
MECHANISM
  V002 的合并门槛 tileCount>=4 来自 V001 的方向（t=2 中带净亏、t=8 净赚），
  但 t=3/4/5 从未测过（1x16384 FP32 恰好 t=4 命中合并且没计时）。
  先用现有 V002 二进制对父版补 t=3/4/5 的 P/C，找到实测断点 K*，
  再把常数 4 改成 K*（单常数改动）。若 t=4 已明显净赚，可能下调到 3
  以覆盖 12288 档；若 t=4 压线或亏，上调到 5，把 16384 档留给父版块。

BOTTLENECK
  合并的净收益 = 描述符节省 − 延迟写回的重叠损失，两者都随 tileCount 变。
  常数 4 是单点外推，断点形状（1x16384 = t=4）恰好没有本地证据。

EXPECTED_SHAPES
  断点带：1x12288（t=3）、1x16384（t=4）、1x20480（t=5）FP32；
  校准形状 1x32768（t=8）作上限对照。

WHY_IT_MAY_HELP
  门槛贴合实测断点后：t=4 若净赚而常数是 4，无需改也能确认覆盖正确；
  若断点其实在 3 或 5，改一个常数就能吃下 12288 档或避开 16384 档的小亏，
  风险极低、可归因性极强。

WHY_IT_MAY_FAIL
  断点带差值可能全落在噪声内（短 kernel ±5~10%），无法区分 3/4/5，
  只能保持 4 并承认边界未知；或 case 14 的 tileCount 远大于断点，
  门槛怎么调都不影响主要目标。

WHY_NOT_DUPLICATE
  vs V002：V002 引入常数 4 但没有断点测量；本假设的变量是常数取值本身
  （调优轴），不是写回形态。vs H1：H1 改写回粒度/时机，本假设只动判定常数。
  vs V001：V001 无判定（t>=1 全合并）。
  vs 其他路线：纯输出侧条件调优，不触算术/归约/调度/tiling。

MINIMAL_OFAT_DIFF
  补测量（不动代码）→ 单常数改动 `tileCount >= K*`。一次只改一个数。

ASCEND_FEASIBILITY
  极高。纯常数；测量用现有 V002 与父版可执行文件即可。

UB/DMA/SYNC_IMPACT
  无（只改变哪些形状走合并分支）。

PRECISION_RISK
  无（两条分支各自已正确）。
```

---

## H3 — 行首对齐判定放宽（合并条件加宽）

**Classification: NEEDS_MORE_EVIDENCE**

```text
MECHANISM
  现条件 rowWidth%8==0 是「全批所有行都 32B 对齐才合并」。
  实际约束是每个 UB run 起点：offset = batchRow*rowWidth*4 字节，
  与 32 对齐只在 rowWidth%8==0 时对所有行成立；rowWidth%8!=0 时
  首行（offset=0）仍合法。改成逐行判定
  `((batchRow*rowWidth*4) % 32 == 0)`，合法行合并、非法行回退父版块。

BOTTLENECK
  全有或全无判定让奇数步长行宽的形状整体放弃合并，
  即使其中若干行完全可以一次写回。

EXPECTED_SHAPES
  wide FP32 且 rowWidth 非 8 倍数（例如 8192+256=8448、10000 一类）；
  官方 case 是否存在此类宽度未知，需从 runner 形状表核对。

WHY_IT_MAY_HELP
  若官方含非对齐宽行，当前是 0 合并，放宽后首行（或多行批中的合法行）
  直接吃到 H2B 的描述符收益。

WHY_IT_MAY_FAIL
  官方 15 个 case 的宽度可能全是 2 的幂（256/…/32768），此条件永不触发
  → 代码白加、差值 0。DataCopyPad 对非对齐 src 若另有隐含限制，
  逐行判定也要保留 32B 起点约束（不能只看 count）。

WHY_NOT_DUPLICATE
  vs V002：V002 的判定是批级全量；本假设把判定粒度从批级降到行级，
  写回形态（整行 1 次 Store）不变。vs ALIGN-TAIL-X：不选拷贝原语、
  不拆 bulk/tail；只是合法行集合变大。vs R015：仍不跨行。
  vs H1：H1 改 run 切分；本假设改合法行集合。两者不叠加在同一次 Revision。

MINIMAL_OFAT_DIFF
  只改 mergeRowRuns 的计算：批级布尔 → 每 batchRow 一个布尔；分支体不动。

ASCEND_FEASIBILITY
  高。纯判定；需先确认 DataCopyPad 对 src 起点的要求（现有证据：起点须 32B 对齐）。

UB/DMA/SYNC_IMPACT
  无（合法行的写回形态与 V002 相同）。

PRECISION_RISK
  无（非法行走父版原样；合法行 pad 语义与 V002 相同）。
```

---

## H4 — 去掉输出遍的假 MTE3_V 等待（条件不成立路径的写回等待点微调）

**Classification: NEEDS_MORE_EVIDENCE**（先做别名/活性核验）

```text
MECHANISM
  ProcessWideFp32FullCacheRows 输出遍每个 tile 开头把 xBuf_/residualBuf_
  当 gamma/bias 用之前，等两个 ring 槽的 MTE3_V（:2191-2201 注释写
  「drain any store still referencing the previous tile's staging pair」）。
  但本函数 Store 的源是 valueLocal = valueFp32Buf_（:2224/:2272），
  与 xBuf_/residualBuf_ 是不同 InitBuffer；MTE3 从不读这对 staging。
  把这两段 WaitFlag 拿掉（或仅在真正从 staging 写回的路径保留），
  gamma/bias 的 MTE2 装载即可与在飞 store 并行。

BOTTLENECK
  假依赖串行化：每个 tile 的参数装载被上一 tile 的 store 完成时间卡住，
  把 store 延迟灌进输出遍的参数流。父版按 tile 写回路径（tileCount<4
  或未对齐时 V002 原样走的那段）每 tile 都付这笔等待。

EXPECTED_SHAPES
  到达本函数且走父版块的形状：wide FP32、8192<rowWidth<=12288（t=3）
  或未对齐行宽；以及未来若门槛上调后 t=4 附近的形状。
  1x32768（合并分支）不受影响（合并分支循环内不发 store，等待不触发）。

WHY_IT_MAY_HELP
  参数面板装载与 store 重叠后，输出遍每 tile 的等待段消失；
  对走父版块的宽行是纯等待点缩减，不改任何写回形态。

WHY_IT_MAY_FAIL
  核验后若发现别名或历史路径里 Store 源确实覆盖过 xBuf_（通用 Process
  路径 outputLocal 可取 xBuf_，但那不是本函数），等待就是真依赖，不能删。
  或本函数条件不成立时 tileCount<=3，tile 数少，收益绝对值小、测不出。

WHY_NOT_DUPLICATE
  vs R31A V021：V021 是把每 tile 写回的 MTE3→V 完成等待推迟到整行结束
  （同一类等待后移，在 R31A-V016 的 CachedRows 输出遍、不同父系）；
  本假设删的是「store 完成 → staging 复用」的假等待，等待对象不同
  （MTE3_V→MTE2 装载 vs MTE3_V→V 写回槽），站点不同（FullCacheRows
  输出遍参数段 vs CachedRows 按 tile 写回段），且 V021 的等待后移
  在 V002 路径上本来就不需要（合并分支循环内无 store）。
  vs ASYNC-TRIPLE-X：不改 ring 深度/旗帜/每格 in-flight 数，只删一处
  与真实数据来源无关的等待。vs MIX-A 的 SyncVToMTE2 删除：那是 MTE2 侧
  等待，这里是 MTE3_V→staging 假依赖。
  vs H1/H2/H3：不改写回形态与判定，仅动等待点；不与它们同 Revision。

MINIMAL_OFAT_DIFF
  删输出遍 tile 循环开头两个 `if (storeOutstanding*) WaitFlag<MTE3_V>` 块
  （合并与不合并两分支共用的那段）；其余不动。

ASCEND_FEASIBILITY
  中。需先核验：(1) valueFp32Buf_ 与 xBuf_/residualBuf_ 无别名（Init 处确认）；
  (2) 本函数 Store 源只取 valueLocal；(3) 删等待后 gamma/bias 装载与
  在飞 store 并行不产生 UB 覆写竞争。三条都成立才可实现。

UB/DMA/SYNC_IMPACT
  UB/DMA 不变；同步点每 tile 少 2 次 WaitFlag（两分支共用段）。
  ring 活性不变（store 源 valueLocal 在批结束 drain 前不被覆写）。

PRECISION_RISK
  无（不改数值路径）；风险全在活性推理，正确性先行。
```

---

## H5 — store ring 事件生命周期上提（逐批 Alloc/Release 摊薄）

**Classification: NEEDS_MORE_EVIDENCE**

```text
MECHANISM
  ProcessWideFp32FullCacheRows 在每个 batch 开头 AllocEventID ×4
  （V_MTE3 ×2 + MTE3_V ×2）、结尾 drain 后 ReleaseEventID ×4。
  把这 4 个事件的分配/释放上提到函数级（batch 循环外），
  批内仍 drain 后进入下一批，每次 Store 的 SetFlag/WaitFlag 原样不动。

BOTTLENECK
  batchRows 通常 2（kWideFp32FullCacheRows=2），行数多的 wide 形状
  batch 次数 = 行数/2；每批 4 次事件分配 + 4 次释放是纯管理开销，
  在 case 14 量级（行数巨大）反复支付。

EXPECTED_SHAPES
  多行 wide FP32（2x16384、8x16384、2x32768、更大行数的官方大 case）；
  1x32768 只有 1 批，收益≈0（对照）。

WHY_IT_MAY_HELP
  每批省 8 次事件管理调用；批数多时是稳定的指令侧节省，
  与 H1/H2 的写回形态正交，可单独归因。

WHY_IT_MAY_FAIL
  事件管理成本相对 kernel 时间可能只是噪声（每批纳秒~微秒级）；
  或 AscendC 事件分配本身有隐含的每批状态要求（上提后跨批复用
  事件 ID 是否合法需查 API 约束）。测不出就是零收益，不亏。

WHY_NOT_DUPLICATE
  vs ASYNC-TRIPLE-X：他们拥有 ring 深度、旗帜语义、每格 in-flight 数量、
  等待点位置；本假设四项全不动，只改事件对象的分配/释放次数
  （生命周期管理轴）。vs V002：V002 保持逐批 Alloc/Release 原样。
  vs R31A V021：无关（那是等待点后移）。vs H4：H4 删假等待，本假设动
  事件管理；不同 Revision。

MINIMAL_OFAT_DIFF
  把 4 个 AllocEventID 移到 batch 循环前、4 个 ReleaseEventID 移到循环后；
  批末 drain 保留，确保下一批开始时两槽空闲。一个概念变量 = 事件生命周期。

ASCEND_FEASIBILITY
  中高。需确认 AscendC 硬事件 ID 总量约束（本 kernel 只占 4 个，余量足）
  与跨批复用合法性；API 层面 Alloc/Release 是配对管理，上提后语义应保持。

UB/DMA/SYNC_IMPACT
  UB/DMA 不变。同步点（SetFlag/WaitFlag）逐 Store 原样；少的是管理调用。

PRECISION_RISK
  无。
```

---

## 汇总

| ID | 机制一句话 | 成熟度 | 主打形状 | 与 V002 的变量关系 |
|---|---|---|---|---|
| H1 | 行 run 拆成 K 块、块级合并写回并提早发出 | READY_FOR_MAIN_REVIEW | 1x32768 / case 14 类多行 wide | 写回形态（拆分粒度）+ 发出时机 |
| H2 | 实测 t=3/4/5 断点后调 tileCount 门槛常数 | NEEDS_MORE_EVIDENCE | 1x12288 / 1x16384 / 1x20480 | 判定常数取值 |
| H3 | 行首对齐判定从批级降到行级 | NEEDS_MORE_EVIDENCE | 非 8 倍数宽行（存在性待查） | 合法行集合 |
| H4 | 删输出遍 staging 假 MTE3_V 等待 | NEEDS_MORE_EVIDENCE | 条件不成立宽行（t<=3 / 未对齐） | 等待点（假依赖） |
| H5 | ring 事件 Alloc/Release 上提到函数级 | NEEDS_MORE_EVIDENCE | 多行 wide、batch 数大 | 事件生命周期管理 |

推荐顺序：H1（机制最大、可归因清楚）→ H2 的边界测量（可与 H1 的测量窗口
合并安排）→ H4（先完成三条核验再动）→ H5 → H3（先查官方宽度分布）。

禁止事项复核（本轮 5 个假设均满足）：不改归约/算术顺序、不做 dtype 分路、
不改 row/block 调度与 tiling 策略、不跨行合并（R015 轴）、不选拷贝原语
（ALIGN-TAIL 轴）、不与 EPILOGUE-ARITH-CHAMPION-X 的 RMS 后算术链重叠。

## V003 草稿声明（待 Main 选定假设后填实；本轮不写代码）

```
ROUTE=STORE-EPILOGUE-X
REVISION=V003
DIRECT_PARENT=V002
PARENT_SOURCE_SHA=59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839
PARENT_SCORE=LOCAL -5.62% (1x32768, 6/0) / Official 45.07
SINGLE_HYPOTHESIS=<Main 从 H1-H5 中选定的一个>
CONTEXT_CLASS=FROZEN_STRONG_BASELINE_OUTPUT_STORE
WHY_NOT_DUPLICATE=<按选定假设引用上文对应段>
```

Main 一次只批一个假设；Route Agent 不自行从本列表开写。
