# W4-R12 V002：单 tile 归约直读

```text
ROUTE = W4-R12
REVISION = V002
DIRECT_PARENT_REVISION = V001
DIRECT_PARENT_COMMIT = d74a93a4a6add6329bfaea7b68779965e5ab76a0
DIRECT_PARENT_SOURCE_SHA256 = 04c158a5a617f88738351014154fa7645a5fcb47568c5830e6833d2806e83994
REFERENCE_PARENT = R31B V011
REFERENCE_PARENT_COMMIT = 43a1049a1e08e518c88e354a754fdebb85a96f99
REFERENCE_PARENT_SOURCE_SHA256 = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE = 本地实验/W4-R12/V002/submission.asc
CANDIDATE_SOURCE_SHA256 = 4c32c79b65087431a12fe907b6d3a1378f010c6186e281f1216edae882588d7c
CHANGE = tileCount==1 时直接读取唯一 partial sum，略去一次单元素 ReduceSum。
COMPILE = PASS（server3 实际依赖为 V002/submission.asc）
CORRECTNESS = PASS（1x64 FP32，64/64）
LOCAL_SCORE = NONE
LOCAL_DELTA = NONE
LOCAL_VERDICT = MEASUREMENT_BLOCKED
CURRENT_LOCAL_BEST = NONE
OFFICIAL = NOT_SUBMITTED
PUSH = NO
```

## 假设与改动

`1x64 FP32` 命中 R12 已测的 generic cached-row 路径，只有一个 64 元素 tile。原路径先将该 tile 的平方和写入 `reduceFp32[0]`，随后再以 `tileCount=1` 调用一次 `ReduceSum` 取回同一个 partial。V002 在 `tileCount==1` 时于 V→S 同步后直接读取 `reduceFp32[0]`；多 tile 情况维持原归并代码。该改动检验 tiny 单 tile 是否能省下一次归约与对应的向量/标量切换。

V001 Judge 结果报告了 `op.Process<kSingleRow>` 的 dependent template 编译错误。V002 将调用写成 `op.template Process<kSingleRow>`，并单独增加上述单 tile 改动；V001 源码、提交与评测结果均未更改。新增的 `template` 关键字不改变运行语义。

## 构建与正确性

server3 `hwnput3`，设备 4，CANN `8.5.0.alpha002`，Ascend910B3 / `dav-2201`。首次尝试未能上传到尚未建立的 Candidate 目录；其后的旧工程构建被确认未使用 V002，故不记作本版构建结果。随后在 V002 独立目录用实际依赖文件核实来源；第一次该来源构建发现 V001 原有的 dependent template 调用需要 `template` 关键字，V002 加入该语法写法后再次构建成功。

最终依赖记录指向 `/home/data4t2/lelinfeng/server_runs/W4-R12/candidate-v002-20261009/submission.asc`，与本地源 SHA256 相同。最终构建时间为 `2026-10-09T11:11:58Z`。详细输出在 `server3/compile.log`。

复用本 Route 已有 runner 与输入，对 `1x64 FP32`、epsilon `1e-5`、`availableCoreNum=8` 执行 CPU FP64 reference 对照。60 次预热后运行，输出先置 NaN；实际 kernel launch 后逐元素核对。64/64 通过，最大绝对误差 `1.924902224281766e-7`。完整输入、参考、输出、误差和 pass 标记见 `results/v002-correctness-local.reference.tsv`；阶段日志见 `server3/correctness-local.log`。本次未测其他 dtype 或 shape。

## Local 观察

设备 event runner 在 Candidate 二进制上采集 124 个计时样本。原始数值全部保留于 `results/v002-correctness-local.raw.tsv`：

```text
ROUTE = W4-R12
REVISION = V002
PARENT_SHA256 = a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SHA256 = 4c32c79b65087431a12fe907b6d3a1378f010c6186e281f1216edae882588d7c
COMPILE = PASS
CORRECTNESS = PASS
SHAPE = 1x64
DTYPE = FP32
TIMING_METRIC = Device event（主测量）；kernel-task 仅作诊断
PARENT_MEDIAN_US = NONE（本次未运行同窗口 Parent）
CANDIDATE_MEDIAN_US = 5.440000
LOCAL_DELTA_PERCENT = NONE
PARENT_RAW_SAMPLES = NONE（同窗口）
CANDIDATE_RAW_SAMPLES = results/v002-correctness-local.raw.tsv
SAME_BINARY_QUALIFICATION = event MAD/median 16.54%，未通过 R12 已用 10% 参考线
ORDER_EFFECT = runner 内 P1/P2 标签对应同一 Candidate 二进制；不能形成 P/C 次序对照
OUTPUT_ADDRESS_CONTROL = 同一进程复用 runner 输出地址；没有 Parent 对照
DEVICE_ID = 4
FREE_HBM_MB = 6291.25（ACL 读取；npu-smi 估算 6553）
LOAD_NOTE = 采集时 AICore 约 75%、AIVector 约 71%，load average 42.32/51.56/56.51；运行结束后任务已退出
LOCAL_VERDICT = MEASUREMENT_BLOCKED
```

event 中位数为 `5.44 us`，MAD 为 `0.90 us`，均值为 `48.12 us`，范围 `3.68–1300.12 us`。MAD/median 为 `16.54%`，且存在明显长尾；缺少同窗口 Parent 数据，故不计算 Local delta，不将观察写为 Local Best。

另以同一 Candidate binary、同一 runner 和 `msprof --task-time=on --aic-mode=task-based` 采集一次。124 个计时调用对应 kernel-task 中位数 `1.36 us`、MAD `0.06 us`、MAD/median `4.41%`，范围 `1.26–1.54 us`。该次 task-time 只作诊断，不替代现行 event 主测量。R12 先前 Parent P/P 的 profiler 记录属于较早窗口，不是本次配对 Parent；因此不据此宣称 Candidate 性能提升。

profiler event 样本中位数 `15.62 us`、MAD/median `72.47%`，与无 profiler 样本分开保存，不能混合统计。profiler 的完整数据库、CSV、采集日志在 `results/kernel-task/`；本次 runner raw/reference 在 `results/v002-profile.*.tsv`，命令日志在 `server3/profile.log`。

## 原始文件与状态

- `submission.asc`：本次 Candidate 源码。
- `server3/compile.log`：源码错配尝试、V002 构建错误与最终实际来源构建记录，均予保留。
- `server3/correctness-local.log`：正确性、Local runner 与资源上下文。
- `server3/profile.log`：profiler 命令、输出和前后设备上下文。
- `results/v002-correctness-local.raw.tsv`、`*.reference.tsv`：首轮 event 样本与正确性数据。
- `results/v002-profile.raw.tsv`、`*.reference.tsv`、`results/kernel-task/`：profiler 观察及完整原始采集。

最终 Candidate SHA256 已在本地和 server3 相互核对。无在途设备操作；未进行 Official 提交、push、主线合并或共享记录修改。结果类别为 `LOCAL_OBSERVATION_ONLY`，本版没有有效 Local 分数。
