# MAIN-1 八路线证据与五路线建议

日期：2026-09-26。范围为本 Main 的八条指定路线。本文记录证据与本轮五个执行 slot 的建议，不删除路线对象、不改其他 Main 的归属；未进入五个 slot 的路线仍保留源码、分支、证据和 owner。路线树的永久调整由规划层决定。

## 结论

建议当前五个 MAIN-1 slot 由以下路线组成：

1. R31B
2. R31A
3. MIX-A
4. MODE-X-R015C
5. DTYPE-SPECIAL-X

本轮没有任何 MAIN-1 Candidate 获得符合现行规程的 Parent/Candidate 性能结论。五条路线的 `LOCAL_PERFORMANCE_CONFIDENCE=LOW`，原因是精确形状的 same-binary 资格或配对样本缺失；旧的负载污染样本不作收益或退化结论。建议暂不让 WIDE-X-FRESH4 占用本轮五个 slot：其 Fresh Blind 来源状态无法确认，尚无可用配对样本，且当前后续假设与其他活跃路线重叠较多。此项只释放本轮执行 slot，不等于 PARK；保留原 Agent、worktree、branch 和全部证据，永久路线决定仍待规划层。

## 八张路线证据卡

### 1. R31B

- `BEST_REVISION / BEST_OFFICIAL_SCORE`：V011，15/15，45.16；线上结果与正式来源身份记录一致。
- `CURRENT_CANDIDATE / CURRENT_CORRECTNESS`：V016，直接父版本 V011。源码 SHA-256 `9f5c353e65a13a740fe97dc7e6415df032d27560831a3ad142c77592b8208eb5`；目标 FP16/BF16 宽度 8192、12288、16384、32768 正确性通过。FP32 D=16384 的 Parent 与 Candidate 均失败，记录为变更范围外的继承或 harness 问题。
- `VALID_LOCAL_SIGNAL / LOCAL_MEASUREMENT_QUALITY`：无。四组旧配对读数方向混杂，均为 `LOAD_CONTAMINATED`；当前统一 runner 已构建但未运行。质量低。
- `SOURCE_IDENTITY / SERVER3_REPRODUCIBILITY`：Candidate 与 source-meta、local-result、correctness 记录一致；server3 编译与链接通过，定向 NPU 正确性通过。
- `ARCHITECTURE_MECHANISM / ORTHOGONALITY / DUPLICATE_RISK`：仅把低精度宽行初始 tile 从 4096 改为 8192，FP32、数学、同步及 row 选择不变。对当前 V016 单一变量清晰；与其他路线未来的 wide/dtype 研究有邻接风险，需逐假设审阅。
- `OWNERSHIP_CONFLICT / FAILURE_HISTORY`：未发现 ownership 冲突。历史上有 FP32 控制形状失败及污染测时；V015 不属于 V016 的父版本。
- `RECENT_PROGRESS / NEXT_HYPOTHESIS_QUALITY`：Source、SHA 清单、Direct Parent diff、runner build/link 已完成。Track-B 有 3 项可筛选方向：BF16 full-y 缓存压缩、重复 BF16 参数扩宽去除、pass-1 row/tile 索引递增化；均尚需编译器产物与形状证据。
- `EXPECTED_INFORMATION_GAIN / EXPECTED_GLOBAL_UPSIDE / MEASUREMENT_COST`：信息增益中高；全局上行空间高，路线 Best 接近全局 Best；成本中等，需逐 dtype/宽度资格化。
- `RECOMMENDATION`：保留在五个 active slot，V016 等待合格测量，不开 V017。

### 2. R31A

- `BEST_REVISION / BEST_OFFICIAL_SCORE`：V016，15/15，45.00。V017 为 44.45，低于父版并已拒绝。
- `CURRENT_CANDIDATE / CURRENT_CORRECTNESS`：V021，父版 V016。Candidate SHA-256 `4f5bfc319b72d1f0bcfd453bc92e80ac64216719898757292daf1aa1b73b6063`；已有目标 direct-probe correctness PASS，改动为单一 fence placement。
- `VALID_LOCAL_SIGNAL / LOCAL_MEASUREMENT_QUALITY`：无。旧 D=32768、D=24576 配对差异分别有反向方向，且 `LOAD_CONTAMINATED`；新配对 runner 未在 NPU 执行。质量低。
- `SOURCE_IDENTITY / SERVER3_REPRODUCIBILITY`：Parent 与 Candidate 源码 SHA、配对模块、ELF、runner SHA 均有记录；paired targets 编译/链接通过，host bridge 测试 4/4 通过。ACL/NPU runner correctness 尚未执行。
- `ARCHITECTURE_MECHANISM / ORTHOGONALITY / DUPLICATE_RISK`：V021 只移动宽 FP32 cached-row 路径的输出完成等待。与 V019 tile 宽度、V020 输入预取不同；跨路线的 Rsqrt、FMA、对齐 DataCopy 后续想法需要和 DTYPE/MIX 去重。
- `OWNERSHIP_CONFLICT / FAILURE_HISTORY`：未发现 ownership 冲突。V017 Official 退化；V019、V020 的 D=32768 correctness 失败；所有旧 timing 不作有效判断。
- `RECENT_PROGRESS / NEXT_HYPOTHESIS_QUALITY`：fail-closed paired runner 构建与 host identity/dispatch tests 完成。Track-B 已列出 3 项：scalar Rsqrt、FP32 affine FusedMulAdd、对齐 tile 的 DataCopy；均需 target compile/correctness 和跨路线重复筛选。
- `EXPECTED_INFORMATION_GAIN / EXPECTED_GLOBAL_UPSIDE / MEASUREMENT_COST`：信息增益中等；全局上行空间高，Official 45.00；成本中等，需先跑相同 runner 的 NPU correctness、same-binary 与形状配对。
- `RECOMMENDATION`：保留在五个 active slot，V021 不变，等待完整本地结果，不开 V022。

### 3. MIX-A

- `BEST_REVISION / BEST_OFFICIAL_SCORE`：V003，15/15，44.69；V003 的 provenance 标为 `MULTI_CHANGE`。
- `CURRENT_CANDIDATE / CURRENT_CORRECTNESS`：V007，父版 V003。Candidate SHA-256 `a63ad29a997ae2fe8a1238c1a47a9d5ddfb14d16f725fca975ce5c55523f28eb`；rows=1、D=256 的 FP32/FP16/BF16 targeted correctness PASS。
- `VALID_LOCAL_SIGNAL / LOCAL_MEASUREMENT_QUALITY`：无。四组旧配对均为 `LOAD_CONTAMINATED` 且方向混杂；统一 runner 已编译/链接但未运行。质量低。
- `SOURCE_IDENTITY / SERVER3_REPRODUCIBILITY`：Candidate、sidecar 和 local evidence 一致；CANN 8.5.0.alpha002 / Ascend910B3 / dav-2201 build/link PASS；host lease tests PASS。
- `ARCHITECTURE_MECHANISM / ORTHOGONALITY / DUPLICATE_RISK`：V007 在原 `localRows == 1` 条件下移除一次 `SyncVToMTE2()`，不改 dispatch、算术和 copy 顺序。此 revision 变量明确；宽路径的 wait placement 与之接近，但不是同一依赖边。
- `OWNERSHIP_CONFLICT / FAILURE_HISTORY`：未发现 ownership 冲突。祖先 V003 是多变量组合；V007 旧 timing 不可用。
- `RECENT_PROGRESS / NEXT_HYPOTHESIS_QUALITY`：当前 runner 已验证身份和租用参数，但未执行。Track-B 已有 8 个候选；优先项包括合并首轮 MTE2-to-V wait、对齐 narrow-mid DataCopy、去掉单行退出前无消费者的 MTE3-to-V wait。后两项须核实目标 API/依赖。
- `EXPECTED_INFORMATION_GAIN / EXPECTED_GLOBAL_UPSIDE / MEASUREMENT_COST`：信息增益中等；全局上行空间中等，已知 Official 低于两条 exploit；成本低至中等，主探测形状为 1x256。
- `RECOMMENDATION`：保留在五个 active slot，V007 等待合格 paired-local 结果，不开 V008。

### 4. WIDE-X-FRESH4

- `BEST_REVISION / BEST_OFFICIAL_SCORE`：无 Official 分数。
- `CURRENT_CANDIDATE / CURRENT_CORRECTNESS`：V001，父版 BUILD-FIX-001。Candidate SHA-256 `f7628795e6699288669dbff8963181e376e10741ba4d46c768af51cf09bf6895`；9/9 NPU correctness PASS。
- `VALID_LOCAL_SIGNAL / LOCAL_MEASUREMENT_QUALITY`：无。旧 host shim 数值不是 NPU kernel timing；unified runner 已 build/link 但从未运行。质量低。
- `SOURCE_IDENTITY / SERVER3_REPRODUCIBILITY`：V001 和父版 SHA、runner 与正确性 executable 身份都有记录；CANN build/link 和 9 个 correctness case 通过。Parent 性能 runner 未执行。
- `ARCHITECTURE_MECHANISM / ORTHOGONALITY / DUPLICATE_RISK`：V001 是 wide tile-width 方案。其 `FRESH_BLIND` 尚无法确认，记录显示曾接触 archived analysis，路径与影响未知。Track-B 的 parameter reuse、Rsqrt、aligned-copy 分别邻接 R31/DTYPE/MIX；双缓冲方向的 UB 预算已显示不可行。
- `OWNERSHIP_CONFLICT / FAILURE_HISTORY`：未发现跨 Main ownership 冲突。此前六次 runner build 尝试失败并已保留，后续 build/link 通过；source exposure 来源未厘清。
- `RECENT_PROGRESS / NEXT_HYPOTHESIS_QUALITY`：正确性与 paired-runner build/link 是实质进展，但无形状资格或 timing。现有 4 项后续想法里，3 项与活跃路线相邻，1 项当前 UB 预算不支持；下一项独立且可实施的 hypothesis 尚不够清楚。
- `EXPECTED_INFORMATION_GAIN / EXPECTED_GLOBAL_UPSIDE / MEASUREMENT_COST`：若来源隔离成立，信息增益和宽形状上行空间中等；现有来源不确定性降低其价值。成本高，需先确认来源边界与 Parent 可测性，再做 exact-shape same-binary。
- `RECOMMENDATION`：本轮不占五个 active slot。只把 scheduler 状态改为 `EVIDENCE_HOLD_NOT_IN_MAIN1_FIVE`；保留 Agent/worktree/branch/源文件/日志，不作 PARK 或永久路线变更。

### 5. MODE-X-R015C

- `BEST_REVISION / BEST_OFFICIAL_SCORE`：无 Official 分数；有效 correctness 起点为 R015C-r3。
- `CURRENT_CANDIDATE / CURRENT_CORRECTNESS`：R015C-r4，父版 r3。Candidate SHA-256 `9367db4ebb4edf6b7bf6cde97f4846f987e6ab230c2aeee44d942d5e881a1c74`；server3 exact run 绑定 kernel、tiling 和 executable SHA，(2,256)、(5,4096)、(3,8192) 三形状均 bit-exact PASS。
- `VALID_LOCAL_SIGNAL / LOCAL_MEASUREMENT_QUALITY`：无。runner build/link 与 host identity/argument tests 通过，ACL runner、same-binary 和 timing 未运行。质量低。
- `SOURCE_IDENTITY / SERVER3_REPRODUCIBILITY`：server3 executable SHA `21934a8cf15508b365e252810c8f858b4fb105c5cc5000f460d61c511a37e188`；tiling SHA `0939ba8498426fcd77645d826fcc45a8eb65c78996ad0ce1da1a00d1ac01a250`。源码、执行文件和 correctness log 的绑定明确。
- `ARCHITECTURE_MECHANISM / ORTHOGONALITY / DUPLICATE_RISK`：r4 只改为一行一 block，保留 segment/copy 路径、顺序、barrier 与 64 KiB staging。Track-B 的 per-row-segment block mapping 与缩小 staging 和现有 row/core 方向靠近；aligned copy 与 stride descriptor 候选需分别区分。
- `OWNERSHIP_CONFLICT / FAILURE_HISTORY`：未发现 ownership 冲突。r2 为 correctness fail、`DIAGNOSTIC_ONLY`；r3 是 valid correctness 起点。r4 的 build attempt 1 缺 `<vector>`，attempt 2 通过。
- `RECENT_PROGRESS / NEXT_HYPOTHESIS_QUALITY`：新增 exact-source/executable NPU rerun，解决了此前身份缺失。Track-B 至少 7 项：每 row segment 分 block、aligned DataCopy、缩小 staging、segment copy descriptor 批处理、固定 segment 分支、窄化 serial barrier 等；各项已有独立 OFAT 与证伪测试。
- `EXPECTED_INFORMATION_GAIN / EXPECTED_GLOBAL_UPSIDE / MEASUREMENT_COST`：信息增益高，因该 Route 的中宽度 copy/segment 机制尚无 Official 结果；全局上行未知；成本中等，需先给 3 个目标形状逐一做 same-binary 资格。
- `RECOMMENDATION`：保留在五个 active slot。r4 Main 结论为 `NEEDS_ONE_MORE_LOCAL`；正确性已 PASS，但不能替代同 binary 与配对性能结果。

### 6. DTYPE-SPECIAL-X

- `BEST_REVISION / BEST_OFFICIAL_SCORE`：本 Route 尚无 Official 分数。Direct Parent R31B-V011 的 45.16 是父源参考分，不是 DTYPE 的 Route 成绩。
- `CURRENT_CANDIDATE / CURRENT_CORRECTNESS`：V001，FP32，SHA-256 `e2717055199f541d98d85ceef2e011e52e2749d932ca954dc887868de7db880f`；39/39 NPU correctness PASS，max abs error `2.6226044e-06`。
- `VALID_LOCAL_SIGNAL / LOCAL_MEASUREMENT_QUALITY`：无。unified runner build/link 与 10 项 host identity 测试通过，runner 尚未执行；24 个候选 shape 均无 same-binary 资格。质量低。
- `SOURCE_IDENTITY / SERVER3_REPRODUCIBILITY`：V001 源码与 source-meta / correctness log SHA 一致；Parent、Candidate、runner 的 ELF SHA 已记录，CANN 8.5.0.alpha002 build/link PASS。
- `ARCHITECTURE_MECHANISM / ORTHOGONALITY / DUPLICATE_RISK`：V001 仅针对 FP32 通用 arithmetic/conversion 路径中的多余操作，不以 wide-row specialization 为中心。它与 R31 的宽路径有部分相邻机制，但 DTYPE 的候选矩阵覆盖窄/中形状和 dtype 计算路径。
- `OWNERSHIP_CONFLICT / FAILURE_HISTORY`：未发现 ownership 冲突。早期缺 C++ include、设备运行时符号和链接路径失败均有保留记录，之后 build/link/correctness 通过。
- `RECENT_PROGRESS / NEXT_HYPOTHESIS_QUALITY`：Exact source correctness 39/39、runner 与 identity host tests 已完成。Track-B 有多项筛选项；优先考虑 FP32 helper copy elision、完整输出 tile 的 direct copy、irregular narrow-row batch；Pattern AR 和 block cap 作为额外备选。FP32 affine/Rsqrt 与 R31A 方向有重合，实施前须再去重。
- `EXPECTED_INFORMATION_GAIN / EXPECTED_GLOBAL_UPSIDE / MEASUREMENT_COST`：信息增益高，可判断 generic path 是否在 FP32 上产生无效转换/复制；全局上行未知但可能覆盖多个小中宽度；成本中高，24 个 shape 需先选代表形状并各自资格化。
- `RECOMMENDATION`：保留在五个 active slot，V001 等待 runner NPU identity qualification、same-binary 和配对测量，不开 V002。

### 7. ASYNC-TRIPLE-X（MAIN-1 historical copy）

- `BEST_REVISION / BEST_OFFICIAL_SCORE`：无。
- `CURRENT_CANDIDATE / CURRENT_CORRECTNESS`：V001 SHA-256 `61223a486cca4c54e099f760e9f48a4cd2fbedb2645967b936fe80d2785e1e1a`；Main-1 copy 的 Parent 与 Candidate runtime 均遇到 507035，未取得 NPU correctness PASS。
- `VALID_LOCAL_SIGNAL / LOCAL_MEASUREMENT_QUALITY`：无 timing。
- `SOURCE_IDENTITY / SERVER3_REPRODUCIBILITY`：build/link PASS；编译器管理和 native ACL 运行故障日志保留。
- `ARCHITECTURE_MECHANISM / ORTHOGONALITY / DUPLICATE_RISK`：MTE2/V/MTE3 overlap，与 Main-2 当前 canonical ASYNC 所有权冲突，且 Main-2 有同名 Route。
- `OWNERSHIP_CONFLICT / FAILURE_HISTORY`：已确认双 Main ownership collision。Main-1 copy 维持 `COLLISION_FROZEN` 状态，只留历史证据；不可触碰 Main-2 worktree、branch 或实验。
- `RECENT_PROGRESS / NEXT_HYPOTHESIS_QUALITY`：没有本 Main 可继续执行的新进展；失败定位为 runtime 507035，不等同架构性能失败。
- `EXPECTED_INFORMATION_GAIN / EXPECTED_GLOBAL_UPSIDE / MEASUREMENT_COST`：当前增量信息低；潜在收益未知；任何进一步执行的成本与冲突风险均不可接受。
- `RECOMMENDATION`：不纳入 Main-1 五个 slot；保持历史 copy 与 collision 证据。

### 8. EXT-ASCEND-X

- `BEST_REVISION / BEST_OFFICIAL_SCORE`：无。
- `CURRENT_CANDIDATE / CURRENT_CORRECTNESS`：V001 最终源码 SHA-256 `bd2b18cedf25ee4cb51a63690a1e2e4f48a9fa18766674f7401aa24b56072b8a`；FP32 correctness 27/27 FAIL。
- `VALID_LOCAL_SIGNAL / LOCAL_MEASUREMENT_QUALITY`：无 timing；正确性失败后未测。
- `SOURCE_IDENTITY / SERVER3_REPRODUCIBILITY`：build/link PASS；exact source 与修复尝试、日志均保留。
- `ARCHITECTURE_MECHANISM / ORTHOGONALITY / DUPLICATE_RISK`：外部启发的 blockFactor/rowFactor/ubFactor tiling；概念上与 MODE/WIDE 的行和 tile 工作相邻。
- `OWNERSHIP_CONFLICT / FAILURE_HISTORY`：无跨 Main ownership 冲突；已 PARK。四轮同步、alignment 和输出 lifetime correctness-only 尝试后仍 27/27 失败。
- `RECENT_PROGRESS / NEXT_HYPOTHESIS_QUALITY`：没有 correctness breakthrough、local signal 或新可执行 hypothesis；失败与 PARK handoff 均留存。
- `EXPECTED_INFORMATION_GAIN / EXPECTED_GLOBAL_UPSIDE / MEASUREMENT_COST`：新增信息空间低；收益没有可用证据；继续路线需先重新规划正确性架构，成本高。
- `RECOMMENDATION`：不纳入五个 slot；保留已 PARK 路线的源码、分支和所有失败证据。

## 正式成绩、本地结果与测量条件

- 经 `result.json` 复核的 Main-1 Official anchors：R31B-V011 45.16，R31A-V016 45.00，MIX-A-V003 44.69；三者均 15/15。
- Main-1 当前候选的有效本地性能结果：无。污染的 R31B/R31A/MIX 旧数值不纳入方向统计；WIDE 的旧 host shim 不属于 NPU kernel timing。
- `local-timing-protocol.md` 当前 harness 状态为 `PARTIAL_SHAPE_CONDITIONAL`，资格按 Route、shape、dtype、binary 分别计算。Main-1 当前候选均没有满足本轮要求的完整 same-binary + paired 记录。
- 历史现场快照（server3 UTC `2026-09-25T22:02:40Z`）：device 0–3 AICore 为 35%–36%，HBM 59969–60026 MB 且有 VLLM worker；device 4–6 AICore 为 0%，HBM 59186–59876 MB 且分别有 VLLM 进程。device 7 AICore 为 0%、HBM 3431 MB、无进程，但现行规程要求避开。
- 现场快照（server3 UTC `2026-09-26T06:53:18Z`）：npu-smi 确认 device 0–7 均为 Ascend 910B3。device 0–3 的 HBM 为 91%，AICore 为 33%–39%；device 4–6 的 HBM 为 90%–91%，AICore 为 0%；device 7 的 HBM 为 5%、AICore 为 0%，但现行计时规则不允许使用 device 7。device 4 的 `proc-mem` 显示 PID 2999855 `VLLMEngineCor` 使用 55664 MB；驱动不支持全局 `npu-smi info proc` 查询。租用表当时没有 `LEASED` 记录。没有可用的授权计时卡，本次未启动 same-binary 或 P/C。
- 最新现场复核（server3 UTC `2026-09-26T07:34:13Z`）：device 0–3 的 HBM 仍为 91%，AICore 为 36%–37%；device 4 为 HBM 90%、AICore 0%，device 5–6 为 HBM 91%、AICore 0%；device 7 为 HBM 5%、AICore 0%，仍按现行规则避开。device 4 仍显示 PID 2999855 `VLLMEngineCor` 使用 55664 MB；租用表没有活动 `LEASED` 记录。设备状态未改善，本次也未运行 same-binary 或 P/C。
- 最新现场复核（server3 UTC `2026-09-26T09:24:47Z`）：device 0–3 的 HBM 为 59969–60026 MB、AICore 为 35%，各有 `VLLMWorker_TP` 进程；device 4 的 HBM 为 59185 MB、AICore 为 0%，PID 2999855 `VLLMEngineCor` 使用 55664 MB；device 5–6 的 HBM 为 59875–59876 MB、AICore 均为 50%，各有 `VLLMWorker_TP` 进程。device 7 的 HBM 为 3431 MB、AICore 为 0%、无运行进程，但现行规程要求避开。按每个 lease ID 的最新记录汇总，没有活动租约；没有合格设备，本次未启动 same-binary 或 P/C。
- Main-1 `ONLINE_CANDIDATES`：无。

## 缺少的本地验证队列

每次 server3 状态查询只代表其记录时间。恢复测量前须按统一 timing protocol 重新查询设备并取得独占 lease；候选只在 Parent exact-shape same-binary 合格后进入 P/C。

- **P0，按串行顺序**：R31A V021 ← V016；R31B V016 ← V011；MIX-A V007 ← V003；MODE-X-R015C r4 ← r3；DTYPE-SPECIAL-X V001 ← R31B-V011。五个当前 Route Candidate 均为 `MEASUREMENT_BLOCKED`；这次实时快照没有合格设备，same-binary 与新 P/C 都未运行。
- R31A V021 的 Route-owned paired runner 已按当前 warmup/配对协议更新。源码 SHA-256 `c95dec4cfd552020462e509fa3fbbf2dbe3b10162ff719cf735df15b1f08b2e7`；加载 CANN 环境后 Parent module、Candidate module 和 runner 构建返回码为 0，runner ELF SHA-256 `d11fa9aa541e5c75dc38bd509cd515ca44347162741f6b93b7a0b371aecfde11`。Host bridge tests 4/4 PASS；首次未加载 CANN 环境的失败日志保留。改后 runner 尚未运行，NPU correctness、same-binary 和 P/C 仍缺；Route commit `5adabc4739ea3bf4dac7e9a03637a3b889def461` 已推送并与分支远端一致。
- **P1**：WIDE-X-FRESH4 V001。先确认 Fresh Blind 来源并准备可核验的 BUILD-FIX-001 Parent executable control，再做 exact-shape same-binary；该 Route 当前不占五个执行 slot。
- **P2**：无。已知正式 Online 结果均有 calibration 行；缺少或不可信的 Local 数值已注明，不补造。
- **P3**：R31B V001–V010、V012–V014；R31A V002–V015、V018；MIX-A V004–V006。逐版记录见 `main1-revision-ledger.tsv`。这些版本当前不影响五条 Route 的直接父子测量，也没有值得占用 server3 的新问题；除非新证据改变 lineage 或 calibration 判断，否则不重跑。

## Route Best 状态与逐版本 lineage

`OFFICIAL_BEST` 指经 Official 结果确认的最高路线成绩；若缺少正式 Promote 决定，表中会明确标注。`LOCAL_BEST` 只接受完整有效的 server3 配对测量和 Main `LOCAL_ACCEPTED`；本次八条路线都没有满足条件的 Local Best。逐版原始字段、SHA、父版本、证据位置和缺失项以 `main1-revision-ledger.tsv` 为准。

| Route | OFFICIAL_BEST | LOCAL_BEST | CURRENT_CANDIDATE / LOCAL_VERDICT | ONLINE_QUEUE_STATE |
|---|---|---|---|---|
| R31B | V011, 45.16, 15/15, `PROMOTED` | None | V016, `MEASUREMENT_BLOCKED` | None |
| R31A | V016, 45.00, 15/15, `PROMOTED` | None | V021, `MEASUREMENT_BLOCKED` | None |
| MIX-A | V003, 44.69, 15/15; promotion remains inconclusive because its recorded parent V002 failed | None | V007, `MEASUREMENT_BLOCKED` | None |
| WIDE-X-FRESH4 | None | None | V001, `MEASUREMENT_BLOCKED`; Parent executable and Fresh Blind provenance unresolved | None; evidence hold outside current five slots |
| MODE-X-R015C | None | None | r4, `MEASUREMENT_BLOCKED`; correctness PASS on 3 recorded shapes | None |
| DTYPE-SPECIAL-X | None | None | V001, `MEASUREMENT_BLOCKED`; correctness PASS 39/39 | None |
| ASYNC-TRIPLE-X (Main-1 historical copy) | None | None | V001, `COLLISION_FROZEN`; parent and Candidate smoke had unresolved 507035 | None; no Main-1 execution |
| EXT-ASCEND-X | None | None | V001, `CORRECTNESS_FAILED`; parked after 27/27 failures | None |

```text
R31B: seed -> V001 43.19 -> V002 43.78 [PROMOTED by score audit] -> V003 43.68 [REJECTED]
      V003 -> V004 runtime failure -> V005 runtime/correctness failure
      V003 -> V006 43.91 [PROMOTED] -> V007 42.07 [REJECTED]
            -> V008 runtime failure; V009 43.81 [REJECTED]; V010 43.91 tie [no promotion]
      V010 -> V011 45.16 [PROMOTED / OFFICIAL_BEST]
      V011 -> V012 45.14 [REJECTED]; V013 45.07 [REJECTED]
           -> V014 [LOCAL EVIDENCE INCOMPLETE]; V015 [MEASUREMENT_BLOCKED]; V016 [CURRENT / MEASUREMENT_BLOCKED]

R31A: V000 -> V001 [BUILD_FAILED] -> V002 43.87 [first valid Official result]
      V002 -> V003 43.16 [REJECTED]; V004 comment-only [HISTORICAL]
           -> V005 43.60 [REJECTED]; V006 43.77 [REJECTED]; V007 43.65 [REJECTED]
           -> V008 42.79 [REJECTED]; V009 42.97 [REJECTED]; V010 44.09 [PROMOTED]
      V010 -> V011 42.18 [REJECTED]; V012 44.05 [REJECTED] -> V013 44.70 [PROMOTED]
      V013 -> V014 44.25 [REJECTED]; V015 44.75 [PROMOTED] -> V016 45.00 [PROMOTED / OFFICIAL_BEST]
      V016 -> V017 44.45 [REJECTED; local proxy misleading]
           -> V018 [SOURCE ONLY]; V019/V020 [CORRECTNESS_FAILED]; V021 [CURRENT / MEASUREMENT_BLOCKED]

MIX-A: V001 [WRONG ANSWER] -> V002 [WRONG ANSWER] -> V003 44.69 [Official result; promotion inconclusive]
       V003 -> V004 43.73 [REJECTED]; V005 44.07 [REJECTED]
             -> V006 [LOCAL EVIDENCE INCOMPLETE]; V007 [CURRENT / MEASUREMENT_BLOCKED]

WIDE-X-FRESH4: BUILD-FIX-001 [valid build/correctness route base, no performance result]
               -> V001 [correctness PASS; Parent control unresolved; MEASUREMENT_BLOCKED]
MODE-X-R015C: r1 [missing revision placeholder]; r2 [CORRECTNESS_FAILED]
              -> r3 [valid correctness base; LOCAL EVIDENCE INCOMPLETE]
              -> r4 [correctness PASS; CURRENT / MEASUREMENT_BLOCKED]
DTYPE-SPECIAL-X: R31B-V011 [Parent] -> V001 [39/39 correctness PASS; CURRENT / MEASUREMENT_BLOCKED]
ASYNC-TRIPLE-X: R013-derived seed -> V001 [runtime 507035 unresolved; collision-frozen]
EXT-ASCEND-X: V001 [27/27 correctness failures after retained fix attempts; PARKED]
```

The complete revision ledger lists every retained version, including Main-1 ASYNC collision evidence and parked EXT results. Historical versions with no recoverable local samples remain `MISSING_LOCAL_SCORE` or `LOCAL_SCORE_INVALID`; no result was inferred from compile, correctness, later revisions, or Official scores.
- SCHED-ROWGROUP-X V001 属 Main-2。已核实 Judge 正式结果 15/15、Official 22.27、LOCAL/SIDECAR/REMOTE SHA 一致；本地 33x100 配对改善方向与 Official 上升方向一致。局部 latency 百分比与 Official score points 不可直接比较，单条样本不足以改本地 evaluator。结果已追加到 shared local-online calibration 表。

## 当前五个 MAIN-1 Agent 与双轨工作

| Route | Agent | Worktree | Branch | Track-A | Track-B |
|---|---|---|---|---|---|
| R31B | `01a0d88c-cf41-7fd1-9add-ead2863fc070` | `/Users/sunyiyang/Desktop/Project/cann-sixlane/R31B` | `exec/sixlane-20260923-r31b` | V016 等同 binary 资格与窗口 | H1/H5/H6：BF16 缓存、重复参数转换、索引递增 |
| R31A | `01a0d88c-d016-71f0-a73e-cd7c78d6c160` | `/Users/sunyiyang/Desktop/Project/cann-sixlane/R31A` | `exec/sixlane-20260923-r31a` | V021 runner NPU correctness 与窗口 | Rsqrt、FusedMulAdd、对齐 copy，先跨路线去重 |
| MIX-A | `01a0d83b-897c-7453-bac5-b30fdf63ebb4` | `/Users/sunyiyang/Desktop/Project/cann-sixlane/MIX-A` | `exec/sixlane-20260923-mix-a` | V007 exact Parent/Candidate qualification | wait 合并、aligned copy、单行末尾 wait |
| MODE-X-R015C | `01a0d83b-88b3-7510-b242-ac2a5545314c` | `/Users/sunyiyang/Desktop/Project/cann-sixlane/MODE-X-R015C` | `exec/sixlane-20260923-mode-x-r015c` | r4 correctness PASS 后等资格与窗口 | row-segment mapping、staging 容量、descriptor batches |
| DTYPE-SPECIAL-X | `01a0d83b-87f9-7980-8130-337043bce812` | `/Users/sunyiyang/Desktop/Project/cann-sixlane/DTYPE-SPECIAL-X` | `exec/sixlane-20260924-dtype-special-x` | V001 exact runner identity、same-binary 与窗口 | copy elision、aligned output、irregular narrow-row batching |

每条 active Route 都保留原 Agent/worktree/branch。当前无干净窗口时 Track-B 只做本路线研究；不在未收到 Main terminal decision 前叠加性能 Revision。

## 三条后续 Route 候选（只研究，不建立 Route）

### MULTIROW-STRIDED-DMA-X

- `CORE_MECHANISM`：一个 block 处理连续多行时，以 strided multi-block copy descriptor 搬运相同行内 segment，减少逐行发起的 DMA descriptor；每行数学和 core 映射保持不变。
- `TARGET_BOTTLENECK`：多行中宽 shape 的 MTE2/MTE3 copy setup 次数。
- `WHY_NOT_DUPLICATE_WITH_ACTIVE_FIVE`：Main-1 当前候选没有跨多行批量描述同一列区间的 2D strided transfer。MODE H05 只讨论同一行相邻 segment descriptor，轴不同；实施前仍需与 MODE owner 的新研究结果复核。
- `HISTORICAL_EVIDENCE`：R015 在 `idea-pool-29-routes.md` 标为尚未专门验证的 multi-row strided DataCopy；既有多行 tile 不等于 descriptor 数量减少已获证明。
- `EXTERNAL_EVIDENCE`：vLLM-Ascend `add_rms_norm_bias_multi_n.h` 在 rowFactor 下为多个 row chunk 分配 UB，并以 `calc_row_num * numCol` 传输；这是架构参考，不代表目标形状或 DAV_2201 性能结论。来源：[pinned source](https://github.com/vllm-project/vllm-ascend/blob/2bb3f44716f3505d2723a4e5badb10211a6c5589/csrc/moe/add_rms_norm_bias/op_kernel/add_rms_norm_bias_multi_n.h#L64-L81) 与 [copy path](https://github.com/vllm-project/vllm-ascend/blob/2bb3f44716f3505d2723a4e5badb10211a6c5589/csrc/moe/add_rms_norm_bias/op_kernel/add_rms_norm_bias_multi_n.h#L125-L132)。
- `EXPECTED_SHAPES`：R>1、D=256–4096 的连续行组；包含整行、尾 tile 和非 32-byte 行长。
- `RISKS`：descriptor stride 单位、尾部长度、源/目的跨度容易出错；MTE 延迟可能比 setup 主导；同路线 MODE H05 有邻接研究。
- `MINIMUM_V001_EXPERIMENT`：先建立 descriptors 的 byte-level 地址模型；只改一类 copy 调用；CANN build/link 后跑精确拷贝与完整算子 correctness。未通过 correctness 即停止，不计时。
- `WHY_IT_DESERVES_A_SLOT`：可直接验证一项未被当前候选覆盖的 DMA 调用成本，且保留每行数学，可为多行中宽 shape 提供清晰信息。

### LOWP-SQSUM-ACCUM-X

- `CORE_MECHANISM`：只在有明确输入范围依据的 FP16 小/中宽 shape，将 sum-of-squares 中间 accumulator 的表示从 FP32 改为低精度；其余 residual、affine 和输出路径不动。
- `TARGET_BOTTLENECK`：低精度输入扩大到 FP32 与 FP32 reduction scratch 带来的 UB/向量指令开销。
- `WHY_NOT_DUPLICATE_WITH_ACTIVE_FIVE`：Main-1 当前 DTYPE V001 的目标为 FP32 conversion path；R31B V016 只改低精度宽行 tile 大小，未改 square-sum accumulator precision。该候选是一个独立数学精度变量，不应与上述路线同轮实现。
- `HISTORICAL_EVIDENCE`：R004 与 G001 有低精度中间量失败记录；只说明未受约束的 cast 风险，不证明受限形状方案有效。需把历史失败作为强反例。
- `EXTERNAL_EVIDENCE`：vLLM-Ascend low-precision path 显示 Cast、FP32 reduction scratch 与输出 Cast 的分离步骤：[pinned split-D source](https://github.com/vllm-project/vllm-ascend/blob/2bb3f44716f3505d2723a4e5badb10211a6c5589/csrc/moe/add_rms_norm_bias/op_kernel/add_rms_norm_bias_split_d.h#L128-L150)。这是设计结构参考，不是精度或性能证据。
- `EXPECTED_SHAPES`：仅限官方/本地输入矩阵可证明值域安全的 FP16 D=64–1024；若没有值域依据，不建立 V001。
- `RISKS`：RMS 平方和累积误差会改变 normalization，可能直接超容差；hidden workload 值域未知；UB 节省不一定超过误差与转换成本。
- `MINIMUM_V001_EXPERIMENT`：在任何 NPU kernel 前以极小/极大、近零、符号混合输入运行 CPU 数值 sweep；只有确定容差有充分余量后，才做单 shape CANN compile 与 exact correctness。
- `WHY_IT_DESERVES_A_SLOT`：若安全范围确有可证明子域，可回答低精度累积能否减少 FP32 中间开销；现阶段只进入规划候选，不授权实现。

### TILE-ACROSS-CORES-PIPE-X

- `CORE_MECHANISM`：探索把不同 D tile 的 producer/consumer 阶段分配给不同 core，以 tile queue/显式 workspace 交接数据；不做跨 core 部分和归约，也不增加 reduction tree。
- `TARGET_BOTTLENECK`：低 row count、宽 D 时可用 core 数不足，单 core 内 MTE 与 Vector 阶段串行。
- `WHY_NOT_DUPLICATE_WITH_ACTIVE_FIVE`：五个候选均按 row 或 row group 分配工作；R31B Track-B 的 D-split partial reduction 是另一机制。本想法要求跨 core tile handoff，不把 D-slice reduction 换名字。
- `HISTORICAL_EVIDENCE`：R008 在 29-route 池中仍是 PLANNED；D-slice 的 C001 TLE 是风险证据，不直接否定无 D-slice 的 producer/consumer 方案。
- `EXTERNAL_EVIDENCE`：FlashInfer fused AddRMSNorm 在 CUDA 使用 `cluster_n` 切分 H，并根据 shared-memory footprint 选择 cluster；这是另一 GPU 的 cluster 范例，不能推定 DAV_2201 有等价同步或共享存储。[pinned source](https://github.com/flashinfer-ai/flashinfer/blob/bf82326b0c524048c7f810da9f0022f8316ac3ff/flashinfer/norm/kernels/fused_add_rmsnorm.py#L79-L110)。vLLM-Ascend 另有 split-D workspace 路径，进一步说明额外交接会带来 workspace 与同步成本：[pinned source](https://github.com/vllm-project/vllm-ascend/blob/2bb3f44716f3505d2723a4e5badb10211a6c5589/csrc/moe/add_rms_norm_bias/op_kernel/add_rms_norm_bias_split_d.h#L60-L88)。
- `EXPECTED_SHAPES`：rows=1–2，D>=8192，多 tile 且存在未用 core。
- `RISKS`：DAV_2201 是否支持所需跨 core 可见性/同步尚未确认；若用 GM workspace 交接，额外访存可能抵消收益；CUDA cluster 资料不具可移植性。
- `MINIMUM_V001_EXPERIMENT`：只做硬件/API feasibility study 与单 tile 可见性模型。若无明确合法的 core-to-core 同步，立即判为不可行，不改 Candidate、不做计时。
- `WHY_IT_DESERVES_A_SLOT`：有机会回答“宽行低并行度”是否存在非 D-slice 解法；先做低成本可行性判定可避免无效完整实现。

## Git 与共享状态

- Canonical branch：`exp/independent-breadth`。本轮推送前后均 fetch 并核对 local HEAD 与 origin 一致；R31B、R31A、MIX-A、MODE-X-R015C、DTYPE-SPECIAL-X 五条继续路线的分支在 handoff 后均无 ahead/behind，worktree 干净。
- shared `scheduler.tsv` 只调整 WIDE-X-FRESH4 自己的状态行；其 owner、worktree、branch 不变。Main-2 行和 Candidate source 均未改。
- `local-online-calibration.tsv` 追加 SCHED-ROWGROUP-X 已正式提交的一行；该结果只增加校准样本，不改变 Main-2 Route 状态或 evaluator 版本。
- canonical 工作树中其他既有修改与未跟踪文件均保持原样，没有纳入本次 stage。
