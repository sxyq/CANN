# 技术路线主线目录

本目录按路线汇集项目各阶段的入口。每个一级子目录是一条技术路线，目录内的分类链接指向归档材料；源码、结果和研究记录保存在主仓对应分类目录，完整提交历史保存在 `main`。

当前共 132 个路线目录和 1,140 个版本说明页。每条路线、每个版本各有独立入口；版本页记录源码来源、可用源码链接和已有成绩。研究事件、占位、无候选及暂未找回源码的页面均注明原因。N01–N05 的旧名版本继续按旧名分组，不与新命名版本按相同编号串接。

补入版本：EPILOGUE-FUSE-X V001–V003、EPILOGUE-ARITH-CHAMPION-X V001–V002、SHAPE-TILING-CHAMPION-X V001–V002。每条路线的版本、结果和来源材料分别从对应目录进入；结构化事实见 `../全版本记录.tsv` 与 `../路线成绩表.tsv`。

N06–N10 的逐版结果见各路线页指向的 `实验记录/结果摘要/` 清单；源码可由归档中的 `路线源码/对象/` 和 `索引/Git源码对象.tsv`、`索引/Git源码版本.tsv` 对照。W5-R01–R05 保留研究结论与原始测量材料，不作为 Official 版本。

## 旧 W4 五路线

ROW-SCALE-HOIST-X（67 版）、MODE-DISPATCH-CUTOFF-X（111 版）、SYNC-BARRIER-ELISION-X（81 版）、STAGING-LIVENESS-X（34 版）和 ALIGNED-TAIL-DATACOPY-X（1 版）共 294 个 Revision，现已逐版进入路线目录。各版本页包含 README.md、源码入口和评分记录-001.json；266 份去重源码链接到比赛归档对象库。W4 正式结果仍按原始记录标为未记录，用户提供的分数单独注明。

版本来源：[W4 归档说明](../../归档/W4/README.md)、[版本来源表](../../归档/W4/索引/revision-source-index.tsv)、[分数摘要](../../归档/W4/官方结果摘要.tsv)。

## 版本阅读方式

- 打开路线目录下标有 `LOCAL`、`ONLINE`、`RESEARCH`、`WORKSPACE` 或归档阶段名称的链接，即可进入该路线相应材料。
- 版本目录保留原来的 `V001`、`V002` 等编号；同一编号若出现在不同阶段或不同实现系列，按来源分别阅读。
- phase3 的 `R001–R029` 目录同时收录对应 `FULL-R001–FULL-R029` 材料；`PURE-Rxxx`、外部实现和旧版 R029 各自保留独立来源。
- 外部 ChatGPT 的 `R029-V003/V004` 单列于 `LEGACY-R029-WIDE-CACHED-ROW-EXTERNAL/`，不与 `R029-官方Tiling五模式/` 的 FULL 实现合并。
- `H002-PHASE3-MIX/` 下的外部 `MIX-R014-R002*` 与仓库内同名实现通过来源名称区分。
- phase3 deep-exploration 的 `MIX-A-D001..D003` 使用 `MIX-A-PHASE3-DEEP-EXPLORATION/`；不与比赛后期的 MIX-A V001–V007 连成同一版本线。
- `MULTIMODE-R031-RECONSTRUCTION/` 单列为独立路线；R031 原始多模式实现保留在 `R031-FULL-MULTIMODE-REWRITE/`。
- `H001-PHASE3-MIX` 与 `H001-PHASE4-FRESH-SMALL-D` 是两条不同路线。
- N01–N05 目录汇集原名和收口后的新名入口。新旧记录仍按各自路线账目阅读，不因版本号相同就视为同一版本。
- `N06-UB-BANK-PLACEMENT-X/` 至 `N10-ROW-CORE-BALANCE-X/` 是远端新增的五条路线；每条目录链接到结果清单和源码索引。
- `W5-R01-CROSSROW-EPI-PROLOGUE-X/` 至 `W5-R05-VECTOR-ILP-DUALCHAIN-X/` 保存五条研究路线，均没有新增 Official 结果。
- `SHAPE-TILING-CHAMPION-X/`、`EPILOGUE-FUSE-X/`、`EPILOGUE-ARITH-CHAMPION-X/` 的本地版本和研究资料已集中归档，并列为独立路线入口。
- deep-exploration 的共享 `results.tsv` 和 `scheduler-state.tsv` 保留在 phase3 原目录；各路线的结果文件已挂入对应路线入口。

## 权威记录

- 路线关系与版本摘要：[`../技术路线总表.md`](../技术路线总表.md)
- 比赛成绩及证据位置：[`../路线成绩表.tsv`](../路线成绩表.tsv)
- 逐版本字段：[`../全版本记录.tsv`](../全版本记录.tsv)
- 全局版本图：[`../技术路线图.md`](../技术路线图.md)

目录链接只提供导航；所有事实以其指向的归档记录和以上索引为准。线上回执中的提交时路径保留原貌，当前源码入口以本目录和项目材料为准。
