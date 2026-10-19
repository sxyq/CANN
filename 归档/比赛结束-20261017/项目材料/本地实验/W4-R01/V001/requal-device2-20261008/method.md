# V001 设备 2 复测方法

本次只补充 V001 测量证据，不新增性能版本。Parent 与 Candidate 算子源码沿用 c8a1577ddcb53dd3f67048d09117a865480af359；Direct Parent 为 R31B/V011，V001 尚未成为 Local Best。Online=PAUSED，PUSH=NO。

## 复用与变动

本机工作树为 `/Users/sunyiyang/Desktop/Project/cann/worktrees/w4/R01-selective-param-pipeline-x`，分支 `w4/r01-selective-param-pipeline-x`。远端沿用 `cann-server3:/home/data4t2/lelinfeng/cann/w4/W4-R01/V001`。原有动态库、旧日志、设备 0 的 `support/requal-20261008/` 均保留。

只扩展 `support/runner_main.cpp`：增加同一 Parent 函数的 P/P 对照，并在计时前分别核对两个输出与独立 CPU reference。原有单次 launch 的 device-event 计时边界、输入模式、分配、H2D、预热逻辑均保留。只重新编译 host runner，复用原有 P/C 动态库。

## 精度口径

reference 根据 `归档/历史整理记录/shared/PROBLEM.md` 的公式计算，与 `归档/历史工作区/WIDE-X/scripts/AddRmsNormBias.py` 的数学运算顺序一致；CPU 使用 FP64 中间值，最终转 FP16。epsilon=1e-5；逐元素 `abs(actual-reference) <= 1e-3 + 1e-3*abs(reference)`。初始补充测试要求全部有限且全部通过，失败日志保留为 `correctness-r16-d16384.log`。

随后对齐既有 `归档/历史工作区/WIDE-X/scripts/verify_result.py`：rtol=atol=0.001，允许不匹配比例 tol=0.001，即 0.1%。此比例在本次测量前已经存在于模板，初始补充测试多加了全通过要求。没有改变逐元素容差；严格计数与模板判定分别输出，未把严格失败改写成零差异。四个 shape 的双侧 reference 模板判定通过后才测 Candidate 性能。分别报告 Parent/reference、Candidate/reference、Parent/Candidate；不推断正式 Judge 结果。

精度只覆盖原有 activation_matrix 中的四个 FP16 输入，blocks=8：16x16384（ON）、8x16384（OFF，单行）、16x32768（OFF，驻留容量）、16x16352（ON，边界）。任一 reference 验证失败则不进入 Candidate 性能采样，保留失败记录。

## 固定采样安排

设备只用 2。每次运行前读取可用 HBM，达到 100 MB 即执行；负载与其他进程仅作为结果背景。每次运行前后记录 UTC、HBM、AICore/AIVector 和系统负载。

初始安排为固定三轮，每轮 P/P 16x16384 → P/C 16x16384 → P/C 16x32768 → P/C 16x16352。实际先完成三组 P/P，精度口径对齐后继续三轮 P/C；各轮为目标 → OFF 对照 → ON 边界。每项 45 次交错预热、42 对样本（PC 与 CP 各 21 对）、每个 event 区间一次 launch。所有样本保留，不删除离群点，不因结果方向增加运行次数。8x16384 沿用既有 runner 的性能输入限制，只做精度。

每组及每个 shape 分别报告全样本 median、MAD/median、CV、极值、配对差值和 `(C/P-1)*100`。沿用既有测量解释线：MAD/median 与组间漂移比例均不超过 0.10；漂移按所有组中位数到双侧合并中位数的最大距离计算，另报跨度比例。该数值不用于决定是否运行。P/P 还需接近零差异；若稳定性不成立，状态为 MEASUREMENT_BLOCKED，不晋升 Local Best，不计入 STAGNATION_3。

旧 9 组跨设备样本不与本次新窗口合并。设备 0 的已有 P/P 数据单独保留，不再重复运行。无需独占、定时任务或后台轮询。

## 实际结果与限制

P/P 最后一个 post 为 04:07:46 UTC；P/C 首个 pre 为 04:16:35 UTC，相隔 529 秒。两阶段之间进行了 host-only 精度口径对齐，不能称紧邻窗口。P/C 最后命令于 04:17:26 UTC 结束；`paired-window-end.txt` 记录本 Route runner 进程数为零。设备一直使用 2，所有采样前后可用 HBM 为 60,948 MB；其他 VLLM 进程保持运行。

| 输入 / 对照 | Parent median µs | 第二侧 median µs | 中位数差异 |
|---|---:|---:|---:|
| P/P 16x16384 | 19.180001 | 18.890000 | -1.5120% |
| P/C 16x16384，ON | 24.080000 | 24.020000 | -0.2492% |
| P/C 16x32768，OFF | 21.439999 | 20.410001 | -4.8041% |
| P/C 16x16352，ON 边界 | 17.720000 | 17.409999 | -1.7494% |

每行各侧 126 个样本。P/P 三组差异分别为 -9.3098%、+10.2953%、-0.7609%；0/3 组双侧满足 MAD 比例，组间漂移 45.2444%。目标 P/C 的双侧 MAD/median 为 28.9867% / 28.6012%，组间漂移 38.4679%。目标 pooled 中位数稍偏向 Candidate，但配对差值中位数为 +0.3700 µs，不能归因到参数槽位。

四个 shape 的 Parent/Candidate 均逐位一致。按既有模板双方均通过 reference；严格逐元素差异每侧分别为 1、0、3、0。16x16384 的索引 261298，两个 NPU 输出均为 -2.75390625，reference 为 -2.75；独立 NumPy FP32 公式与分步 FP16 算术复现了这一差异。它属于继承的数值路径，未修改 Candidate 来处理，不能据此推断 Official 失败。

结论：MEASUREMENT_BLOCKED；当前 Local Best 仍为 R31B/V011。新性能版本 0、有效 Local 0、连续无改善贡献 0。原有 -5.9979% 仍是未获接受的历史观察，不与本次数据合并。

## DUPLICATE_AUDIT 与下一研究动作

MECHANISM：沿用 V001 的 FP16 pass-2 参数初始槽位，仅在 batchRows>=2 时取 B。本轮没有提出新的性能概念。

SEARCHED_HISTORY：本 Route 的 c8a1577d / 1e6b06bb；W3 R2 至 V040、R4 至 V031、R5 至 V028 的本地 Git 提交记录；R5 V012/V016 的声明、R4 V012 summary；本工作树历史版本表中的 R31/R31A/R31B、MIX、STORE/EPILOGUE 相关条目和 STORE 研究；相关 W4 R08 已提交记录与 R14 的已提交研究。跨分支材料通过本工作树 Git 对象读取，未进入其他实际工作树。

MATCH_FOUND：V001 已存在；R5 V012 是参数初始槽位的直接来源。FP16 affine 改为 FP32 中间值的方向已经出现在 R4 V012，不能把它直接另造一个新性能版本。

WHY_NEW_OR_DUPLICATE：这是同一 V001 的测试支持与测量证据补充。没有创建 V002，以上阅读不代表其他路线的所有独立方向已耗尽。

REMAINING_INDEPENDENT_AXIS：尚未确认新的独立性能轴；V001 的测量归因问题仍在。下一动作是在本 Route 使用现有 Parent，先区分真实 task duration 与 device-event 区间中的 launch/排队影响，形成一个有明确差异的短测量方案；保留现有 shape/blocks 与已完成的四组 reference 结果。不继续无变化重复当前单 launch event 方法，不借用其他 Route 的 profile 时间作为 Candidate 分数。

## 构建与复现入口

算子动态库沿用远端 `support/build/libw4r01_v001_parent.so` 与 `libw4r01_v001_candidate.so`。一次 CMake 调用意外重新编译 ASC 对象，因 `vector` 头文件路径失败；原日志保留，之后仅编译 host runner。实际 host 命令在该 Route 远端 V001 根目录执行：

```bash
/usr/bin/c++ -O2 -std=c++17 \
  -I/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/include -Isupport \
  support/runner_main.cpp -Lsupport/build -lw4r01_v001_parent -lw4r01_v001_candidate \
  -L/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64 -lascendcl -ldl -lm \
  -Wl,-rpath,'$ORIGIN:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/lib64:/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/lib64:/usr/local/Ascend/driver/lib64/driver:/usr/local/Ascend/driver/lib64/common' \
  -o support/build/w4r01_v001_paired_runner
```

每条精度与 Local 命令、返回码见对应 `.log`。本机 `support/reference_probe.py` 复现数值差异，`support/analyze_requalification.py` 从所有原始 TSV 生成 `result.json`；不启动设备工作。

任务结束前已从本工作树完整读取 `9f91895506023d917637f707bb3f61cd9d9f8765` 的八个 Route 适用规则入口，并发布新的 RULE_REFRESH_RECEIPT。只负责 R01，规则、共享表和 Dashboard 未写入。PUSH=NO，Online=PAUSED；交还 SLOT-1 不改变 Route 生命周期。
