# FULL-R013 V001 线上结果

## 身份

| 字段 | 值 |
| --- | --- |
| route | `FULL-R013-DOUBLE-BUFFER-PIPELINE` |
| version | `V001` |
| technology_name | `FULL-R013-V001-DOUBLE-BUFFER-PIPELINE-ARCH` |
| Git branch | `exp/full-r013-double-buffer-v001` |
| Git commit | `f27c02980018d213b9e367a7522bc97aa2f0556a` |
| 父提交 | `70aa294f72162baf3deaefa9cb58e2324c3f18e5` |
| 源码 | `提交/单方案/FULL-R013-DOUBLE-BUFFER-PIPELINE/V001/kernel.txt` |
| SHA-256 | `8e00792c69af079f99f83bd57be1ac74c6c4c560c44ed2f0bf1b544ef88db9f7` |
| 行数 / 字节数 | `1178 / 30835` |
| 平台提交 ID | `6aacb325b0477ec41e1707d2` |
| 平台提交时间 | `2026-09-18T03:42:29.228Z` |

## 线上结论

- 平台状态：`Pass`
- 通过：`15/15`
- Official Score：`18.76`（平台返回值）
- 最大输出误差：`0%`
- 结果原始 JSON：同目录 `result.json`

## 本地证据

- 服务器 CANN 9.0 工具链完成编译和链接，返回码 `0`。
- 服务器真实 NPU 精度和性能未采集：8 张 NPU 均有其他任务。
- 静态核验记录了 `kernel.txt:162` 的 `DataCopyPadExtParams<T>` 聚合初始化问题；线上 V001 源码保持 exact upstream 内容。
- 服务器日志目录：`/home/data4t2/lelinfeng/cann/实验/FULL-R013-DOUBLE-BUFFER-PIPELINE/V001-agent3-validation/`。

## 15 个测试点

| 点位 | 状态 | 误差 (%) | 耗时 (us) | 平台公开最佳 (us) |
| ---: | --- | ---: | ---: | ---: |
| 1 | Pass | 0 | 4.33 | 1.71 |
| 2 | Pass | 0 | 11.78 | 2.16 |
| 3 | Pass | 0 | 7.67 | 2.48 |
| 4 | Pass | 0 | 985.27 | 6.66 |
| 5 | Pass | 0 | 16.80 | 5.21 |
| 6 | Pass | 0 | 2004.89 | 11.45 |
| 7 | Pass | 0 | 4005.30 | 14.05 |
| 8 | Pass | 0 | 382.28 | 30.64 |
| 9 | Pass | 0 | 188.31 | 50.61 |
| 10 | Pass | 0 | 307.55 | 47.35 |
| 11 | Pass | 0 | 9230.12 | 132.05 |
| 12 | Pass | 0 | 234.34 | 76.55 |
| 13 | Pass | 0 | 25421.21 | 408.18 |
| 14 | Pass | 0 | 119086.67 | 3750.12 |
| 15 | Pass | 0 | 13176.80 | 8499.88 |

## 状态

`PASS_FROZEN`。下一条路线为 `FULL-R002-RETAINED-Y`；阶段 2 仍处于 breadth-first 阶段，不进入同一路线 V002。
