# W4-R01 V002 结果

## 变更与来源

Direct Parent 为 `W4-R01/V001`。唯一性能变化位于 `Candidate.asc` 的 FP16 wide pass-2 参数双缓冲起始槽选择：V001 按 `batchRows >= 2` 选择 B 槽；V002 改为按 `tileCount >= 2` 选择 B 槽。其余输入遍历、gamma/bias 载入、事件、算术及输出顺序保持不变。

在 `FP16 16x32768` 路径，V001 现有执行证据显示 `batchRows=1`；源码中 tile 宽度为 4096，因此此路径有 8 个参数 tile。V001 谓词在该路径选择 A 槽，V002 谓词选择 B 槽，循环中的双缓冲加载机制保持原样。这是一个参数流水线选择条件变化。

源码 SHA256：`ad6cfdf29535b4f72d00eaa6e2c0959804617081d714ab68f9517d3bfbd5e6b1`。

## 已有 Official 逐 Case 事实

V001 的 Direct Parent `R31B/V011` Official 记录为 `Pass`、15/15、45.16 分，源码 SHA256 为 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`，来源 `线上结果/R31B/V011/result.json`。该文件没有 W4-R01/V001 或 V002 的 Judge 结果；此处数据只说明 Parent 的历史 Official 表现，不代表 V002 分数。Case 文件没有给出 shape 映射，故不将 case 指向具体输入宽度。

| Case | Testcase ID | 状态 | 时间 (µs) | 分数 |
|---:|---|---|---:|---:|
| 1 | `6a9a9a99bf41025d6013eb8a` | Pass | 5.44 | 25.849 |
| 2 | `6a9a9a99bf41025d6013eb8e` | Pass | 3.34 | 48.193 |
| 3 | `6a9a9a99bf41025d6013eb92` | Pass | 5.16 | 35.499 |
| 4 | `6a9a9a99bf41025d6013eb96` | Pass | 16.55 | 30.817 |
| 5 | `6a9a9a99bf41025d6013eb9a` | Pass | 9.93 | 38.529 |
| 6 | `6a9a9a99bf41025d6013eb9e` | Pass | 28.46 | 29.620 |
| 7 | `6a9a9a99bf41025d6013eba2` | Pass | 52.34 | 23.507 |
| 8 | `6a9a9a99bf41025d6013eba6` | Pass | 69.21 | 32.802 |
| 9 | `6a9a9a99bf41025d6013ebaa` | Pass | 71.85 | 88.607 |
| 10 | `6a9a9a99bf41025d6013ebae` | Pass | 74.82 | 46.641 |
| 11 | `6a9a9a99bf41025d6013ebb2` | Pass | 160.26 | 67.111 |
| 12 | `6a9a9a99bf41025d6013ebb6` | Pass | 97.69 | 62.294 |
| 13 | `6a9a9a99bf41025d6013ebba` | Pass | 563.74 | 53.005 |
| 14 | `6a9a9a99bf41025d6013ebbe` | Pass | 16486.82 | 21.496 |
| 15 | `6a9a9a99bf41025d6013ebc2` | Pass | 9637.47 | 73.423 |

## Compile

`cann-server3` / `hwnput3`，Ascend 910B3，GCC 11.4.0，CANN 路径 `/usr/local/Ascend/ascend-toolkit/latest`。V002 Parent kernel 库、Candidate kernel 库和配对 runner 均构建成功。

| 构建对象 | SHA256 |
|---|---|
| Candidate 源码 | `ad6cfdf29535b4f72d00eaa6e2c0959804617081d714ab68f9517d3bfbd5e6b1` |
| Parent kernel 库 | `7dd6c75a0786fe8a64171f95f6eb7c670699415ed30f4d0ac17974afb0ec5f1a` |
| Candidate kernel 库 | `bd9ecdf543266f16e5390b8c74a49d05cb2d618f80938b6b57b04a6f5de9a639` |
| Paired runner | `9ab3224194c84cac1e19bd041ed3fa248976a0af7c5dcd13529e3d5bb186eca8` |

日志：`configure-server3.log`、`compile-server3.log`。

## Correctness

设备 7，FP16、blocks=8，使用 V001 对 V002 的逐位比较及现有 reference 模板。四个 shape 的 parent/candidate 位级差异均为 0，reference 模板均通过。

| Shape | Parent/Candidate 位差 | Reference 模板 | 严格逐元素超差 |
|---|---:|---|---:|
| 16x16384 | 0 | Pass | 各 1 个 |
| 8x16384 | 0 | Pass | 0 |
| 16x32768 | 0 | Pass | 各 3 个 |
| 16x16352 | 0 | Pass | 0 |

严格逐元素超差仅见于两个大 shape；最大绝对差为 0.00390625，比例低于现有模板的 0.1% 范围。V001 与 V002 的超差位置和数值一致，未修改容差或模板。逐 shape 输出保存在 `results/correctness-d7-*.log` 与 `results/correctness-summary.log`。

## Local

设备 7，FP16 `16x32768`、blocks=8。三组各 45 次 warmup、21 对计时，共 63 对 parent/candidate raw samples，顺序交错。设备 Event 为主要计时值，Wall 为辅助值。

| 计时 | Parent 中位 (µs) | Candidate 中位 (µs) | 配对差中位 (µs) | 中位变化 |
|---|---:|---:|---:|---:|
| Device Event，63 对合并 | 29.900 | 31.180 | +0.280 | +4.281% |
| Wall，63 对合并 | 84.391 | 85.601 | +0.269 | +1.434% |

| Block | Parent 中位 (µs) | Candidate 中位 (µs) | 配对差中位 (µs) | Candidate/Parent |
|---:|---:|---:|---:|---:|
| 1 | 18.060001 | 18.200001 | +0.100000 | +0.775% |
| 2 | 31.500001 | 32.340001 | +0.299998 | +2.667% |
| 3 | 30.479999 | 31.459998 | +0.340004 | +3.215% |

汇总 Local score 记录为 Candidate Device Event 中位数 `31.180 µs`，Local delta 为 `+4.281%`（Candidate 较慢）。合并样本范围为 Parent `17.720–79.900 µs`、Candidate `17.320–101.760 µs`；配对差中位数 `+0.280 µs`，p05/p95 为 `-17.800/+34.900 µs`。三组中心有明显漂移，故数字作为本次测量事实；当前 Local Best 保持 `R31B/V011`。

设备上下文：采样前 FREE_HBM 为 40632 MB、AICore/AIVector 为 0%/0%；采样后 HBM 使用率 38%、AICore/AIVector 为 41%/44%。设备 7 上有其他用户训练进程及 Ray worker；R14/R15 同设备进程快照为 NONE。该负载变化与样本离散一并保留，不改变 Official 提交流程。

全部 raw samples、每组 runner 输出及采前/采后设备数据保存在 `results/local-v002-d7-r16-d32768-b*.tsv`、对应 `.log` 和 `results/local-*-usages.txt` / `results/local-*-proc-mem.txt`。

## 版本状态

- Official Score：NONE；尚无 V002 Judge 结果。
- 当前 Local Best：`R31B/V011`，V002 本地中位结果未改善 Parent。
- Route 工作树：`/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R01-selective-param-pipeline-x`。
- 分支：`w4/r01-selective-param-pipeline-x`；PUSH=NO。
- 本版本文件均位于 `本地实验/W4-R01/V002/`；V001 与共享记录未改。
