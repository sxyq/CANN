# W4-R13：两个新尾部尺寸的 Parent 实测

本次占槽完成研究结论，未创建性能 Candidate 或性能版本。`1×9216`、`1×10240` FP32 的 Parent 均未通过 reference；两个尾部区都通过，错误出现在前面的整 tile。当前证据支持继续研究 gamma/bias 暂存区的复用顺序，不能宣布性能提升，也不能据此认定所有宽行输入均有问题。

## 实测结果

Parent 为本树 `线上结果/R31B/V011/submission.asc`，内容未修改；与 `w4/r10-active-core-d-aware-x:本地实验/W4-R10/V001/parent.asc` 的文本对比无差异。实际执行位置为 server3 `/home/data4t2/lelinfeng/cann/server_runs/W4-R13/parent-tail-probe/`。

| FP32 shape | 整 tile 超差数 | 尾部超差数 | 整行最大绝对误差（双精度 reference） | 结果 |
|---|---:|---:|---:|---|
| 1×8192 | 0 / 8192 | 无尾部 | 5.01750e-7 | PASS，控制项 |
| 1×9216 | 5631 / 8192 | 0 / 1024 | 1.20398729 | FAIL |
| 1×10240 | 7165 / 8192 | 0 / 2048 | 1.10208545 | FAIL |

容差为现有 runner 的 `atol=1e-4, rtol=1e-4`。原 runner 的 FP32 reference 与独立双精度 reference 得到相同超差数量；另一份先把 `x+residual` 舍入为 FP32 的参考计算仍有相同量级误差。两个尾部区最大绝对误差分别为 `3.36605e-7`、`5.51815e-7`，未发现非有限值。

这些是本轮新实测的研究尺寸，不是 Official shape。执行参数均为 `device=4, rows=1, dtype=0, warmup=0, samples=1, blocks=1, gap_sec=0, batch_n=1`。计时处于首次启动条件，原始值保留作诊断，不能作为稳定性结论或 Local 成绩。

硬件与环境：`hwnput3`、`lelinfeng`、Ascend910B3 / dav-2201、CANN `8.5.0.alpha002`。三次执行前 `FREE_HBM_MB=6553`，HBM 使用率 90%，AICore/AIVector 快照均为 0%；系统 load1 约 59.75–64.23。完整前后快照、命令、退出码和原始输出在 `parent-tail-probe/evidence/`。

## 新的可证伪线索

输出数据可部分由后一个参数 tile 解释：

| shape / 元素索引 | 实际值 | 正确 reference | 改用末 tile 对应 gamma/bias 后的值 | 与实际值的差 |
|---|---:|---:|---:|---:|
| 1×9216 / 700 | 2.78834248 | 1.58435519 | 2.78834255，参数列 8892 | 7.25087e-8 |
| 1×10240 / 1400 | 2.52721858 | 1.42513313 | 2.52721837，参数列 9592 | 2.05688e-7 |

这支持参数暂存区提前被下一次 MTE2 写入的假设，尚未通过单变化实验确认因果。

精确源码位置均在 Parent：

- `:2179`、`:2180`：gamma/bias 使用 `xBuf_`、`residualBuf_`。
- `:2183`、`:2187`：现有等待为 `MTE3_V`。
- `:2191`、`:2192`：同一暂存区加载下一 tile 的 gamma/bias。
- `:2203`、`:2205`：Vector 仍以这些暂存区作为乘加输入。
- `:2128`、`:2224`：FP32 直接把值写入保留区，并从保留区输出；该路径没有可直接删除的独立尾部拷贝。

`D=40000` 仅作源码推算：`:1293` 的容量选择给出 tile=2048、缓存行数=1，从而需要 20 个 partial；`:79` 只分配每行 16 个位置，`:2133` 以 tile 编号写入。该输入的目标支持范围未确认，本轮未运行。`D=36736` 超出现有原样 runner 的宽度上限 32768，本轮未运行。未取得设备端字段快照；tile 和缓存行数属于源码推导值。

## DUPLICATE_AUDIT

`MECHANISM=unchanged Parent tail-path/reference probe`。

`SEARCHED_HISTORY`：本分支无已提交 R13 实验；读取 `w3/m1/record-owner:调度/当前任务.tsv` 的 R13 行；W3 R2 至 V040（`6321ad44`）、R4 至 V031（`ce6c6dc5`）、R5 至 V028（`1efa0863`）的版本历史及相关结果；R031 集成记录、R31A/R31B、MIX、STORE/EPILOGUE 的相关记录；`m2/reduce-hier` 的 V001–V005；相关 W4 入口只通过 Git 对象读取。

`MATCH_FOUND`：partial-sum lifetime 已有 REDUCE-HIER-X V001，其他归约变体见 V002–V005；整行归一化提前执行已有 EPILOGUE-ARITH V001，合并行内写出已有 STORE-H2B。未把这些机制重新包装为本轮性能版本。旧记录对 staging initialization 的笼统描述没有在本轮转化为新 Candidate。

`WHY_NEW_OR_DUPLICATE`：旧 R13 行明确列出两个新尾部尺寸尚未实测；本轮增加了实际运行输出、整 tile/尾部分区误差及末 tile 参数误用模型。没有重测既有 `1×16384/1×32768` 失败项，没有重新展开归约结构。

## 构建与证据入口

当前唯一构建目标为 `w4r13_ref_parent_probe`，使用 `CMakeLists.txt`、`runner_ref_parent.asc`、`runner_ref.inc`、`local_types.h` 和远端未改动的 `parent_source.asc`。来源为 R10 V001 的完整 ASC executable 链路。对 runner 仅增加末次输出的二进制保存，没有改输入、reference、容差或 Parent 算法。

`compile-01.log` 至 `compile-05.log` 保留失败输出，`compile-06.log` 为成功构建。首次启动因 HCC 自带 libstdc++ 缺少 `GLIBCXX_3.4.29` 而未进入 NPU；失败日志保留。成功运行仅为本进程指定系统 `/usr/lib/aarch64-linux-gnu/libstdc++.so.6`，未修改服务器库或服务。

`abi.h`、`parent_kernel.asc`、`path_kernel.asc`、`runner.cpp` 是失败包装的保留材料，不在当前 CMake 目标中，也没有作为设备探测结果使用。

`analyze_outputs.py` 复用已保存输出生成 `evidence/reference-analysis.json`。三个 `*-output.bin` 保留完整输出，`*-raw.tsv` 保留原 runner 的全部诊断计时。原仓库忽略 `.bin`，因此只对这三个明确文件作显式暂存，不改忽略规则。

## 事件与交接

```text
ROUTE_RESEARCH_EVENT
EVENT_ID=W4-R13-PARENT-TAIL-20261008
ROUTE=W4-R13
REVISION=NONE
DIRECT_PARENT=R31B V011
PERFORMANCE_REVISIONS_ADDED=0
COMPILE=PASS (compile-06)
CORRECTNESS=FAIL (1x9216, 1x10240); CONTROL=PASS (1x8192)
LOCAL_SCORE=NONE
LOCAL_DELTA=NONE
CURRENT_LOCAL_BEST=NONE
STAGNATION_COUNTER=0
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
PUSH=NO
BRANCH=w4/r13-wide-fp32-cache-tail-x
STATUS=RESEARCH_RESULT_READY
RUNNING_DEVICE_OPERATION=NONE
VERSION_RECORD_EVENT=NONE (no performance revision)
```

`STATE_SYNC_GAP`：旧调度行使用 V001 只读分析标签；本轮接管分支没有对应性能提交。本次继续按研究事件记录，不补造 V001 性能事实。最终 commit 与提交后状态以同轮回执为准。

下一动作：先与 W4-R09 最新事实核对 `:2191` 前对 gamma/bias 暂存区复用的 `V_MTE2` 依赖是否已实测；若未覆盖，在同一 ASC 探测链路内只做这一处正确性诊断，分别报告 Parent 和诊断版本对 reference 的结果。该动作不更改归约、tile 宽度、所有权或输出形式。当前证据尚不足以认定该处同步就是唯一根因；精度可用前不采 Candidate 性能。

本轮只修改 `研究/W4-R13/`；Parent、共享 TSV、规则、Dashboard、其他 Route 工作树及服务均未修改。远端所有本轮设备命令已结束，未创建持久后台任务。
