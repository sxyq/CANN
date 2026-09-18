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
2026-09-18：R019-L003 线上结果为 15/15 Pass，Official Score `18.68`；submission `6aad5c03b0477ec41e73a5b7`，完整结果位于 `实验/online/FULL-R019-NORMALIZATION/L003/6aad5c03b0477ec41e73a5b7/result.json`。
2026-09-19：R010-L001 线上结果为 15/15 Pass，Official Score `17.66`；submission `6aad6c97b0477ec41e7b50df`，完整结果位于 `实验/online/FULL-R010-TAIL-CENTRIC/V001/6aad6c97b0477ec41e7b50df/result.json`。
2026-09-19：R012-L001 线上结果为 15/15 Pass，Official Score `22.96`；submission `6aad70a5b0477ec41e7cf41b`，完整结果位于 `实验/online/FULL-R012-ALIGNMENT-ROWGROUP/V001/6aad70a5b0477ec41e7cf41b/result.json`。
2026-09-19：R009-L002 线上结果为 15/15 Pass，Official Score `18.61`；submission `6aad748eb0477ec41e7e6b20`，完整结果位于 `实验/online/FULL-R009-COPY-CENTRIC/L002/6aad748eb0477ec41e7e6b20/result.json`。
2026-09-19：R029-L005 线上结果为 Runtime Error，4/15；submission `6aad7849b0477ec41e7fd3b1`，完整结果位于 `实验/online/FULL-R029-WIDE-CACHED-ROW/V001/6aad7849b0477ec41e7fd3b1/result.json`。
2026-09-19：审计确认本轮 FULL V001 共 13 条路线；在 R030 线上终态返回后，13 条路线均已覆盖。覆盖矩阵位于 `实验/local/FAST-BREADTH/online-baseline-matrix.tsv`。
2026-09-19：R002 与 R013 的线上记录已从各自实验分支同步到当前活动分支；R012 的线上源码 SHA 与候选 commit 文件 SHA 不一致，已在矩阵中单列。
2026-09-19：R030-L004 线上终态为 Runtime Error，4/15；submission `6aad8c4db0477ec41e864499`，完整结果位于 `实验/online/FULL-R030-WIDE-PARAM-REUSE/V001/6aad8c4db0477ec41e864499/result.json`。13 条 FULL V001 均已有线上终态，停止本阶段。
