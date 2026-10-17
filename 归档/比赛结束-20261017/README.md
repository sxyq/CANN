# CANN 比赛结束归档

状态：比赛已结束，五条路线已收口并归档。全部本地支线历史已并入 `main`；79 个非 `main` 本地分支名和五份重复路线克隆已清理，远端分支未操作。候选源码按路线版本保存在项目材料中。

## 统一基础源码

五条当前技术路线统一回到同一份基础源码：

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

## 当前五路线收口

| 路线 | 最新归档记录 | 收口事实 | Official |
|---|---|---|---|
| N01 ROW-CORE-GEOMETRY-X（原 REGISTER-LIVE-RANGE-X） | V010 源码留档；V009 最近完成测试 | V009 Compile、Correctness 完成；18 组 Local 无稳定收益 | 未提交 |
| N02 WIDE-UB-RESIDENCY-X（原 CODE-FOOTPRINT-ICACHE-X） | V017 | Compile、ldd 完成；Correctness/Local 未启动 | 未提交 |
| N03 REDUCTION-TOPOLOGY-X（原 VECTOR-ILP-DUALCHAIN-X） | V016 源码查阅；V015 最近执行 | V015 Local 无稳定收益；V016 未创建候选或运行设备 | 未提交 |
| N04 DMA-PIPELINE-OVERLAP-X（原 GM-CACHE-STREAMING-X） | V020 | Compile、Correctness、Local、Official 完成；Local 无稳定收益 | 41.21 |
| N05 PARAM-RESIDENCY-EPILOGUE-X（原 SCALAR-ADDRESS-GENERATION-X） | V016 | Correctness 8/8；四组 Local 区间跨零；Official 未运行 | V011 41.25；V012 41.12 |

上述路线的逐版本事实仍以 `技术路线/全版本记录.tsv` 和本归档内的来源材料为准。

本地实验源码入口：N01 `项目材料/本地实验/N01-ROW-CORE-GEOMETRY-X/`；N02 `WIDE-UB-RESIDENCY-X/`；N03 `REDUCTION-TOPOLOGY-X/`；N04 `DMA-PIPELINE-OVERLAP-X/`；N05 `PARAM-RESIDENCY-EPILOGUE-X/`。每个目录按原 `Vxxx` 保存版本文件和实验记录。

补充归档路线：EPILOGUE-FUSE-X 保存 V001–V003，EPILOGUE-ARITH-CHAMPION-X 保存 V001–V002，SHAPE-TILING-CHAMPION-X 保存 V001–V002。三条路线均有独立目录；FUSE V003 与 SHAPE 两版均未产生 Official 成绩，EPILOGUE-ARITH V002 Official 为 44.96（15/15）。

## 全阶段路线目录

项目根目录的 `技术路线/路线主线/` 按路线汇集 phase3、phase4、历史工作区和本次比赛结束材料的入口。phase3 R001–R029 与 FULL-R001–FULL-R029 的对应实现可逐条查看；phase3 H001 与 phase4 Fresh Small-D 的 H001 分开列出。N01–N05 的旧名和收口后名称共用路线入口，版本顺序仍按原账目区分；新增归档的 SHAPE-TILING、EPILOGUE-FUSE 和 EPILOGUE-ARITH 也有各自路线目录。

外部 ChatGPT 轨道的 R029 V003/V004 单独归入 `LEGACY-R029-WIDE-CACHED-ROW-EXTERNAL/`；phase3 MIX-A、MIX-B、R006、R017 与 R031 reconstruction 各自保留路线入口，原始材料仍位于 `归档/phase3-before-reset-20260920/`。

## 归档对象

- `项目材料/本地实验/`：全路线每个版本的源码和实验材料，包括 N01–N05、EPILOGUE-FUSE、EPILOGUE-ARITH 和 SHAPE-TILING。
- `项目材料/线上结果/`：比赛线上结果与回执；`R31B/V011/submission.asc` 为指向 `基础源码/submission.asc` 的符号链接。
- `项目材料/研究/`、`项目材料/调度/`、`项目材料/管理/`、`项目材料/证据集成/`、`项目材料/工具/`：原目录及补入的路线研究和 W2 文档。
- `项目材料/管理/提交队列/runtime/` 中的本机浏览器会话资料保留在当前工作区，按原忽略规则不写入 Git；目录说明见 `项目材料/管理/README.md`。
- `材料索引/`：从仓库根目录移入的比赛沟通文档、讨论记录及整合说明。
- `../历史阶段/`、`../历史工作区/`、`../历史整理记录/`：既有历史归档，保持原位置。
- `../../技术路线/路线主线/`：按路线进入现有材料的统一目录索引，不复制源码和记录。
- 根目录原路径通过符号链接指向 `项目材料/`，保留文档中已有的相对引用。

按版本归档的编译、运行和测量日志作为实验记录保留。历史 MIX-A Runner 目录中的两个已编译可执行文件及空构建目录已移除；W4-R14 的 CMake 生成文件与空构建目录也已移除，编译日志保留。路线源码、性能与正确性数据仍在原分类中。线上回执中的提交时源码路径作为历史记录保留；当前材料入口以 `项目材料/` 与 `技术路线/路线主线/` 为准。

