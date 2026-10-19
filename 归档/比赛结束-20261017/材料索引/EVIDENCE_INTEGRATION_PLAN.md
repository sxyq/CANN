# Evidence Integration Plan

## Final closeout status

`INTEGRATION_BRANCH_RECONCILED=YES`；当前 Integration HEAD=`6ec8c174c7352677ff623e760d6ddd55dc48e84e`，分支为 `integration/w4-closeout-20261008`。`PRIMARY_MAIN_UPDATED=NO`；`PRIMARY_MAIN_UPDATE=DEFERRED_DIRTY_17`；`MAIN_MERGE_STATUS=DEFERRED`。Primary main HEAD=`e7f669692a2f5815c4ce444cfb52dde1d24a93a2`，仍有 9 tracked dirty + 8 untracked，未修改；当前没有主线写入者进入。`origin/main=1ea9677ba8e7307a73f13041c7b639ab6a96425c`。

Evidence copy completed W4 R01-R15 where available。Candidate 与生产 kernel 未合入。总核验为 `1566 manifest rows = 1097 included + 469 excluded`；`1097/1097 source/archive blobs 一致`；cross-batch source key duplicates=`0`；archive selected source extensions `.asc/.c/.cc/.cpp/.cxx/.h/.hpp`=`0`。

五批归档统计如下；Batch-02 与 Batch-04 各由两个连续提交完成。`Files` 为该批次归档路径下的 tracked 文件数，包含 manifest；纳入/排除数按 manifest 行统计。

| Batch | Commit(s) | Files | Manifest rows | Included | Excluded |
|---|---|---:|---:|---:|---:|
| Batch-01 Research | `c235eb00` | 140 | 164 | 139 | 25 |
| Batch-02 R01/R02 | `4df78bc6`, `97cdf851` | 360 | 484 | 359 | 125 |
| Batch-03 R07 | `c9ff35a7` | 3 | 2 | 2 | 0 |
| Batch-04 R08/R09 | `cf95a3d7`, `4fb3441c` | 88 | 219 | 87 | 132 |
| Batch-05 R10/R11/R14 | `6ec8c174` | 511 | 697 | 510 | 187 |
| Total | 7 commits | 1102 | 1566 | 1097 | 469 |

所有 33 个注册 worktree 均保留，`RETIREMENT_STATUS=HELD`。原因包括 Route lifecycle 未被 Planning 关闭、Candidate branch 仍需保留、W2/W3 独有 evidence 未全部整合、R02/R08/R09/R14 旧 runtime/command UNKNOWN、R01/W3-crossrow/M2 ignored evidence、external detached dirty checkout，以及 primary dirty。

本轮只做报告更新、精确 add 两个报告文件并提交；没有运行 Compile、Correctness、Local、NPU、Online；没有删除、clean、reset、rebase、force-push；`PUSH=NO`。

盘点基准：integration worktree 初始快照 `edcbaab4c506e05ed369830245e60aeb711ca714`；本次状态更新日期 2026-10-09。最新 Main 图回执为 `e7f669692a2f5815c4ce444cfb52dde1d24a93a2`，相对 `origin/main=1ea9677ba8e7307a73f13041c7b639ab6a96425c` ahead 16 / behind 2；最新只读 Main 状态回执确认 primary main 有 9 tracked dirty + 8 untracked，未暂存，精确路径列于 inventory，内容未读、未复制、未覆盖。integration 已合入 `566e...`、`1ea9677...`、`e7f669...`；最终报告提交前 HEAD=`6ec8c174c7352677ff623e760d6ddd55dc48e84e`。

integration branch 已合入 local `main` 至 `e7f669692a2f5815c4ce444cfb52dde1d24a93a2`，并合入已核实的 `origin/main=1ea9677ba8e7307a73f13041c7b639ab6a96425c`；三处 canonical 记录冲突已在 integration 内逐项保留双方差异，当前 `MERGE_HEAD` 不存在。后续只在出现新授权来源时再评估 merge。不合入任何 Candidate，不运行 Compile、Correctness、Local、NPU 或 Online。A/B/C 是证据处置分类，不构成新的 Route 生命周期决定。主工作树 17 项 dirty 保持不动。

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
| W4-R08 | 已保存的 V001 measurement、复测与 task/event attribution 证据 | `c38218735f51`, `f4b3c22b497a`, `1bc84959fbc5` |
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
| W4-R02/R05/R08/R09/R12 | 最新 task-table agent IDs 的 runtime 查询均为 `not_found`；这不是 close receipt。R02/R08/R09 current stage 或 running operation UNKNOWN；local process scan 无匹配设备命令；server3 命令归属 UNKNOWN | `CLOSE_STATUS=UNKNOWN`、`RETIREMENT_STATUS=HELD`；不关闭 Route |
| W4-R14 | HEAD `64bb1b04c02aee792179e6e75c9f4ffb655d49e1`、clean；旧 task ID `not_found`；current stage 和 `RUNNING_DEVICE_OPERATION` UNKNOWN | `ACTIVE_AGENT=UNKNOWN`、`RUNNING_COMMAND=UNKNOWN`、`RETIREMENT_STATUS=HELD`；Parent fingerprint evidence 待整合，不关闭 Route |
| W4-R04 / W2 Selective | Main 已确认 Agent 关闭；R04 有 93 个新增 evidence 文件；Selective 有 correctness helper | 仅 Agent close 可记 YES；worktree retirement 仍 NO，等待 evidence 归档说明 |
| W2 Tiny | Agent 已关闭；detached remote task 仍 UNKNOWN；Parent codegen wrapper 已提交 | 保留 provenance UNKNOWN；不得据 Agent close 标成 worktree 可退役 |
| Primary main | 最新只读 Main 状态回执确认 9 tracked + 8 untracked、合计 17、未暂存；精确路径见 inventory。最新可用图回执 HEAD=`e7f669...`、ahead 16 / behind 2；路径回执未提供新 HEAD。primary 未进入 | `MAIN_DIRTY_STATE_PRESERVED=YES`；不读取、复制、覆盖、暂存或合并 dirty 内容 |
| External detached `/Users/sunyiyang/.codex/worktrees/f0e7/cann` | HEAD `8f9338f8d01230635e617ff037e5951bb77bf67f`；9 tracked + 8 untracked；owner/agent/command UNKNOWN | 保留注册和 dirty 项；`CLOSE_STATUS=UNKNOWN`、`RETIREMENT_STATUS=HELD` |

## Agent shutdown 与 Worktree retirement

`SAFE_TO_CLOSE` 是是否可以结束 Agent/运行上下文；`RETIREMENT_STATUS` 是建议是否可将物理 worktree 从后续工作中退役。前者为 YES 不会自动令后者为 YES。所有 branch、worktree 和证据在本轮均保留。

| 范围 | SAFE_TO_CLOSE | RETIREMENT_STATUS | 依据 |
|---|---|---|---|
| W4-R01、R03、R06、R07、R10、R11、R13、R15 | NO（本轮没有 Main close receipt；server3 有来源不明的瞬态进程） | NO | 不以 task-table 空槽或本机扫描单独认定远端命令结束；W4 生命周期仍需 Planning 处置 |
| W4-R02、R05、R08、R09、R12 | NO | NO | 旧 agent IDs 为 `not_found`，没有 close receipt；远端命令归属 UNKNOWN |
| W4-R04 | YES（Agent only） | NO | Main 已确认 Agent close；93 个 evidence 文件仍待整合 |
| W4-R14 | NO | NO | old agent ID `not_found` 不是 close receipt；current stage / `RUNNING_DEVICE_OPERATION` UNKNOWN；Parent fingerprint evidence 待整合 |
| W2 Tiny | YES（Agent only） | NO | Main 已关闭 Agent；detached remote task 未核实；Parent evidence 来源人 UNKNOWN |
| W2 Selective | YES（Agent only） | NO | Main 已关闭 Agent；`correctness-run-001` evidence 和 creator identity UNKNOWN |
| W2 CASE14、CASE47、SYNC | NO | NO | 当前状态/Agent receipt `SNAPSHOT_PENDING`；unique commits 与 source history 留在分支 |
| W3 七个 worktree | NO | NO | Git 状态 clean，但当前 Agent shutdown receipt UNKNOWN；unique commits 与历史 Candidate/evidence 未集成 |
| Support 两个 worktree | NO | NO | Git clean、零 unique commits；Support owner/Agent shutdown receipt UNKNOWN |
| M2 VECTOR-MATH-X | NO | NO | 独立 active workstream 状态不在本轮确认范围，保留分支 |

以下逐项 `RETIREMENT_STATUS` 建议均为 NO：W4-R01 至 W4-R15；W2 CASE14、CASE47、SYNC、Tiny、Selective；W3 R1-R5 和 W3 integration/record-owner worktrees；Support arch-hardware 与 community-intelligence；M2 VECTOR-MATH-X；外部 detached checkout。Agent 关闭状态与物理 worktree 退役分开记载。本轮保留所有工作树、branch 与证据，不作删除或归档。

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

### Merge completion receipt (historical; superseded)

`SUPERSEDED_BY_FINAL_CLOSEOUT=YES`

```text
LOCAL_MAIN_MERGE=540484b8eeaee702de219b9092d4f3a168788f40
LOCAL_MAIN_PARENT=566e329e31f0b971e0c66bebeb60f73e467ee7b7
ORIGIN_MAIN_MERGE=075a5931a55dbe549bd2292c30b450f74861e111
ORIGIN_MAIN_PARENT=1ea9677ba8e7307a73f13041c7b639ab6a96425c
INTEGRATION_HEAD_AFTER_MERGE=075a5931a55dbe549bd2292c30b450f74861e111
MERGE_STATUS=completed; superseded by later local-main merge 6433b48cea810881530db6126fbefe5bddfe4914
MAIN_DIRTY_STATE_PRESERVED=YES
PUSH=NO
```

Latest local-main update: `e7f669692a2f5815c4ce444cfb52dde1d24a93a2` (parent `566e329e31f0b971e0c66bebeb60f73e467ee7b7`), title `records: 补入 R04/R14 历史来源证据`, four shared-record paths only. Integrated as `6433b48cea810881530db6126fbefe5bddfe4914`, parents `075a5931a55dbe549bd2292c30b450f74861e111` and `e7f669...`; no conflict, `MERGE_HEAD=ABSENT`. Current local main / origin direction is 16 / 2. The new Main receipt reports 9 tracked dirty + 8 untracked, none staged; primary main remains untouched.

三处 canonical 冲突均已在 integration branch 处理，没有整文件选边覆盖：保留 local-main 与 origin-main 的不同记录行。版本账本保留 W4 行和 origin 的 W3 行；`W4-R11/V002` 的原始性能记录与 Parent-only 补充是两个不同事件，均保留。路线成绩表保留 15 条 W4 行和 5 条 W3 行；历史 `H001` 两个不同阶段行也各自保留。当前任务表保留 W4 当前调度行及 origin 的 W3 R1-R5 状态行。文件内原有的历史字段数差异与空行来自共同祖先记录，未在本次合并中改写。

origin merge 同时带入其正式提交内容；这些文件并非本轮手工路径拷贝：

| Route | 来源 commit | 随 origin 合并的文件 |
|---|---|---|
| W3 R2 / Adaptive Core Ownership V015 | `1ea9677ba8e7307a73f13041c7b639ab6a96425c` | `线上结果/ADAPTIVE-CORE-OWNERSHIP-CHAMPION-X/V015/official-result.json`, `submit.stderr.log`, `submit.stdout.log` |
| W3 R5 / Crossrow Full Pipeline V012 | `1ea9677ba8e7307a73f13041c7b639ab6a96425c` | `线上结果/CROSSROW-FULL-PIPELINE-CHAMPION-X/V012/official-result.json`, `submit.stderr.log`, `submit.stdout.log` |
| W3 R4 / Multirow Panel RMS V020 | `1ea9677ba8e7307a73f13041c7b639ab6a96425c` | `线上结果/MULTIROW-PANEL-RMS-CHAMPION-X/V020/official-result.json`, `submit.stderr.log`, `submit.stdout.log` |

对 merge 变更路径的核对未发现 Candidate 源码或性能路径。主工作树 17 项 dirty 没有进入 integration。Evidence-only 路径筛选授权有效；在本次两份报告更新提交后立即开始按 Route 分批整合 W4 已提交证据。

### Incoming historical rule commit and scope (historical; superseded)

`SUPERSEDED_BY_FINAL_CLOSEOUT=YES`

```text
INCOMING_INTEGRATION=e7c553ec0da4f96af461baa0dfdb02d4730f8fc7, followed by wording-only 566e329e31f0b971e0c66bebeb60f73e467ee7b7
POLICY_COMMIT=e7c553ec0da4f96af461baa0dfdb02d4730f8fc7; parent=82a45cb2db775d6b6787450c1e0c914aa7dbdefb; seven policy files
POLICY_FOLLOWUP=566e329e31f0b971e0c66bebeb60f73e467ee7b7; one wording-only Online Owner Skill change
W4_OFFICIAL_SCOPE=W4_BEST_OF_ROUTE_VALIDATED_CANDIDATES_ONLY
MAX_SUBMISSIONS_PER_ROUTE=1
MEASUREMENT_RECORD=Build/Compile PASS; Correctness PASS; valid Local with numeric score/delta, raw samples and medians, shape/dtype, device, free HBM, load note, current best; route-internal comparable best; submitted source exactly matches validated Candidate commit; route quota/Judge quota/current Judge rules confirmed
INTEGRATION_BRANCH_RECONCILED=YES
PRIMARY_MAIN_UPDATED=NO
PRIMARY_MAIN_UPDATE=DEFERRED_DIRTY_17
MAIN_MERGE_STATUS=DEFERRED
BLOCK_REASON=DIRTY_17_AND_PENDING_REVIEW; three canonical paths were reconciled in integration. Primary main remains dirty 9 tracked + 8 untracked and was not entered.
INTEGRATION_OWNER_ONLINE_PERMISSION=NO
SERVER3_TRANSIENT_PROCESS_UNATTRIBUTED=clx_ref_parent_; remote RUNNING_COMMAND globally UNKNOWN
NEXT_ACTION=Planning review and primary-main merge only after dirty-state resolution; no worktree retirement yet
```

两个 policy commit 已随 local main merge 进入 integration；primary main 的 17 项 dirty 未带入。六项资格全部满足前，Route 标记为 `SUBMISSION_STATE=NONE_RECORDED`，本 Integration Owner 不执行 Online。

### Evidence-selection status (historical; superseded)

`SUPERSEDED_BY_FINAL_CLOSEOUT=YES`

`PATH_LEVEL_EVIDENCE_COPY=SUPERSEDED_BY_FINAL_CLOSEOUT`。五批 evidence integration 已完成：`1566 manifest rows / 1097 included / 469 excluded / 1097 blob exact / 0 duplicate / 0 selected source extensions`。Candidate 与生产 kernel 未合入；各批次来源、路径和排除项继续以本报告及 inventory 为准。`PUSH=NO`。

`INTEGRATION_BRANCH_RECONCILED=YES; PRIMARY_MAIN_UPDATED=NO; PRIMARY_MAIN_UPDATE=DEFERRED_DIRTY_17; MAIN_MERGE_STATUS=DEFERRED`
`NEXT_ACTION=Planning review and primary-main merge only after dirty-state resolution; no worktree retirement yet`

### Batch 01 Evidence Receipt

`BATCH_01_SCOPE=W4-R03,W4-R06,W4-R12,W4-R13,W4-R15`。归档根目录为 `证据集成/W4/Batch-01-Research/`，来源路径保持 Route 原相对路径；`SOURCE_PATH_MANIFEST.tsv` 逐文件记录 Route、事件/Revision、结果分类、来源 commit、Git blob、源路径和归档路径。已纳入 139 个 evidence 文件；另有 25 个分析脚本、探针源码或构建/执行支持源码明确列为 `EXCLUDE_SOURCE_SUPPORT`，未进入归档。纳入内容包含研究报告、Parent-only result、日志、raw/profile、反汇编和诊断输出；`CANDIDATE_PRODUCTION_PATHS_INCLUDED=0`，不改变生产 kernel 或 Candidate。

`BATCH_01_SOURCE_COMMITS=R03:712e47230287182bc65ab433a3ce714e6fdafd9f,5e95f9bc9d9c44fab03aecf9c84c29c0b947614b; R06:bd825c5fd59c74c283478026e8b32702ba331eb5,b40c09e62adcd85f51258ed1cef3723b80b010c8; R12:a018f702f3e6beb91569299fe903f59826961840,e697ddf35d4c42e735236e597338a148bfb437f2; R13:91e891a2a266ecb2b870afead3c3aa0fd94b67ac,a10cf2a1e5c8f25aa256ce9dbfb79cd3347d12b4,f417e4361ddac6b626ced1e62cfb75de8e3695ee; R15:4213e6f19502875b8efbc70cda1c1cf16d8e0ca5,d13e51e90ab81451ab86e46034db3ddfde4b05d2,744f3ad313f7586c8368a0765021ba3cac8c138e`。

### W3 Batch-09 evidence receipt

`BATCH_09_SCOPE=W3-R1-R3-R4-R5`。来源仅从 Integration worktree 的 Git 对象读取：R1 `64e32f535348cdde1828cd8a8fd89fad100aaa33`（12 commits），R3 `584c590cef81349969c4dcf014a5c04d05032e9d`（12），R4 `ce6c6dc568256ac8b80b674096bd7ca081b897cb`（34），R5 `1efa0863611f1a9b76ab13dd361eba161b32b46d`（29）；共同 base 为 `503b98cb22ae88798bb6bfca83a46676933f3600`。R1/R3 的 Batch-07 来源路径未重复收录；本批追加其余 25 个日志文件。

Batch-09 的 manifest 有 1,570 行：纳入 910 项，排除 660 项。纳入包括 Local 表格与上下文、Correctness/Compile/运行日志、R5 的 15 份修订声明、R4 V001/V002 的四个 UTC 时间记录，以及 R4 V020、R5 V012 的 Official JSON 和 stdout/stderr。六个 Official 附件来源为 `1ea9677ba8e7307a73f13041c7b639ab6a96425c`，提交说明为 `docs: record W3 official calibration`；仅归档已有文件，不改 Official 内容或共享记录。`commits.tsv` 记录四条来源分支的 87 个提交及该 Official 来源提交。

排除项逐条保留来源路径与 commit/tree/blob 标识，类别为 Candidate/Parent/kernel/support 源码、构建配置和执行脚本；未复制 kernel 源码。所有纳入项按其来源 `commit:path` 解析到 manifest 所列 blob，并保留原字节。来源路径与 Batch-07/08 manifest 不重复。R4/R5 来源分支未发现 `研究/` 路径或独立研究报告。R5 ignored profile 文件未读取，相关数量与状态保持 `UNKNOWN`；各 Route 的 Agent/运行命令状态也保持 `UNKNOWN`。没有进入或改动其他工作树、共享账本、分支或 Route 生命周期；`PUSH=NO`。