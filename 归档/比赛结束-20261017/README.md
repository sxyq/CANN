# CANN 比赛结束归档

状态：比赛已结束，N01-N10 十条源码路线及 W5-R01-R05 五条研究路线均已归档。全部本地支线历史已并入 `main`；归档整理期间曾清理 79 个非 `main` 本地分支名和五份重复路线克隆。随后 `main` 接入并推送 `origin/main` 的完整历史；原有 96 条非 `main` 远端分支尖端均由 `main` 保留，远端现在只保留 `main`。截至 2026-10-18，仓库登记 41 个本地分支引用（均已合入 `main`）和 34 个注册工作树；这些历史工作树仍保留。

## 2026-10-18 主线收口

服务器 3 上已再次核对整个 `cann/`：本地 `main` 与 `origin/main` 均为
`c5550935bdd971da076588e340343fa0659d0ab9`，本轮归档提交已通过 GitHub API 非强制更新推送完成。工作区干净；41 个本地分支引用全部是 `main` 的祖先，没有待合并的已提交分支。34 个注册 worktree 中仍有 11 个带未提交内容，内容以 N06-N10 原始结果证据和 Runner 工作树改动为主；发现的路线源码 SHA 均已在本归档对象库中，没有遗漏的 Kernel 源码。

外部 worktree 和 `/home/data4t2/lelinfeng/cann-archive-20261017.Gz8SZ7/` 的完整恢复副本继续作为取证材料保留。各 worktree 的 CMake 生成目录、ELF、对象文件和共享库不属于 Git 归档，可在备份确认后单独清理；源码、结果摘要、失败日志和原始证据不得因清理 worktree 而删除。Runner 工作树的未验证改版不改变主线工具归档，也不代表 Correctness、Canonical Local 或 Official 结果。

本次已从外部 worktree 移动 96 个明确生成的 CMake/build/out 缓存目录，共 102,486,518 bytes（约 97.74 MiB），放入仓外可恢复隔离区
`/home/data4t2/lelinfeng/cann-archive-20261017.Gz8SZ7/移除产物/外部工作树-构建缓存-20261018/`。`MOVED.tsv` 保存每个来源 worktree、原相对路径和大小，清单 SHA 已登记在 `索引/移除产物.tsv`；这些缓存未进入 Git。结果 JSON/TSV、实验日志、失败诊断、源码和 N09 ELF 取证文件均未移动。

## 统一基础源码

所有代码路线均以同一份基础源码作为归档参照：

- 基础源码：`基础源码/submission.asc`
- 基础源码记录：`技术路线/冠军/R31B-V011/RESULT_REF.txt`
- 源码 SHA256：`a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Official：`15/15`，`45.16`
- Submission ID：`6ab2c10c0304f72a56a0c5cb`

实验路线的候选源码、参数变化、构建文件和 Runner 不合入基础源码，按路线保存在 `项目材料/`，完整提交历史保留在主仓 Git 历史中。原路径 `线上结果/R31B/V011/submission.asc` 仍解析到这唯一源码实体。

五条路线分支末端及归档提交：

| 路线 | 分支末端提交 | 路线本地归档提交 |
|---|---|---|
| N01 | `ca01d193a4dbfad233a55c94a62cac86817a5a87` | 同左 |
| N02 | `c12a93289822a7da43462c9293043022dc1d84d8` | 同左 |
| N03 | `7fdf13efdc13265c60665899e10b05a75df80a9a` | 同左 |
| N04 | `1ed31156e76b672de1ff42f9ea448a07362e328f` | 同左 |
| N05 | `9ac30d1761707e438b9b3db5809d8f695e9609b3` | 同左 |

五条路线末端提交均是主仓 `main` 的祖先。路线源码和实验记录已按版本归入项目材料；没有将候选拼入基础源码。

## N01-N10 路线结果

| 路线 | 最新归档记录 | 收口事实 | Official |
|---|---|---|---|
| N01 ROW-CORE-GEOMETRY-X（原 REGISTER-LIVE-RANGE-X） | V010 源码留档；V009 最近完成测试 | V009 Compile、Correctness 完成；18 组 Local 无稳定收益 | 未提交 |
| N02 WIDE-UB-RESIDENCY-X（原 CODE-FOOTPRINT-ICACHE-X） | V017 | Compile、ldd 完成；Correctness/Local 未启动 | 未提交 |
| N03 REDUCTION-TOPOLOGY-X（原 VECTOR-ILP-DUALCHAIN-X） | V016 源码查阅；V015 最近执行 | V015 Local 无稳定收益；V016 未创建候选或运行设备 | 未提交 |
| N04 DMA-PIPELINE-OVERLAP-X（原 GM-CACHE-STREAMING-X） | V020 | Compile、Correctness、Local、Official 完成；Local 无稳定收益 | 41.21 |
| N05 PARAM-RESIDENCY-EPILOGUE-X（原 SCALAR-ADDRESS-GENERATION-X） | V016 | Correctness 8/8；四组 Local 区间跨零；Official 未运行 | V011 41.25；V012 41.12 |
| N06 UB-BANK-PLACEMENT-X | R217 | 最高实测 Local 96.815574（R008，POOR）；没有可靠 Local Best | 未提交 |
| N07 PARAMETER-TILE-REUSE-X | R100 | 最高实测 Local 102.610179（R005，POOR）；复验未证明可复现收益；R100 Local 20.681914（POOR） | 未提交 |
| N08 TAIL-GM-TRANSACTION-X | R099 | 最高实测 Local 99.440183（R087，POOR）；R099 Local 19.399426（POOR） | 未提交 |
| N09 RMS-REDUCTION-TOPOLOGY-X | R105 | 最高实测 Local 99.257867（R067，POOR）；R105 Correctness 7/7、C11 bad=0，Local 29.254252（POOR） | 未提交 |
| N10 ROW-CORE-BALANCE-X | R064 | 最高实测 Local 99.089207（R049，POOR）；R064 Local 17.482701（POOR） | 未提交 |

上述路线的逐版本事实仍以 `技术路线/全版本记录.tsv` 和本归档内的来源材料为准。

W5-R01 CROSSROW-EPI-PROLOGUE-X、W5-R02 REGISTER-SPILL-ELIMINATION-X、W5-R03 CODE-FOOTPRINT-ICACHE-X、W5-R04 GM-L2-CACHE-POLICY-X、W5-R05 VECTOR-ILP-DUALCHAIN-X 均按独立研究路线保存，没有新增 Official 成绩。N06-N10 的最高实测值属于 Canonical Local 记录，不等于 Official 分数；五条路线均没有可靠 Local Best。

本地实验源码入口：N01 `项目材料/本地实验/N01-ROW-CORE-GEOMETRY-X/`；N02 `WIDE-UB-RESIDENCY-X/`；N03 `REDUCTION-TOPOLOGY-X/`；N04 `DMA-PIPELINE-OVERLAP-X/`；N05 `PARAM-RESIDENCY-EPILOGUE-X/`。每个目录按原 `Vxxx` 保存版本文件和实验记录。

补充归档路线：EPILOGUE-FUSE-X 保存 V001–V003，EPILOGUE-ARITH-CHAMPION-X 保存 V001–V002，SHAPE-TILING-CHAMPION-X 保存 V001–V002。三条路线均有独立目录；FUSE V003 与 SHAPE 两版均未产生 Official 成绩，EPILOGUE-ARITH V002 Official 为 44.96（15/15）。

## 全阶段路线目录

项目根目录的 `技术路线/路线主线/` 有 132 个路线目录和 1,140 个版本说明页（按 `版本/*/README.md` 统计），按路线汇集 phase3、phase4、历史工作区和比赛结束材料。phase3 R001–R029 与 FULL-R001–FULL-R029 的对应实现可逐条查看；phase3 H001 与 phase4 Fresh Small-D 的 H001 分开列出。N01–N10、W5-R01–R05 各有独立入口；N01–N05 的旧名和新名仍按原账目区分。SHAPE-TILING、EPILOGUE-FUSE 和 EPILOGUE-ARITH 也各自保留路线目录。

外部 ChatGPT 轨道的 R029 V003/V004 单独归入 `LEGACY-R029-WIDE-CACHED-ROW-EXTERNAL/`；phase3 MIX-A、MIX-B、R006、R017 与 R031 reconstruction 各自保留路线入口，原始材料仍位于 `归档/phase3-before-reset-20260920/`。

## 归档对象

- `项目材料/本地实验/`：全路线每个版本的源码和实验材料，包括 N01–N05、EPILOGUE-FUSE、EPILOGUE-ARITH 和 SHAPE-TILING。
- `路线源码/对象/`：4,006 个去重源码及构建输入对象；`索引/` 保存 22,198 条来源路径映射、3,480 个 Git 源码对象和 9,414 条版本变更。
- `实验记录/结果摘要/N06..N10/`：2,156 份逐版本结果摘要保存在 Git 中。另有 11 份原始工作树恢复副本，共 288,730 个文件（约 1.4G），保存在仓库外的 `/home/data4t2/lelinfeng/cann-archive-20261017.Gz8SZ7/压缩前原始记录/`；它们不属于远端 Git 归档。当前 `索引/` 提供源码映射和移除产物记录，不包含这些恢复副本的清单。
- N02 `REGISTER-SPILL-ELIMINATION-X` 工作树中的 43 份原始诊断记录与仓外恢复副本逐字节一致，仍保留在 `/home/data4t2/lelinfeng/cann-archive-20261017.Gz8SZ7/压缩前原始记录/worktrees-N01-N05/n02-code-footprint/研究/REGISTER-SPILL-ELIMINATION-X/`；主线只补入两份收口收据，避免把单 Shape、UNCLASSIFIED 或 WIDE-X 诊断误列为 W5-R02 成绩。
- `项目材料/工具/route-runner/` 保存 9 个 Runner 源码/脚本、2 个测试 fixture 和 3 份失败诊断记录。来源工作树的 `build-smoke/`、`build-abi-smoke/` 及其对象、共享库、可执行文件未纳入 Git；现有记录为工具构建/注册失败，未产生 Correctness、Canonical Local 或 Official 结果。
- `实验记录/历史补充/HOTLOOP-INDEX-ARITH-X/V001/` 保存 5 份纯文本构建/probe 诊断日志；来源工作树的 `build-env/`、`build-formal-20261006/`、`support/build-rerun/` 等 CMake 生成目录未纳入 Git。
- N09 R075、R076 各有一份约 0.5 MB 的 AArch64 ELF 取证文件，不属于源码，且不在 Git 历史、比赛归档或原始工作树恢复副本中；它们仍保留在 `/home/data4t2/lelinfeng/cann-archive-20261017.Gz8SZ7/worktrees/n09-rms-reduction-topology/results/canonical-sweep/` 下各自版本的 `evidence/` 目录中，本次未复制进 Git。R075 SHA256：`d1d65f21d4225e85272ad9c68efa8b87c088233235fd2b804acb3054444a11ef`；R076 SHA256：`67c932dbfe44bf289c13573185e9a0a9de3da846f7f48fa6654d95e92f65d204`。R077 ELF 的内容仍可从 main 历史提交 `a047e24b20053eb566028a2460005cf1ec3d644e` 恢复。
- `项目材料/研究/`：W5-R01-R05 研究记录及各路线原始来源。
- `项目材料/线上结果/`：比赛线上结果与回执；`R31B/V011/submission.asc` 为指向 `基础源码/submission.asc` 的符号链接。
- `项目材料/研究/`、`项目材料/调度/`、`项目材料/管理/`、`项目材料/证据集成/`、`项目材料/工具/`：原目录及补入的路线研究和 W2 文档。
- `项目材料/管理/提交队列/runtime/` 中的本机浏览器会话资料保留在当前工作区，按原忽略规则不写入 Git；目录说明见 `项目材料/管理/README.md`。
- `材料索引/`：从仓库根目录移入的比赛沟通文档、讨论记录及整合说明。
- `../历史阶段/`、`../历史工作区/`、`../历史整理记录/`：既有历史归档，保持原位置。
- `../../技术路线/路线主线/`：按路线进入现有材料的统一目录索引，不复制源码和记录。
- 根目录原路径通过符号链接指向 `项目材料/`，保留文档中已有的相对引用。

按版本归档的编译、运行和测量日志作为实验记录保留，临时可执行文件与 CMake 生成目录不作为源码版本保存。本轮从当前工作区移除 7 个确认的 ELF 构建文件；归档源码与成绩记录保留。旧提交历史中的二进制对象仍由 Git 历史保存，未改写历史。线上回执里的提交时路径按原文保留；当前材料入口以 `路线源码/`、`实验记录/`、`项目材料/` 和 `技术路线/路线主线/` 为准。
