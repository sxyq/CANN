# W4-R13 V001 结果

```text
ROUTE=W4-R13-WIDE-FP32-CACHE-TAIL-X
REVISION=V001
DIRECT_PARENT=DIAG-PARAM-REUSE-01 / BATCH-REUSE-01 at f417e436
PARENT_SOURCE=研究/W4-R13/parent-tail-probe/param_reuse_source.asc
PARENT_SHA256=096647fdbb1df8b7bcd9ba09096e5548ba5339a1e89495b745476df1c8fed0f0
CANDIDATE_SOURCE=研究/W4-R13/parent-tail-probe/submission_v001.asc
CANDIDATE_SHA256=0ddd434ad22effe1df2b9ed6f8f41d17d9bcfa3a8a3b515ca13e69a5eec7032f
FOCUS_AXIS=wide-FP32 cached output writeback granularity
SINGLE_CHANGE=process each cached batch in valueLocal, then issue one contiguous Store per batch
```

## 变化依据

`ProcessWideFp32FullCacheRows` 已把一批的完整 FP32 输出保留在连续的 `valueLocal` 区域。Parent 按 tile、按行分别写回，并围绕每次写回分配事件；`121x9216` 的 `tileCount=3`，每个完整三行批次对应 9 次 Store。V001 在参数 tile 全部处理完后，以 `batchBegin * rowWidth` 为 GM 起点，将 `batchRows * rowWidth` 个元素一次写回。每批的输出字节范围与行尾长度保持精确，算术、归约、tile 宽度、缓存行数及参数读取顺序不变。

Parent 使用 `runner_ref_batch_reuse.asc` 编译，其中启用了 R13 已验证的批次边界参数等待。Candidate 将同一等待直接写入源码，使提交源自身保留这项已验证行为。本版相对该 Parent 的新增性能变化只有 batch 级连续写回；参数复用诊断不是本版的新变化。

## Compile

```text
HOST=cann-server3
TOOLCHAIN=CANN 8.5.0.alpha002
SOC=Ascend910B3 / dav-2201
COMMAND=cd /home/data4t2/lelinfeng/cann/server_runs/W4-R13/parent-tail-probe && bash build_server3.sh --target w4r13_ref_candidate_probe
RESULT=PASS, exit code 0
OUTPUT=/home/data4t2/lelinfeng/cann/server_runs/W4-R13/parent-tail-probe/build/w4r13_ref_candidate_probe
```

编译输出含 runner 中既有的 `GM_ADDR` 属性及 printf 宽度告警；目标完成链接。执行摘要见 `evidence/candidate-v001/compile-result.md`。首个 Correctness 启动因运行环境未带共享库路径而停在加载阶段，原始记录保留；按同一 R13 脚本设置库路径后的运行通过。

## Correctness

设备 3，FP32 `121x9216`，原 runner `bad=0`，返回码 0。独立 FP64 参考对 1,115,136 个元素逐项比较，full 区域 991,232 项、tail 区域 123,904 项均为 0 项超差；最大绝对误差分别为 `6.7801585e-7` 与 `5.8024017e-7`。候选与 Parent 的两组 Local 输出逐位相同。

## Local

相同二进制 runner、输入、shape、dtype、设备和参数；device event 为主计时，host wall 为辅助。顺序为 Parent A、Candidate A、Parent B、Candidate B；每次 warmup 5、samples 25。合计 Parent 与 Candidate 各 50 个原始样本。

| 对象 | 样本数 | device median | device p10–p90 | device CV | host wall median |
|---|---:|---:|---:|---:|---:|
| Parent | 50 | 34.62 us | 21.882–64.234 us | 0.439 | 94.535 us |
| Candidate | 50 | 28.32 us | 17.42–48.98 us | 0.394 | 84.48 us |

按 pooled device median 计算，Candidate Local score 为 `28.32 us`，delta 为 `-18.1976%`。分段结果方向不一致：A 段 Parent/Candidate median 为 `34.64/21.88 us`，B 段为 `33.44/35.16 us`。本轮负载下重复性未建立，因此该数值仅作 Local 观察，不提升为 `CURRENT_LOCAL_BEST`；`CURRENT_LOCAL_BEST=NONE`。Local 状态不影响后续 Official 处理。

采样快照均为 HBM 使用率 59%，容量 65536 MB，折算 `FREE_HBM=26869 MB`；AICore 约 37–43%，AIVector 约 16–21%，1 分钟 load average 约 45.84–57.04。设备上有非 R13 的 VLLM 进程，占用约 35357 MB；未对其或服务作改动。

原始 Correctness、Local 命令日志、raw TSV、逐次统计、输出文件及设备快照均在 `evidence/candidate-v001/`。本轮只覆盖 `121x9216 FP32` 的 full-y 多批和 1024 元素尾部；D40000 的既有 partial 容量问题及其他 shape 未验证。Official Score 为 NONE，本 Route 未执行提交。
