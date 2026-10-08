# Evidence Integration Plan

盘点基准：integration worktree 初始快照 `edcbaab4c506e05ed369830245e60aeb711ca714`；本次状态更新日期 2026-10-09。Main 当前回执为 `566e329e31f0b971e0c66bebeb60f73e467ee7b7`，相对缓存 `origin/main` ahead 15 / behind 2；`git ls-remote` 回执确认远端 `1ea9677ba8e7307a73f13041c7b639ab6a96425c`。primary main dirty 17，保持原样；未进入该工作树。

本轮报告更新后，只在 integration branch 依次合入 local `main` 与上述已核实 `origin/main`。不合入任何 Candidate，不运行 Compile、Correctness、Local、NPU 或 Online。A/B/C 是证据处置分类，不构成新的 Route 生命周期决定。主工作树 17 项 dirty 保持不动。

## A 类：可纳入审阅的已提交证据

A 类指已提交、可由 Git 对象定位的研究记录、结果说明、原始测量或 profiler 输出。这里只提出按路径摘取和后续比对的范围；commit 作为来源定位符，不代表可以整笔 cherry-pick。若某 commit 同时包含 Candidate 或支持代码，后续只考虑经确认的证据路径。

| Route / 来源 | 可纳入审阅的证据 | 来源提交 |
|---|---|---|
| W4 audit snapshot | 五份清算交付：`LOCAL_RESULTS_AUDIT.tsv`、`OFFICIAL_RESULTS_AUDIT.tsv`、`W4_FULL_RESULT_AUDIT.md`、`W4_RESULT_REVIEW_HANDOFF.md`、`W4_ROUTE_RESULT_MATRIX.tsv`；五份在当前 82a45 snapshot 已存在 | 当前 main snapshot；对应主线提交含 `e7208c7ff2d9ec3dde8e419a57e4fd8cc2d426a2` |
| W4-R01 | V001 设备复测、task-time attribution、balanced task comparison 的提交结果与报告 | `93f15d9bab643fa6e8dbf6ed8833a144672d9fb3`, `7f8a335f451d4`, `4b9dcade629b` |
| W4-R02 | V002 paired-parent timing attribution 结果 | `e100b5d9b17e` |
| W4-R03 | host dispatch/core-count 与 InitBuffer consumer 研究 | `712e47230287`, `5e95f9bc9d9c` |
| W4-R04 | 93 个 V002 timing-attribution/profiler 原始证据文件，含 768 samples、per-call task map 与 raw profiler；latest commit 未改 Candidate | `766bb53acb85db70a4c2648feae72511ac20a10e` |
| W4-R05 | parameter/output consumer 研究说明 | `61e0aa28d0bd` |
| W4-R06 | cross-input DMA address、shared UB view 与 GM span 研究 | `bd825c5fd59c`, `b40c09e62adc` |
| W4-R07 | V002 task/event timing attribution 说明 | `bca492a32476` |
| W4-R08 | 已保存的 V001 qualification、复测与 task/event attribution 证据 | `c38218735f51`, `f4b3c22b497a`, `1bc84959fbc5` |
| W4-R09 | V001 device-code compare 与 dispatch-location 说明 | `ea18c3346057`, `ec24f3ac550d` |
| W4-R10 | V001 事件恢复、multibatch probe domain 与 V002 timing attribution 研究 | `b1459725d358`, `5b7b92150136`, `205ef6702b37` |
| W4-R11 | V002 timing evidence 与 same-protocol Parent pair 记录 | `4243f4e9e90a`, `1b528176920d` |
| W4-R12 | Parent tiny task timing 与 entry codegen 研究 | `a018f702f3e6`, `e697ddf35d4c` |
| W4-R13 | Parent tail failure、parameter reuse ordering 研究 | `91e891a2a266`, `a10cf2a1e5c8`, `f417e4361dda` |
| W4-R14 | Parent MTE2 findings、task/event mapping 与七文件 Parent profiling harness | `fcbd18147643`, `5a49704ff4d6`, `64bb1b04c02a` |
| W4-R15 | duplicate audit、byte-boundary derivation 与 address/lifetime 研究 | `4213e6f19502`, `d13e51e90ab8`, `744f3ad313f7` |

R04 raw exports 有 `git diff --check` whitespace/trailing-empty-line 提示，来源文件维持原字节状态；本轮不改写、不复制这些原始导出。R14 的新增 Parent harness 路径见 inventory 的 R14 receipt refresh。它包含 `kernel.asc`，其用途来自 Parent fingerprint 研究支持，不列为 Candidate。

## B 类：只留在历史 Candidate 分支

B 类包括 W2/W3 已提交 Candidate 历史，以及 W4 commit range 中实际改动 Candidate 源文件的部分。保留在原 branch/worktree，以 revision provenance 和历史结果回查。不得 cherry-pick 整个 Candidate commit，也不把这些源文件拷入 main。

| 范围 | 保留方式 |
|---|---|
| W2 CASE47、SELECTIVE、SYNC、TINY | Candidate 版本与 runner 维持原 branch。SELECTIVE 的 `1a43be725337`、`fe4f61166c5f` 是 Candidate 代码历史；W2 Tiny V001 及其他已提交 Candidate 源沿原 branch 保存。 |
| W3 R1-R5 | W3 各路线的历史 Candidate 与提交证据按各自 branch 保留；source path 数见 inventory，不将旧 Local 或 Official 事实改写成 W4 状态。 |
| W4-R01/R02/R04/R05/R07/R08/R09/R10 | Candidate 修改留在 Route branch。例：R01 `1e6b06bb20a2`、R02 `60ca277d2afc`、R04 `8ffc6b696292`、R05 `1a31a3b527d8`、R07 `addff7d657da`、R08 `6d35a57c0c1e`、R09 `96044629c978`、R10 `2ca51316eb4b` 与 `1441fc7297b3`。 |
| W4 其他路线 | 以 inventory 的 `BASE_COMMIT..HEAD` 源路径盘点为准。Parent probe、runner 和 Candidate 分开标注；不能只因文件扩展名或目录名推断 Candidate 身份。 |

本轮没有 Candidate merge 或 cherry-pick。

## C 类：来源人或状态未闭合的证据

C 类保留原有位置，待来源人/Owner 通过正式回执确认来源、运行状态和归档归属。当前两份 W2 新增文件已提交且各自 worktree clean，因此不再是 Git untracked；它们仍留在 C 的 provenance hold，直至证据入口确定。

| Worktree / evidence | 当前事实 | 下一责任 |
|---|---|---|
| W2 Tiny `本地实验/TINY-FIXED-OVERHEAD-CHAMPION-X/V002-parent-loop-proof/support/parent_codegen_wrapper.asc` | commit `f1868765509966ca829e6b3456135e237fd669fd`；用途为 Parent codegen evidence；creator identity UNKNOWN；remote detached process state UNVERIFIED | 原 Owner 确认 process 完成状态与文件来源；证据纳入后再评估物理 worktree 退役 |
| W2 Selective `本地实验/SELECTIVE-FASTPATH-CHAMPION-X/V001/support/run_correctness.sh` | commit `36756e8a750bf41628286213ca951e183899d5b1`；source report 引用 `correctness-run-001`；creator identity UNKNOWN | 原 Owner 确认文件来源并给出归档/索引位置；证据纳入后再评估物理 worktree 退役 |
| W2 CASE14、CASE47、SYNC | 最新状态栏为 `SNAPSHOT_PENDING`；未读取其他工作树内容 | 取得状态名称与 owner receipt 后更新 inventory；未知内容不进入本轮 integration |
| W4-R02/R05/R08/R09/R12 | 最新 task-table agent IDs 的 runtime 查询均为 `not_found`；这不是 close receipt。local process scan 无匹配设备命令；server3 命令归属 UNKNOWN | 取得 Main close receipt 和远端命令归属说明前，`SAFE_TO_CLOSE=NO`、`SAFE_TO_RETIRE=NO` |
| W4-R14 | HEAD `64bb1b04c02aee792179e6e75c9f4ffb655d49e1`、clean；Main 已关闭 Agent；旧 task ID `not_found` | Agent 可关闭；Parent fingerprint evidence 整合决定前不退役 worktree |
| W4-R04 / W2 Selective | Main 已确认 Agent 关闭；R04 有 93 个新增 evidence 文件；Selective 有 correctness helper | 仅 Agent close 可记 YES；worktree retirement 仍 NO，等待 evidence disposition |
| W2 Tiny | Agent 已关闭；detached remote task 仍 UNKNOWN；Parent codegen wrapper 已提交 | 保留 provenance UNKNOWN；不得据 Agent close 标成 worktree 可退役 |
| Primary main | Main receipt 在 `82a45...` 有 9 tracked + 8 untracked 路径；后续 `566e...` 回执为 dirty 17。primary 未进入；当前逐路径状态不重新读取 | `MAIN_DIRTY_STATE_PRESERVED=YES`；Integration Owner 不改、不暂存、不合并 dirty 内容 |
| External detached `/Users/sunyiyang/.codex/worktrees/f0e7/cann` | HEAD `8f9338f8d01230635e617ff037e5951bb77bf67f`；9 tracked + 8 untracked；owner/agent/command UNKNOWN | 保留注册和 dirty 项；`SAFE_TO_CLOSE=NO`、`SAFE_TO_RETIRE=NO` |

## Agent shutdown 与 Worktree retirement

`SAFE_TO_CLOSE` 是是否可以结束 Agent/运行上下文；`SAFE_TO_RETIRE` 是建议是否可将物理 worktree 从后续工作中退役。前者为 YES 不会自动令后者为 YES。所有 branch、worktree 和证据在本轮均保留。

| 范围 | SAFE_TO_CLOSE | SAFE_TO_RETIRE | 依据 |
|---|---|---|---|
| W4-R01、R03、R06、R07、R10、R11、R13、R15 | NO（无本轮 Main close receipt；server3 有来源不明的瞬态进程） | NO | 不以 task-table 空槽或本机扫描单独认定远端命令结束；W4 生命周期仍需 Planning 处置 |
| W4-R02、R05、R08、R09、R12 | NO | NO | 旧 agent IDs 为 `not_found`，没有 close receipt；远端命令归属 UNKNOWN |
| W4-R04 | YES（Agent only） | NO | Main 已确认 Agent close；93 个 evidence 文件仍待整合 |
| W4-R14 | YES（Agent only） | NO | Main close receipt；HEAD clean、无设备操作回执；Parent fingerprint evidence 待整合 |
| W2 Tiny | YES（Agent only） | NO | Main 已关闭 Agent；detached remote task 未核实；Parent evidence 来源人 UNKNOWN |
| W2 Selective | YES（Agent only） | NO | Main 已关闭 Agent；`correctness-run-001` evidence 和 creator identity UNKNOWN |
| W2 CASE14、CASE47、SYNC | NO | NO | 当前状态/Agent receipt `SNAPSHOT_PENDING`；unique commits 与 source history 留在分支 |
| W3 七个 worktree | NO | NO | Git 状态 clean，但当前 Agent shutdown receipt UNKNOWN；unique commits 与历史 Candidate/evidence 未集成 |
| Support 两个 worktree | NO | NO | Git clean、零 unique commits；Support owner/Agent shutdown receipt UNKNOWN |
| M2 VECTOR-MATH-X | NO | NO | 独立 active workstream 状态不在本轮确认范围，保留分支 |

以下逐项 `SAFE_TO_RETIRE` 建议均为 NO：W4-R01 至 W4-R15；W2 CASE14、CASE47、SYNC、Tiny、Selective；W3 R1-R5 和 W3 integration/record-owner worktrees；Support arch-hardware 与 community-intelligence；M2 VECTOR-MATH-X；外部 detached checkout。Agent 关闭状态与物理 worktree 退役分开记载。本轮保留所有工作树、branch 与证据，不作删除或归档。

## main / origin/main 方向差异

本地 Git 对象显示 `merge-base=de70b634813dea80783fc57716d6e95c158edeec`。Main 回执在 `82a45...` 时为 ahead 13 / behind 2；当前 local `main=566e329e31f0b971e0c66bebeb60f73e467ee7b7`、cached `origin/main=1ea9677ba8e7307a73f13041c7b639ab6a96425c` 为 ahead 15 / behind 2。Main 后续只读回执称 primary dirty 仍为 17。以下只列 commit/ref 元数据，不读取 primary main 工作树。

`main` 独有（15）：

```text
566e329e31f0b971e0c66bebeb60f73e467ee7b7 docs: 调整 Local 起始测量描述
e7c553ec0da4f96af461baa0dfdb02d4730f8fc7 docs: 明确 W4 路线内择优提交授权
82a45cb2db775d6b6787450c1e0c914aa7dbdefb clarify handoff source-meta count
8d0086189983a7ff034e069f7b5d90ed421d3a74 docs: add W4 audit classification
d3bf9494fbc0b78a66f291bb04ed90b0a17beaf4 docs: add W4 audit addendum
e7208c7ff2d9ec3dde8e419a57e4fd8cc2d426a2 W4 result ledger sync
a2d2c3373351d459495893eaa4761cc666bf218e docs: archive W4 result clearing files
07662d7b96e9beaaa0f56d97c9cf346b86081eb3 docs: update latest W4 slots and pending records
4959725ea0bbf8f1e3c5f939e176db6db69bd9fe docs: record two W4-R04 events and slots
a7d3f04ca2a47669d3eb3111d1ba47a21d72a8a5 docs: sync eleven W4 events and latest slots
285e7b4c2a6810774ca46c053bb7cac0f0b52223 docs: sync same-revision retest and latest slots
066e053d48408a1cbb81d1372c9f30b3b8ab2f9d docs: sync existing results and initial events
9f91895506023d917637f707bb3f61cd9d9f8765 docs: update W4 fresh context and role boundaries
8f9338f8d01230635e617ff037e5951bb77bf67f docs: enforce W4 continuation rules
1dbd8c9a58ee9ee841182997a4a5b8e3496817f6 docs: add durable W4 exploration rules
```

`origin/main` 独有（2）：

```text
1ea9677ba8e7307a73f13041c7b639ab6a96425c docs: record W3 official calibration
373a5a39a6a1144b090ff928cfdda8daa7d48734 docs: reconcile W3 phase-2 route records
```

`git merge-tree` 报告的内容冲突路径：

```text
技术路线/全版本记录.tsv
技术路线/路线成绩表.tsv
调度/当前任务.tsv
```

### Required incoming rule commit and authorization

```text
INCOMING_REQUIRED_INTEGRATION=e7c553ec0da4f96af461baa0dfdb02d4730f8fc7, followed by wording-only 566e329e31f0b971e0c66bebeb60f73e467ee7b7
POLICY_COMMIT=e7c553ec0da4f96af461baa0dfdb02d4730f8fc7; parent=82a45cb2db775d6b6787450c1e0c914aa7dbdefb; seven policy files
POLICY_FOLLOWUP=566e329e31f0b971e0c66bebeb60f73e467ee7b7; one wording-only Online Owner Skill change
W4_OFFICIAL_AUTHORIZATION=W4_BEST_OF_ROUTE_VALIDATED_CANDIDATES_ONLY
MAX_SUBMISSIONS_PER_ROUTE=1
QUALIFICATIONS=Build/Compile PASS; Correctness PASS; valid Local with numeric score/delta, raw samples and medians, shape/dtype, device, free HBM, load note, current best; route-internal comparable best; submitted source exactly matches validated Candidate commit; route quota/Judge quota/current Judge rules confirmed
MAIN_MERGE_BLOCKED=YES until integration branch completes path-level canonical conflict resolution
BLOCK_REASON=main/origin/main has 15/2 directional commits and three canonical-record content conflicts; primary main remains dirty 17 and is not entered.
INTEGRATION_OWNER_ONLINE_PERMISSION=NO
SERVER3_TRANSIENT_PROCESS_UNATTRIBUTED=clx_ref_parent_; remote RUNNING_COMMAND globally UNKNOWN
NEXT_ACTION=merge local main then verified origin/main in integration only; resolve three canonical paths line by line; retain Candidate branches and select evidence paths only.
```

两个 policy commit 已通过 Git 对象核对；只在 integration branch 合入，primary main 的 17 项 dirty 不带入。六项资格全部满足前，Route 标记为 `NO_ELIGIBLE_SUBMISSION`，本 Integration Owner 不执行 Online。

### Merge and evidence-selection execution

1. 在本 integration branch 先合入 local `main=566e329e31f0b971e0c66bebeb60f73e467ee7b7`，再合入已核实的 `origin/main=1ea9677ba8e7307a73f13041c7b639ab6a96425c`。
2. 对 `技术路线/全版本记录.tsv`、`技术路线/路线成绩表.tsv`、`调度/当前任务.tsv` 逐行保留双方有来源的 W3/W4 记录；无法安全调和时停止，保留冲突，不使用整文件覆盖。
3. W4-R01 至 W4-R15 优先按已提交研究报告、Local/Correctness/raw/profiler 路径及正式记录引用来源做路径级选择。纯 evidence commit 在文件清单确认后可整体应用；混有 Candidate 的 commit 仅取 evidence 路径。每个纳入路径记录原始 commit；Candidate 源码、性能改动和未验证结果不纳入。
4. 路径级全量 evidence copy 当前按用户要求暂停；本轮不复制 Route evidence。待用户只读审阅报告与 main/origin merge 后，再按其下一批精确清单恢复。后续每批只提交已明确纯 evidence 路径，未决来源、运行状态、远端命令和 creator identity 保留 UNKNOWN，保持 `PUSH=NO`。
