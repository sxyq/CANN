# W4-R09 V002 结果

## 单项改动

直接父版本为 `本地实验/W4-R09/V001/Candidate.asc`。在 `ProcessNarrowMidOverlap` 中，FP16/FP32 将逐行的 `SyncMTE3ToV()` 移到下一行 MTE2 输入就绪后、Vector 复用输出前；同步仍保留。BF16 分支保持原位置，因为其 Store 源为 `xBuf_`，与下一行输入 DMA 共用缓冲。函数末尾的 Store 收尾同步未改。

## Compile 与 Correctness

- CANN：`8.5.0.alpha002`，DAV-2201，device 2。
- Compile：PASS，见 `compile-attempt02.log`。首次直接启动脚本返回 126，Compile 尚未开始；使用 `bash` 启动后通过。两份日志均保留。
- Correctness：`16×4095 FP16`、8 blocks；Parent/Candidate 逐位差异 0。双方与独立 FP64 CPU reference 比较均为 0 个失败元素，最大绝对误差 `0.00390625`。见 `correctness.log`。

## Local

- Parent/Candidate：V001/V002；`16×4095 FP16`、8 blocks。
- 方法：45 warmups，31 对交错 PC/CP 样本。全部 raw samples 位于 `results/paired-r16-d4095-b01.tsv`。
- device-event 中位数：Parent `35.519999 µs`，Candidate `35.700001 µs`。
- Local delta：`+0.5068%`；Local score：`99.4958`；配对差中位数 `+0.179999 µs`。
- PC 与 CP 顺序的配对差中位数分别为 `+2.63 µs`、`−1.24 µs`；Parent/Candidate MAD/median 为 `12.61%`、`10.76%`。
- Parent 同二进制对照也显示较大跨组变化：两侧中位数 `10.50 µs`、`8.70 µs`，原始数据位于 `results/same-r16-d4095-b01.tsv`。Local 仅作观测；本版不记为 Local Best，当前 Local Best 为 UNKNOWN。
- device 2 测量前后 FREE_HBM 均为 `62259 MB`，AICore/AIVector 均为 `0%`，无进程。host load average 前后分别为 `17.42/26.22/32.92` 与 `17.46/26.09/32.84`。
- 完整运行记录见 `local.log`、`same.log`。

## Official 与身份

V001 的 Official JSON 为 `线上结果/W4-R09/V001/result.json`：15/15 通过、42.46 分，源码身份匹配。V002 未提交 Judge，Official 状态为 NONE。V002 `Candidate.asc` SHA256：`8da825652c9deeb7c73dc4c98c7b4ac7d60981abd744e1e62f6ce1607aab2db4`，与 V001 `e9d2005207230649342b790c1848687559daa205406d7fb2be1a92dd221db8a9` 不同。
