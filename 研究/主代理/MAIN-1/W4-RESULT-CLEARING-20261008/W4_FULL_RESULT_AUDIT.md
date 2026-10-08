# W4 全量结果清算

清算日期：2026-10-08。数据范围为仓库 `线上结果/` 中可读取的官方 `result.json`、相邻 source metadata、当前主树 W4 共享账本与调度表、可由主树 Git 对象读取的证据，以及本对话中 Main/Route 明确转交的 receipt。未访问任何 Route 工作树的未提交文件。四份共享记录已由独立提交 `e7208c7ff2d9ec3dde8e419a57e4fd8cc2d426a2` 同步；本报告补充另行提交。

## 官方提交结果

Node 脚本逐个读取官方 JSON，产出116条记录；每行保留原始 `testcases` JSON 数组。115份含15个 case 的数组；`线上结果/ASYNC-OVERLAP-CHAMPION-X/V001/result.json` 的原始数组为空，未补造 case。116个 `submissionId` 均唯一，65条有 `officialScore`。结果状态分布如下：

| Official 状态 | 数量 | `officialScore` |
|---|---:|---|
| Pass | 65 | 有值 |
| Runtime Error | 19 | UNKNOWN |
| Wrong Answer | 18 | UNKNOWN |
| Time Limit Exceeded | 6 | UNKNOWN |
| Compile Error | 8 | UNKNOWN |

成绩前列按 `officialScore` 排列；同分采用并列名次。完整116行排名与每条15 case 字段见 `OFFICIAL_RESULTS_AUDIT.tsv`。

| 名次 | 结果 | Official Score | 正确性 |
|---:|---|---:|---|
| 1 | R31B/V011 | 45.16 | 15/15 |
| 2 | R31B/V012 | 45.14 | 15/15 |
| 3 | STORE-EPILOGUE-X/V002 | 45.07 | 15/15 |
| 3 | R31B/V013 | 45.07 | 15/15 |
| 5 | R31A/V016 | 45.00 | 15/15 |
| 6 | EPILOGUE-ARITH-CHAMPION-X/V002 | 44.96 | 15/15 |
| 7 | R31A/V015 | 44.75 | 15/15 |
| 8 | R31A/V013 | 44.70 | 15/15 |
| 9 | MIX-A/V003 | 44.69 | 15/15 |
| 10 | R31B/V017 | 44.68 | 15/15 |

R31B/V011 的官方结果为45.16、15/15；source SHA 为 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`，metadata 中 source commit 为 `43a1049a1e08e518c88e354a754fdebb85a96f99`。未从文件时间推断提交日期。

`officialScore`、JSON 内 `calculatedScore` 与绝对 `timeUs` 分列保留。原公式使用各 case 的 `timeUs` 和 `bestTimeUs` 计算分数；分数与总绝对耗时不是同一指标。Official 表的分数排序不代表 Local 结果。

### R31B V011、V012、V013 对照

三份 JSON 的15个 `bestTimeUs` 全部相同。以下总耗时由各 JSON 的 `timeUs` 求和；逐 case 最大变化均按 `CALCULATED_COMPARISON` 从同一 index 的时间相减。case index 与 ID 直接取原始 JSON，不推断 shape 或 dtype。

| 对照 | Official Score | 正确性 | 绝对 `timeUs` 总和 | `bestTimeUs` | 最大改善 | 最大退化 |
|---|---:|---|---:|---|---|---|
| V011 | 45.16 | 15/15 | 27283.08 us | 15项固定 | — | — |
| V011→V012 | 45.16→45.14 | 15/15 | 27283.08→27305.73 us（+22.65 us） | 全部相同 | index 14 / `6a9a9a99bf41025d6013ebbe`：−6.15 us（−0.0373%） | index 15 / `6a9a9a99bf41025d6013ebc2`：+20.91 us（+0.2170%） |
| V011→V013 | 45.16→45.07 | 15/15 | 27283.08→27269.62 us（−13.46 us） | 全部相同 | index 14 / `6a9a9a99bf41025d6013ebbe`：−29.28 us（−0.1776%） | index 15 / `6a9a9a99bf41025d6013ebc2`：+7.38 us（+0.0766%） |

因此 V012 的 Official Score 下降同时伴随总绝对耗时上升；V013 的 Official Score 也下降，但总绝对耗时减少。不能用总分变化替代耗时变化。

### 官方来源字段

仓库共有24份 `source-meta.json`：23份与 `result.json` 同目录；另有 `线上结果/R31A/V024/source-meta.json`，其目录没有 `result.json`。22个目录同时具备 `submission.sha256` 与 JSON `source.sha256`，逐项比对均一致。116条中115条含 JSON source SHA、13条有 metadata source commit、0条有明确 source date；缺少的值在 Official 表保留 `UNKNOWN`。官方结果 JSON 路径、submission ID 与 case 数组均来自仓库文件。

## W4 Local 与路线概览

当前主树全版本账本包含32条 W4 身份：10条 `REAL_EXECUTED_REVISION`、22条 `ROUTE_RESEARCH_EVENT`。10个已登记性能版均未产生有效 Local；表内候选时间和 delta 保留原字段及观察标签，不据此认定 Local Best。R09 的当前 Local Best 保留 `UNKNOWN`。R05 V002使本轮真实性能版数增加1；R01/V001与R04/V002同版补证不增加性能版。Online 保持 `PAUSED`，`PUSH=NO`。

Local 清算表含38条数据：32条 W4 账本身份行，以及6条补充或独立 receipt 审计行，包含 R01、R03、R04、R07 与 R11 的同版补充来源。`LOCAL_RESULTS_AUDIT.tsv` 将观察值、接受值、研究项、同版补充和身份缺口分列；全文件39行（含表头）。

| Route | 当前可确认结论 | 测量置信标记 | 主要未确认项 |
|---|---|---|---|
| W4-R01 | V001同版 task 对照 `MEASUREMENT_BLOCKED`；P/P调整差−0.04 us，95% CI [−0.24,0.06]跨0；P/C配对95% CI [−0.16,0.036]跨0，漂移10.63%/16.05%超限 | 观察值未接受；Local score/delta/best均为 `NONE` | side ratio −6.2105%仅观察；模板 reference PASS与严格差异分开：目标每侧1个、对照每侧3个，最大绝对误差0.00390625；来源 `4b9dcade629bd7e0fdbe3194c740bd1c7824d107`，Event ID `UNKNOWN`；R01原三项dirty文件已随该提交纳入，worktree clean，device operation NONE |
| W4-R02 | V002 `MEASUREMENT_BLOCKED` | 观察值未接受 | task 与 event 范围、调用次序及槽位漂移仍需区分 |
| W4-R03 | occupancy/InitBuffer 研究；最新 receipt 只支持源码层面候选移除 | 无 Local | 编译消除及设备工作量未证实；新 receipt 与账本同步状态 UNKNOWN |
| W4-R04 | V002 `MEASUREMENT_BLOCKED`；新 receipt 报768样本、双方 reference 通过、输出逐位一致；event、kernel task与host call逐条对应 | 观察值未接受；有效Local仍为0 | task波动大；receipt的source commit与submission ID为 `UNKNOWN`；已有0.992171392044 / +0.789037863747%仍为观察值；本轮receipt不新增性能版 |
| W4-R05 | V002性能版与地址研究均已登记；性能测量 `MEASUREMENT_BLOCKED`，账本共3条记录 | 观察值未接受；Local Best=`NONE` | 4.54 us / −5.8091%与另一输入4.41 us / −8.8843%均为观察；完整库一致性及设备指令冲突未证实；resident标签不证明residentParams执行 |
| W4-R06 | UB/GM结构模型完成；无 Candidate 或 NPU验证 | 无 Local | 60项UB与10项GM模型只验证等式和反例；合法共同GM源视图未证实 |
| W4-R07 | V002 `MEASUREMENT_BLOCKED`；task 14.84/14.84 us 与0%为观察 | 观察值未接受 | predecessor transition、调用位置及event差异影响未分离 |
| W4-R08 | V001 `MEASUREMENT_BLOCKED`；parent-only时序研究 | 观察值未接受 | event内核外中位50.162 us；底层 launch wrapper 来源未识别 |
| W4-R09 | V001 `MEASUREMENT_BLOCKED`；FP16函数13792→13728 bytes，FP32/BF16函数体字节相同 | 无新Local | 无设备助记符/基本块映射，不据字节差估算指令或收益 |
| W4-R10 | V001旧失败保留；V002 `MEASUREMENT_BLOCKED` | 观察值未接受 | task 顺序与长尾归因未完成；不升级候选观察值 |
| W4-R11 | 三条账本记录：V001研究、V002性能版及V002研究；同版补充为Parent-only，未执行Candidate | 观察值未接受；Local Best=`NONE` | task MAD比例target/control=0.33391304/0.13587786；研究Event ID=`UNKNOWN`；保留原性能来源commit `4243f4e9e90a56cc12cf4adcbd7282149238b8f3` |
| W4-R12 | 两条研究记录；入口代码研究 `CODEGEN_EVIDENCE_UNAVAILABLE`，无Candidate或性能版 | 无 Local | host反汇编可读，设备助记符不可用；未证明可减少设备工作；本批无待同步事件 |
| W4-R13 | 参数复用诊断研究已记录 | 无 Local | 跨 batch 顺序与 D40000 容量未验证；未形成性能版 |
| W4-R14 | `MEASUREMENT_BLOCKED`；参数专属时间 UNKNOWN | 观察值未接受 | 旧代理跨设备限制、输入与参数重叠歧义仍在 |
| W4-R15 | `ROUTE_REVIEW_REQUIRED` | 无 Local | 等待 Planning 复核；代理关闭不表示 Route 关闭 |

R05 的 `80x2056` resident 标记需要按 R03 receipt 限定：40 cores 对应每核两行并走 `SmallFp32Batched`；既有 reference PASS 仍有效，但不能由该标签推断 `NarrowMid residentParams` 实际执行。

## 五项已同步事件

以下按 source commit 与路径逐项登记。未知事件 ID 保持 `UNKNOWN`，不从相邻记录生成 ID。

| Route / Revision | 来源身份 | 分类 | 本次清算结论 |
|---|---|---|---|
| R05 / NONE | `1a31a3b5…`；`本地实验/W4-R05/gamma-view-20261008/RESULT.md` | `ROUTE_RESEARCH_EVENT` | 地址探针符合源模型；未测 bank decode；与V002性能行分开 |
| R05 / V002 | `1a31a3b5…`；同上 | 新 `REAL_EXECUTED_REVISION` | `MEASUREMENT_BLOCKED`；1个新增性能版，Local接受数为0 |
| R11 / V002 | `1b528176…`；`研究/W4-R11/LOCAL-PROTOCOL-DIAGNOSTIC-20261008.md` | `ROUTE_RESEARCH_EVENT`，ID=`UNKNOWN` | Parent-only同协议计时研究；Candidate未执行 |
| R11 / V002 | `1b528176…`；`本地实验/W4-R11/V002/RESULT.md` | `VERSION_RECORD_EVENT` 同版补充 | 更新既有V002证据；不新增性能版；保留原性能来源 commit `4243f4e9…` |
| R12 / NONE | `e697ddf3…`；`研究/W4-R12/TINY-ENTRY-CODEGEN-STUDY.md` | `ROUTE_RESEARCH_EVENT` | 设备生成代码不可读；无Candidate、Correctness或Local执行 |

上表五项中，来源支持1个新增性能版、3个新增研究身份、1项既有版本补充。五项均已写入四份共享记录，提交为 `e7208c7ff2d9ec3dde8e419a57e4fd8cc2d426a2`；本批已知待同步数为0，`STATE_SYNC_GAP=NONE_FOR_THIS_BATCH`。R11研究 Event ID 保留 `UNKNOWN`；未知身份字段不作为待同步事件。

## R04 新增 receipt

R04 Agent receipt 报告768样本；Parent 与 Candidate 各自 reference 通过且输出逐位一致；event、kernel task 与 host call 逐条对应，但 task 波动较大，结论为 `MEASUREMENT_BLOCKED`。该 receipt 未给出 source commit、submission ID 或可接受的数值 Local；身份字段保留 `UNKNOWN`。该补证已并入共享提交 `e7208c7ff2d9ec3dde8e419a57e4fd8cc2d426a2`，本次性能版新增数为0，receipt 不提升 Local Best。共享调度表中既有 `0.992171392044` 与 `+0.789037863747%` 仍标为观察值。

## 缺口与边界

- R01 原三项未提交文件已包含在来源提交 `4b9dcade629bd7e0fdbe3194c740bd1c7824d107`；receipt 报告提交后 worktree clean，device operation NONE。R04 receipt相关计时分析、原始采集/profiler数据、研究稿与派发日志仍留在 Route 工作树；本轮没有读取这些文件，精确文件数保持 `UNKNOWN`。
- 主树原有17项未提交状态保持不动：9项已跟踪文件修改、8项未跟踪对象。四份共享表的既有差异也未纳入本报告提交。
- 共享账本当前可读到32条W4身份（10条性能、22条研究）；R05、R11、R12五项来源均已登记，本批待同步数为0。R01补证 Event ID=`UNKNOWN`，R11研究 Event ID=`UNKNOWN`，R04 receipt source commit与submission ID=`UNKNOWN`；这些未知值继续保留，不影响已完成的本批同步状态。
- 本批以外仍有两组来源对应未确认：R03 receipt `W4-R03-INIT-CONSUMERS-20261008`（`5e95f9bc9d9c44fab03aecf9c84c29c0b947614b:研究/W4-R03/HOST-OCCUPANCY-STUDY.md`）与现存 R03 账本行 `R03-HOST-OCCUPANCY-20261008`（commit `712e47230287182bc65ab433a3ce714e6fdafd9f`）的关系未知；R07 timing attribution receipt（commit `bca492a32476d579f44b60516f8aeaac6e36b6d5`，`研究/W4-R07/TIMING-ATTRIBUTION-20261008.md`）的研究及同版补充是否已完整并入共享行尚未确认，当前 R07 V002性能行来源仍为 `addff7d657da4f3fee46e3b882fb80556d916da8`。这两组来源不属于本次五项待同步范围。
- `source-meta` 总量为24；其中1份无同目录结果JSON。未由文件时间补写 Official 日期；没有来源的 source commit/date 均保留 UNKNOWN。
- 报告只归纳现有仓库结果和明确 receipt。未从缺失原始采样、Route 未提交目录、shape 名称或 Official case ID推造数据。

## 生成文件

- `OFFICIAL_RESULTS_AUDIT.tsv`：116条官方结果，包含完整 case JSON。
- `LOCAL_RESULTS_AUDIT.tsv`：38条数据行，39行含表头。
- `W4_ROUTE_RESULT_MATRIX.tsv`：15条授权路线，每 Route 一行。
- `W4_FULL_RESULT_AUDIT.md`：本报告。
- `W4_RESULT_REVIEW_HANDOFF.md`：新会话接手摘要。
