# W4 结果清算接手摘要

## 状态

- 主工作树：`/Users/sunyiyang/Desktop/Project/cann`，分支 `main`；共享同步提交 `e7208c7ff2d9ec3dde8e419a57e4fd8cc2d426a2`。
- 清算目录含五份交付文件；本次补充只更新 Local 表、Route矩阵、完整报告和接手摘要，Official表保持不变。
- 原主树17项未提交内容保留；本轮审计补充提交只含四条报告路径。PUSH=NO；未运行设备、Candidate、Compile、Correctness、Local 或 Online 操作。

## 可复用统计

- Official JSON：116条；Pass 65、Runtime Error 19、Wrong Answer 18、Time Limit Exceeded 6、Compile Error 8；65条含官方分数；submission ID 唯一。
- `source-meta.json`：24份；23份与结果JSON同目录，另有 `线上结果/R31A/V024/source-meta.json` 没有同目录结果JSON。22项可比较的 source 文件摘要值一致。
- W4当前账本：32条，10条 `REAL_EXECUTED_REVISION`、22条 `ROUTE_RESEARCH_EVENT`；15条授权路线均出现在矩阵。Local表38条数据（39行含表头），Official表116条（65条有分数）。
- Local行分类：10条真实性能记录、24条研究事件、3条同版补充、1条同版结果receipt；来源身份键无重复。`AUDIT_FLAGS`为多标签：OBSERVATION_ONLY=12、MEASUREMENT_BLOCKED=13、NOT_MEASURED=25、CORRECTNESS_FAILED=1、BUILD_FAILED=0、VALID_ACCEPTED=0、VALID_REJECTED=0；标签允许重叠，详见完整报告。
- 每Route置信枚举为HIGH=0、MEDIUM=0、LOW=0、INVALID=9、NOT_MEASURED=6；结论枚举计数为MEASUREMENT_UNRESOLVED=9、RESEARCH_ONLY=4、CORRECTNESS_PROGRESS=1、DUPLICATE_OR_EXHAUSTION_RISK=1，其余PROVEN_LOCAL_GAIN、VALID_NO_GAIN、INSUFFICIENT_EVIDENCE均为0。
- 原五项待同步来源现已全部登记：R05研究、R05 V002性能版、R11研究、R11 V002同版补充、R12研究；本批已知pending=0，`STATE_SYNC_GAP=NONE_FOR_THIS_BATCH`。共享提交为 `e7208c7ff2d9ec3dde8e419a57e4fd8cc2d426a2`。
- R01 V001同版补证来源为 `4b9dcade629bd7e0fdbe3194c740bd1c7824d107`；task P/C中位数9.50/8.91us、side ratio −6.2105%仅观察；调整P/P差−0.04us、95% CI [−0.24,0.06]跨0，P/C配对95% CI [−0.16,0.036]跨0，漂移10.63%/16.05%；模板reference通过，严格超差目标每侧1个、对照每侧3个，max abs error=0.00390625。Local score/delta/best=`NONE`；原三项dirty已随该提交纳入，worktree clean，device operation NONE。Event ID=`UNKNOWN`。
- R04新增 receipt：768样本、reference通过、输出逐位一致、计时波动大、`MEASUREMENT_BLOCKED`；submission ID与source commit均 `UNKNOWN`。已有0.992171392044 / +0.789037863747%继续标观察值；本次有效Local仍为0。
- Online=`PAUSED`；PUSH=`NO`。

## 矩阵快照与读取限制

- Route分支及完整HEAD来自主树 `git worktree list --porcelain` 元数据，快照日期为2026-10-08；dirty摘要采用Main提供的只读状态快照。没有进入Route工作树或读取其中未提交文件。
- R04记录8个状态项，含一个未跟踪目录，目录内部文件数 `UNKNOWN`；R14记录1个未跟踪目录项，内容未读。其余13条为该快照报告的 `CLEAN`。
- 矩阵的最新Compile/Correctness/性能版/研究事件来自当前账本与已提交结果或明确receipt；receipt未给出的source identity仍为 `UNKNOWN`。所有W4 Official score均为 `NONE`。

## 后续核验事项

1. 五项同步已完成；R11研究 Event ID 保持 `UNKNOWN`，R01补证 Event ID=`UNKNOWN`，R04 receipt source commit/submission ID=`UNKNOWN`。本批待同步数为0。
2. R04计时分析、原始采样/profiler数据、研究稿及派发日志仍留在 Route 工作树；本次未读取，精确文件数 `UNKNOWN`。768样本不计为有效 Local。
3. R01原三项dirty已随 `4b9dcade629bd7e0fdbe3194c740bd1c7824d107` 提交，worktree clean；无设备操作。
4. Official表保留每份结果JSON的case数组；报告区分官方分数、绝对耗时及 `CALCULATED_COMPARISON`，不根据case ID推断shape或dtype。
5. 本批之外的R03 receipt对应与R07 timing attribution研究/同版补充同步状态仍待确认；来源commit与账本当前行的具体对应见完整报告“缺口与边界”。
6. Local measurement reliability覆盖七轴：event/kernel-task边界、task波动、PC/CP次序、输出地址/slot、wrapper分配释放、同二进制/P/P资格、跨Route可比性；缺少数据处保留 `UNKNOWN`。

## 文件

- `OFFICIAL_RESULTS_AUDIT.tsv`
- `LOCAL_RESULTS_AUDIT.tsv`
- `W4_ROUTE_RESULT_MATRIX.tsv`
- `W4_FULL_RESULT_AUDIT.md`
- `W4_RESULT_REVIEW_HANDOFF.md`
