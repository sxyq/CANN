# N03 REDUCTION-TOPOLOGY-X V016

日期：2026-10-17
父版本：V011 Candidate（源码提交 `fe8495fd`）
状态：只读源码检索完成；未发现新的 Vector tree/partial 依赖入口；未实施 V016 Candidate，未运行服务器 Compile、Correctness 或 Local。

## 检索结论

本轮回到 V011 的四 tile lane block、32B partial 槽和 post-sweep Vector tree，读取当前路线所有相关 Candidate 与结果记录。V011 的 `ProcessWideLowPrecision` 在 `本地实验/REDUCTION-TOPOLOGY-X/V011/Candidate.asc` 中只有一条实际归约链：

- 每个 lane block 在 `blockIndex * kWideN03PartialStride` 写入一个 `ReduceSum` partial。
- 本行 tile 扫描结束后，以 `treeStride=1,2,...` 和 `leftBlock` 组成 Vector-side 二叉 `Add` 树。
- 最终从每行首槽取出平方和并进入 RMS。

已读取并归类的历史入口如下：

- V001：每 tile partial 加活动槽索引的两两 Vector tree，已执行过。
- V005：偶、奇两条 partial 累加链再做一次合并，已执行过。
- V007/V008：pair partial 与活动索引归并；V007 曾出现运行时失败，V008 删除残留归约后仍未形成可用结果，均已有记录。
- V009/V010：partial 配合 Scalar carry/stack 的平衡归并，已执行过。
- V011：四 tile lane block 的 post-sweep Vector 二叉树，已执行过。
- V012：仅改为 2-tile lane block，已执行过且变慢。
- V013：仅改变 partial 槽读取宽度，已执行过且无稳定 Local 收益。
- V014：仅改变跨 `batchRow` 的 tree 发射顺序，已执行过且无稳定 Local 收益。
- V015：仅改为首槽线性 fan-in，已执行过且无稳定 Local 收益。

V011 当前源码没有第二个未使用的 tree/partial 分支、辅助函数或替代写入路径。继续改动现有 `treeStride`、首槽 fan-in、槽读取宽度、lane block 或跨 `batchRow` 发射，都属于已覆盖的组织变化，不满足本轮 V016 的实施条件。

## 本轮命令与结果

只读命令：

```text
git status --short --branch
git log --oneline -6 -- 本地实验/REDUCTION-TOPOLOGY-X/V015 本地实验/REDUCTION-TOPOLOGY-X/V011
find 本地实验/REDUCTION-TOPOLOGY-X -maxdepth 2 -type f -print | sort
rg -n --no-heading 'treeStride|leftBlock|rightBlock|carry|kWideN03LaneBlockTiles|kWideN03PartialStride|partialIndex|blockIndex' 本地实验/REDUCTION-TOPOLOGY-X/V{001,002,003,004,005,006,007,008,009,010,011,012,013,014,015}/Candidate.asc
rg -n --no-heading 'ReduceSum\(' 本地实验/REDUCTION-TOPOLOGY-X/V{001,002,003,004,005,006,007,008,009,010,011,012,013,014,015}/Candidate.asc
diff -u 本地实验/REDUCTION-TOPOLOGY-X/V011/Candidate.asc 本地实验/REDUCTION-TOPOLOGY-X/V015/Candidate.asc
```

- 搜索结果只对应上述已记录入口。
- 未连接 `cann-server3`，未读取或修改其他路线内容，未触碰 NPU 设备。
- 未创建 Candidate、Runner、Compile 日志、Correctness 日志或 Local 原始数据。
- V015 的既有负结果仍保留在 `本地实验/REDUCTION-TOPOLOGY-X/V015/RESULT.md` 及其原始日志中。

## RESULT

V015 首槽线性 fan-in 无稳定收益；本轮没有发现新的 Vector tree/partial 依赖组织入口。按当前源码停止 V016，不重复已有 tree 组织实验。

下一实验：无。只有路线源码出现尚未覆盖的明确 Vector tree/partial 依赖入口后，再定义新的 V016 Candidate 并沿现有 paired Runner 运行 Compile、Correctness、Local。

PUSH=NO；线上提交=NO；共享记录=未修改。
