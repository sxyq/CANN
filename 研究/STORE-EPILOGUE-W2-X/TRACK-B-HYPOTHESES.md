# STORE-EPILOGUE-W2-X — Track-B handoff

- 日期：2026-09-30
- 直接父版本：`STORE-EPILOGUE-X V002`
- `PARENT_SOURCE_SHA`：`59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839`
- Official anchor：45.07
- 分支：`w2/m1/store-epilogue`
- 状态：`MAIN_SELECTED=YES`；选择 H1，Revision `V001` 已声明。
- `CHILD_RECOMMENDED_HYPOTHESIS`：Track-B 初始建议 H4 先做可达性判断；该建议已被 Planning 对 H1 的正式选择取代。

本 handoff 初始阶段只写路线研究。V003 是 Wave-1 供体证据，不是父版本。Main 收到 Planning 选择后，才声明 V001；源码实现与实验状态另见本路线 Revision 记录。

## 范围与父版

- `ROUTE_BOUNDARY`：V002 `ProcessWideFp32FullCacheRows` 中的 Store 发出位置、最终 MTE3 drain、Store event 依赖、行内 chunk 边界/末段长度、现有整行 Store 的启用条件。
- `ALLOWED_CHANGES`：只研究该函数内单行 Store 的 issue 点、行内 chunk 形态、Store 依赖等待，或基于已有 `tileCount`/`batchRows` 的启用 predicate；保留既有 tile 几何、event ring 深度和行归属。
- `FORBIDDEN_CHANGES`：reduction/算术/舍入/dtype；跨行或 stride 多行 DMA；tile 宽度、tiling、block 数、row scheduling/row ownership；Store helper/拷贝原语；其他函数或共享 ledger。H3 仅能调整行内 chunk Store 次数与边界。
- `OPEN_QUESTIONS`：H4 的 `128x12288 FP32` 在现有 runner/tiling 下是否得到 `batchRows>1`；H1 的 issue 空隙是否大于噪声；H3 的 6+2 是否能胜过 V003 的 4+4；目标 Official case 与本地 shape 的对应关系仍未知。

- V002 在 `ProcessWideFp32FullCacheRows` 中使用 `tileCount>=4 && rowWidth%8==0`；命中时每行发出一次整行 Store。`1x32768 FP32` 本地中位数 -5.62%，6/6 配对方向有利；Official 45.07。父源码文件 SHA 与任务给定值一致。
- V003 使用 K=2 分块；本地 `8x16384 FP32` 为 -5.64%（6/0），`1x32768 FP32` 为 -3.15%。Official 44.38，较 V002 低 0.69。仅作 donor evidence。
- V002 的函数尾 drain 后立即释放 event ID，没有后续工作可隐藏等待；V002 的逐 tile 分支已有 Store 前同 slot wait。
- 既有 gap 在 commit `acc92000` 的 `研究/STORE-EPILOGUE-X/next-hypotheses.md`；handoff 在 commit `76cbb94d` 的 `研究/STORE-EPILOGUE-X/handoff-2026-09-29.md`。旧 H4 与本轮 H5 依赖等待种子相同，详见下文。

## H1 — 整行 Store 发出点

- `HYPOTHESIS_ID=STORE-W2-H1-ISSUE-POINT`
- `STATUS=PLANNING_SELECTED_MAIN_REVIEWED`
- `MECHANISM`：合并条件、整行 Store 次数及所有 event 操作不变；把唯一整行 Store 从 tile 循环之后移到最后一个 tile 的算术和 `PIPE_V` barrier 之后、循环退出之前。
- `BOTTLENECK`：最后一次 Store issue 前的循环退出指令可能推迟 MTE3 启动。
- `DIRECT_PARENT`：V002，SHA `59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839`。
- `TARGET_SHAPES`：`1x32768 FP32` 主探针；`8x16384 FP32` 复验。`1x16384 FP32` 暂不测，V002 记录为父子共同 golden mismatch。
- `TARGET_DTYPES`：FP32。
- `WHY_IT_MAY_HELP`：循环退出指令可与同一笔异步 Store 重叠，减少 issue 空隙。
- `WHY_IT_MAY_FAIL`：最后 tile 后没有后续算术可供重叠，省下的指令可能低于测量噪声。
- `UB_IMPACT`：无。
- `DMA_IMPACT`：每行一次、相同长度的 Store；字节数与调用数不变。
- `SYNC_IMPACT`：wait、flag、event ID 和 drain 顺序不变，仅移动 Store issue 点。
- `PRECISION_RISK`：数值路径不变；须保证所有行最后 tile 的算术完成后才 Store。
- `DUPLICATE_CHECK`：与 V003 不同；V003 将一行改为两次 Store，本项保留 V002 的单次整行 Store。与旧 gap 的 chunk 机制也不同。
- `RELATED_OLD_ROUTES`：STORE-EPILOGUE-X V002/V003；ASYNC-OVERLAP-CHAMPION-X 仅作流水时序相邻路线。
- `MINIMAL_OFAT_DIFF`：只移动 merge 分支中的整行 Store 代码块；不改条件、循环顺序、调用数或 event 操作。
- `MINIMAL_EXPERIMENT`：基于 V002 只做上述代码移动；先跑 `1x32768 FP32` correctness 与 same-binary，再按现行协议做至少 4 组相邻交错 P/C。
- `EXPECTED_LOCAL_PROBES`：`1x32768 FP32` 主测，`8x16384 FP32` 复验；每个形状独立通过 same-binary 后才配对。
- `UNCERTAINTY`：高；没有 Store issue-gap 的独立测量，理论收益偏小。
- `EVIDENCE_PATHS`：`线上结果/STORE-EPILOGUE-X/V002/submission.asc:2179-2289`、`本地实验/STORE-EPILOGUE-X/V002/local-result.json`、`线上结果/STORE-EPILOGUE-X/V002/result.json`。
- `RECOMMENDATION_TO_MAIN`：保留为低优先级对照候选；只有 Main 认为小幅 issue 点移动值得测时再选。

## H3 — 两段写回的末段长度

- `HYPOTHESIS_ID=STORE-W2-H3-TAIL-CHUNK-POLICY`
- `STATUS=READY_FOR_MAIN_REVIEW`
- `MECHANISM`：仅对 V002 merge-on 行改用 K=2；令首段结束于 `tileCount-2`，t=8 时为 6+2。
- `BOTTLENECK`：V002 要等整行完成才发 Store；末段长度可能决定最后暴露的写回量。
- `DIRECT_PARENT`：V002，SHA `59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839`；V003 只供机制和性能对照。
- `TARGET_SHAPES`：`1x32768 FP32`（t=8）是区分 6+2 与 V003 4+4 的主探针；`8x16384 FP32`（t=4）仅复验 2+2，不区分末段策略。
- `TARGET_DTYPES`：FP32。
- `WHY_IT_MAY_HELP`：若 6-tile 首段 Store 可在末两 tile 算术期间完成，最终待完成的 Store 仅覆盖两 tile。
- `WHY_IT_MAY_FAIL`：6-tile 首段比 V003 的 4-tile Store 更长，发出后也只剩两个 tile 可重叠；首段未完成时，第二笔 Store 会排在其后。V003 的 Official 结果低于 V002，风险明确。
- `UB_IMPACT`：整行驻留 buffer 大小与布局不变。
- `DMA_IMPACT`：每行 Store 从一次变两次，总字节数与 copy primitive 不变；不跨行。
- `SYNC_IMPACT`：沿用 V002 的 2-deep event ring；两次 Store 各自遵循现有依赖，批尾 drain 保持不变。
- `PRECISION_RISK`：算术顺序及 dtype 不变；需保证 chunk 边界按 tile 对齐、范围不越行，且 Store 完成前不覆盖 UB 源区。
- `DUPLICATE_CHECK`：与 V003 同属 chunked writeback，但唯一新变量是末段边界；V003 的 split 为 `tileCount/2`。V003 不作本轮父项。
- `RELATED_OLD_ROUTES`：STORE-EPILOGUE-X V002/V003；commit `acc92000` 的旧 H1 为较宽泛的分块提早写回想法，本项只检验 t=8 的末段长度变化。
- `MINIMAL_OFAT_DIFF`：只在 V002 merge-on 分支增加两次 chunk Store，边界设为 `splitTile=tileCount-2`；不改启用条件、batch 计算、ring 深度或循环调度。
- `MINIMAL_EXPERIMENT`：仅先测 `1x32768 FP32`；完成 correctness、same-binary，再按协议做至少 4 组交错 P/C。保留原始配对样本，不加 kernel 内计时插桩。
- `EXPECTED_LOCAL_PROBES`：`1x32768 FP32` 为判别形状；`8x16384 FP32` 只作 V003 2+2 复验，不用于确认 6+2。
- `UNCERTAINTY`：高；chunk 的真实传输时长和最后 drain 暴露量未知。
- `EVIDENCE_PATHS`：`线上结果/STORE-EPILOGUE-X/V002/submission.asc:2174-2289`、`线上结果/STORE-EPILOGUE-X/V003/diff.patch`、`线上结果/STORE-EPILOGUE-X/V003/result.json`、commit `64cf431a` 的 `本地实验/STORE-EPILOGUE-X/V003/local-result.json`。
- `RECOMMENDATION_TO_MAIN`：保留为 V003 的窄化后续；不要从 V003 本地优势推断线上收益，先评估首段 Store 未完成时的尾部代价。

## H4 — 多行 t=3 写回启用条件

- `HYPOTHESIS_ID=STORE-W2-H4-MULTIROW-ELIGIBILITY`
- `STATUS=NEEDS_MORE_EVIDENCE`
- `MECHANISM`：仅改 V002 predicate：`tileCount >= (batchRows > 1 ? 3 : 4) && rowWidth % 8 == 0`。t=3 只对已有每核多行 batch 开启整行 Store。
- `BOTTLENECK`：多行 t=3 仍逐 tile Store，描述符数量高于 V002 已合并的 t>=4 行。
- `DIRECT_PARENT`：V002，SHA `59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839`。
- `TARGET_SHAPES`：候选 `128x12288 FP32`，仅在现有 `availableCoreNum`、runner 与 tiling 得出 `batchRows>1` 时适用；`1x12288 FP32` 为单行控制，`128x16384 FP32` 为 t=4 控制。
- `TARGET_DTYPES`：FP32。
- `WHY_IT_MAY_HELP`：可把可达的多行 t=3 从每行三次 Store 改为一次，减少描述符，同时保留单行 t=3 的 V002 行为。
- `WHY_IT_MAY_FAIL`：V001 曾显示短行延后写回代价可超过描述符节省；目标测试集也可能没有 `batchRows>1` 的 t=3 wide FP32 输入。
- `UB_IMPACT`：无；使用 V002 已分配的整行缓存。
- `DMA_IMPACT`：命中行从每行三次 Store 变一次，字节数不变，不跨行。
- `SYNC_IMPACT`：命中行沿用 V002 整行 Store 与现有 event ring；其他形状逐字节维持原分支。
- `PRECISION_RISK`：数值路径不变；新增命中行仍须满足 V002 对整行起点及完整 UB 范围的要求。
- `DUPLICATE_CHECK`：与 commit `acc92000` 旧 H2 的全局 tileCount 阈值不同；本项只对 `batchRows>1` 开 t=3，单行仍需 t>=4。与多行 DMA 不同，每行仍独立写回。
- `RELATED_OLD_ROUTES`：STORE-EPILOGUE-X V001/V002/V003；旧 gap H2；R015 仅作“不跨行”的边界参照。
- `MINIMAL_OFAT_DIFF`：只替换 merge predicate；不动 tile width、batch limit、block count、row ownership 或 Store body。
- `MINIMAL_EXPERIMENT`：先按现有 host blockCount 与 tiling 静态确认 `128x12288` 的 `batchRows>1`；若可达，只改 predicate 并以该形状做 correctness、same-binary、至少 4 组交错 P/C。
- `EXPECTED_LOCAL_PROBES`：主测 `128x12288 FP32`；控制 `1x12288 FP32`、`128x16384 FP32`。若主形状不受现有 runner 支持或 `batchRows<=1`，记为无可用探针，不改 row scheduling。
- `UNCERTAINTY`：中高；谓词差异明确，形状可达性与净性能收益未实测。
- `EVIDENCE_PATHS`：`线上结果/STORE-EPILOGUE-X/V002/submission.asc:2088-2105`、`:2174-2289`、`:3577-3585`、`本地实验/STORE-EPILOGUE-X/V002/local-result.json`、`调度/当前任务.tsv:39`。
- `RECOMMENDATION_TO_MAIN`：先确认现有 workload 中有可达的多行 t=3 形状；确认后再与 H1/H3 比较，不触碰 row scheduling。

## 筛除与 Main 建议

- H2：`STATUS=INFEASIBLE`。V002 的最终 `WaitFlag<MTE3_V>` 后立即释放对应 event ID（`submission.asc:2280-2289`），后面没有可合法并行的工作。单纯移动 drain 只能把完成等待移到资源释放之后，不能构成有意义的 OFAT；不得把它保留为候选。
- H5：`STATUS=DUPLICATE_REJECTED`。旧提交 `acc92000` 的 STORE H4 已提出删除 `ProcessWideFp32FullCacheRows` tile 循环入口两个 `WaitFlag<MTE3_V>` 块，让 gamma/bias MTE2 Load 与前序 Store 重叠；证据为该提交 `研究/STORE-EPILOGUE-X/next-hypotheses.md` 的 H4 段及 V002 `线上结果/STORE-EPILOGUE-X/V002/submission.asc:2186-2200`。本轮 H5 也是只改同一依赖链、Store/Copy 数不变；与旧 H4 的机制、位置和最小 diff 完全相同，`DIFFERENCE=NONE`。Wave-2 SYNC H1 也删除同一对入口等待并保留逐槽等待，见 `6a19f78c:研究/SYNC-TOPOLOGY-CHAMPION-X/TRACK-B-HANDOFF.md`；不得另立 H5。
- Track-B 曾保留 H1、H3、H4 三个有效候选。Planning 随后选择 H1，Main-1 review 确认从 V002 移动现有整行 Store issue point 是单变量范围；V001 声明不把 H3/V003 donor 引入 Candidate。H3 与 H4 仍只是未选研究项。

## 路线证据对照

| 已提交路线/证据 | 已知机制与结果 | 与本路线的关系 |
|---|---|---|
| R001–R029 / FULL：`技术路线/技术路线总表.md:27-33,65-71`；`归档/历史控制文件/idea-pool-29-routes.md` | 相关项为 R002 y 驻留、R005 大 tile、R009 DataCopyPad 尾块、R010 手工尾块、R013 双缓冲、R015 多行 DMA；FULL-R009/R010 各 PASS 15/15，FULL-R015 为 1/15 Runtime Error。 | R009/R010 改尾部搬运形式，不改 Store 发出/依赖；R013 是阶段缓冲；R015 跨行 stride DMA，明确排除。其他 R001–R029 记录属于算术、归约、dtype、tiling、行分配或工程轴，无相同 Store issue/chunk 变量。 |
| R31A V021：`技术路线/全版本记录.tsv:56`；`本地实验/R31A/V021/diff.patch`、`local-result.json` | FP32 CachedRows 将 MTE3-to-V wait 延至行末；四组有效配对方向混合，`NEEDS_ONE_MORE_LOCAL`。 | 等待点后移，与 Store 时序相邻；函数/父版不同，也不移动本轮 H1 的 Store 调用或改变 H3 chunk 边界。 |
| R31B V017：`技术路线/全版本记录.tsv:97`；`线上结果/R31B/V017/diff.patch`、`result.json` | FP16/BF16 pass-2 延后 MTE3_V drain；Official 44.68，低于 V011 的 45.16。 | Store drain 邻接证据，dtype 与缓存路径不同；不等同于 FP32 Store issue 点。收益不可从其 Local 直接外推。 |
| MIX-A V003/V007：`技术路线/全版本记录.tsv:14,18`；`线上结果/MIX-A/V003/diff.patch` | V003 为多行复用加入 V_MTE2 release wait 并限制 `localRows==1`；V007 移除 narrow-mid 的 `SyncVToMTE2`，其目标 shape 未通过 same-binary 资格，未做 P/C。 | MTE2/V 同步与 dispatch，不改 wide FP32 Store issue/chunk；属于邻近同步证据，不重复。 |
| Wave-1 STORE V003：`技术路线/全版本记录.tsv:101`；`线上结果/STORE-EPILOGUE-X/V003/diff.patch`、`result.json`；`64cf431a:本地实验/STORE-EPILOGUE-X/V003/local-result.json` | K=2、按 `tileCount/2` 拆分；本地 `8x16384` -5.64%、`1x32768` -3.15%（相对 V002）；Official 44.38，低于 V002 45.07。 | H3 保留为窄化变量：V002 父版上的 t=8 split 从 4+4 改成 6+2；同一机制方向，须独立单变量评估，V003 仅供体证据。 |

## 当前 Wave-2 重复关系

只对照远端已提交 handoff，不读取其他 Agent 工作树或未提交资料。

| 路线及已提交证据 | 对照结论 |
|---|---|
| SYNC：`6a19f78c:研究/SYNC-TOPOLOGY-CHAMPION-X/TRACK-B-HANDOFF.md` | SYNC H1 删除本函数 tile-loop 入口两处 MTE3_V wait，保留逐槽 wait、event 分配与最终 drain；与筛除的 H5 是相同代码点。SYNC H2 为低精度参数 MTE2_V wait 排序，H3 为现有 Muls 与参数 Load 重叠，均非有效 STORE 候选的同一变量。 |
| EPI：`5031a5fa:研究/EPI-ARITH-CHAMPION-W2-X/TRACK-B-HANDOFF.md` | 行间 Mul/Add 分组、MulAdd 舍入、invRms 折入 gamma 均属算术变量；H4/H5 目标含 `1x32768 FP32`，与 STORE 探针形状相交，但机制不同。EPI 的 Store source 改动不在本路线范围内。 |
| FASTPATH：`9e5a5731:研究/SELECTIVE-FASTPATH-CHAMPION-X/TRACK-B-HANDOFF.md` | FASTPATH H1 使用 R31B V017 的低精度 drain donor，属邻近等待路径；H3 直接使用 STORE V003 的 4+4 donor。STORE H3 只在 V002 上测试 6+2 边界，差异仅为 split 点；形状可能重合，不叠加 donor。FASTPATH H4 是 EPI V002 算术 donor，形状重合但机制不同。 |
| SMALLMID：`47dacfa2:研究/SMALLMID-DATAFLOW-CHAMPION-X/track-b-handoff.md` | SMD-H5 将小 D FP32 输出双槽的 MTE3 wait 移到 slot 复用点；与 STORE 同属等待时序，但函数、buffer 生命周期和 D≤2048 shape 不同，不重复，也不移植。 |

## Main 交接

### Main 选择确认

- `PLANNING_SELECTED_HYPOTHESIS=STORE-W2-H1-ISSUE-POINT`；来源为 C2C CONTROL，Main-1 回执与审阅记录在 `研究/主代理/MAIN-1-W2/campaign-status.md`，提交 `2a27be0bbaa81b7f67777f7d8e99277cbe23946a`。
- `MAIN_SELECTED=YES`；Main review 确认只移动 V002 的现有整行 Store 发出点，Store 次数、地址、chunk geometry、条件、event 顺序、算术和 tiling 保持不变。
- `REVISION=V001`；`DIRECT_PARENT=STORE-EPILOGUE-X V002`；`PARENT_SOURCE_SHA=59fb8eada4da0b2b83cccffa4fb89fb97b7503ba3dcb08d5e0fe348e3caeb839`；`OFFICIAL_ANCHOR=45.07`。
- Revision declaration：`本地实验/STORE-EPILOGUE-W2-X/V001/REVISION-DECLARATION.md`。
- H4 的 `128x12288 FP32` 可达性问题不影响当前 H1；H1 主探针为 `1x32768 FP32`。correctness、same-binary 与 P/C 测量须使用项目现行协议，并等待 Main 分配 server3 job。
