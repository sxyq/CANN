# FAST-BREADTH Session

当前模式：四条独立 mutation lane；每个候选只记录 compile、run、local score 和 Git commit。

固定目录：

- 本地 worktree：`/Users/sunyiyang/Desktop/Project/cann-worktrees/fast-lane-a` 至 `fast-lane-d`
- 服务器 workspace：`/home/data4t2/lelinfeng/cann/fast/lane-a` 至 `lane-d`
- 候选索引：`实验/local/FAST-BREADTH/candidates.tsv`

资源规则：只有 `AVAILABLE_DISK < 15G` 才停止新的编译、运行或 profiling；现有 NPU 服务不停止、不迁移、不清理。每个 lane 同时最多使用一张卡。

首轮候选已登记，后续只追加一行，不为单个候选创建独立报告。

2026-09-18：R005-L002 已完成一次线上提交，submission `6aad349bb0477ec41e606254`，15/15 Pass，Official Score `25.71`；原始结果位于 `实验/online/FULL-R005-LARGE-TILE/V001/6aad349bb0477ec41e606254/result.json`。
2026-09-18：R015-L001 线上结果为 Runtime Error，1/15；submission `6aad46e5b0477ec41e6a178f`，完整结果位于 `实验/online/FULL-R015-MULTI-ROW-DMA/V001/6aad46e5b0477ec41e6a178f/result.json`。
