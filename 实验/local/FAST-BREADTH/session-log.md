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
2026-09-19：阶段范围按新要求扩展为独立 FULL-R001–R029 V001 线上覆盖；现有独立线上证据覆盖 12/29（R002、R005、R006、R009、R010、R012、R013、R014、R015、R016、R019、R029），R030 作为额外路线保留。R001、R003、R004、R007、R008、R011、R017、R018、R020、R021、R022、R023、R024、R025、R026、R027、R028 已进入 `route-queue.tsv`，在 29/29 前不开始 V002/V003、Mix、Router 或已完成路线深挖。
2026-09-19：R021-V001 完成 CANNJudge 线上提交，submission `6aae26f0b0477ec41ebcb50d`，15/15 Pass，Official Score `24.01`；完整结果位于 `实验/online/FULL-R021-CANN9-BUILD-CHAIN/V001/6aae26f0b0477ec41ebcb50d/result.json`。
2026-09-19：R022-V001 完成 CANNJudge 线上提交，submission `6aae2922b0477ec41ebe0671`，15/15 Pass，Official Score `24.11`；完整结果位于 `实验/online/FULL-R022-AUTO-CODEGEN/V001/6aae2922b0477ec41ebe0671/result.json`。
2026-09-19：R023-V001 完成 CANNJudge 线上提交，submission `6aae2d7fb0477ec41ec0638b`，15/15 Pass，Official Score `24.04`；完整结果位于 `实验/online/FULL-R023-ENGINEERING-CONVENTION/V001/6aae2d7fb0477ec41ec0638b/result.json`。
2026-09-19：R011-L002 完成 CANNJudge 线上提交，submission `6aae2edab0477ec41ec128fc`，15/15 Pass，Official Score `16.18`；完整结果位于 `实验/online/FULL-R011-MANUAL-VECTOR-REDUCTION/V001/6aae2edab0477ec41ec128fc/result.json`。
2026-09-19：R017-L001 完成 CANNJudge 线上提交，submission `6aae2fceb0477ec41ec1a82c`，15/15 Pass，Official Score `27.16`；完整结果位于 `实验/online/FULL-R017-FP32-ALL-MIDDLE/V001/6aae2fceb0477ec41ec1a82c/result.json`。
2026-09-19：R018-L002 完成 CANNJudge 线上提交，submission `6aae30acb0477ec41ec2283b`，15/15 Pass，Official Score `23.42`；完整结果位于 `实验/online/FULL-R018-CAST-RINT-OUTPUT/V001/6aae30acb0477ec41ec2283b/result.json`。
2026-09-19：R020-V001 完成 CANNJudge 线上提交，submission `6aae31cbb0477ec41ec2dcb4`，15/15 Pass，Official Score `23.41`；完整结果位于 `实验/online/FULL-R020-SQRT-NORMALIZATION/V001/6aae31cbb0477ec41ec2dcb4/result.json`。
2026-09-19：R007-V001 完成 CANNJudge 线上提交，submission `6aae32e4b0477ec41ec37a64`，15/15 Pass，Official Score `20.91`；完整结果位于 `实验/online/FULL-R007-REDUCE-SUM/V001/6aae32e4b0477ec41ec37a64/result.json`。
2026-09-19：R008-V001 完成 CANNJudge 线上提交，submission `6aae33a6b0477ec41ec3e2f1`，15/15 Pass，Official Score `21.73`；完整结果位于 `实验/online/FULL-R008-TILE-CROSS-CORE/V001/6aae33a6b0477ec41ec3e2f1/result.json`。
2026-09-19：R025-V001 完成 CANNJudge 线上提交，submission `6aae3c14b0477ec41ec89958`，15/15 Pass，Official Score `25.73`；完整结果位于 `实验/online/FULL-R025-SUBMISSION-INTEGRITY/V001/6aae3c14b0477ec41ec89958/result.json`。
2026-09-19：R026-V001 完成 CANNJudge 线上提交，submission `6aae3d6db0477ec41ec974fe`，15/15 Pass，Official Score `25.65`；完整结果位于 `实验/online/FULL-R026-PERFORMANCE-MEASUREMENT/V001/6aae3d6db0477ec41ec974fe/result.json`。
2026-09-19：R027-V001 完成 CANNJudge 线上提交，submission `6aae3f98b0477ec41ecac90c`，2/15 Pass，终态 Wrong Answer；完整结果位于 `实验/online/FULL-R027-GPU-MIGRATION-REFERENCE/V001/6aae3f98b0477ec41ecac90c/result.json`。
2026-09-19：R028-V001 完成 CANNJudge 线上提交，submission `6aae40ffb0477ec41ecd02bc`，15/15 Pass，Official Score `25.06`；完整结果位于 `实验/online/FULL-R028-SCALAR-SYNC-REDUCTION/V001/6aae40ffb0477ec41ecd02bc/result.json`。
2026-09-19：R030-V001 新代表候选完成 CANNJudge 线上提交，submission `6aae421ab0477ec41ecde49f`，4/15 Pass，终态 Runtime Error；完整结果位于 `实验/online/FULL-R030-WIDE-PARAM-REUSE/V001/6aae421ab0477ec41ecde49f/result.json`。
2026-09-19：R001-V001 完成 CANNJudge 线上提交，submission `6aae444eb0477ec41ecf404a`，15/15 Pass，Official Score `23.21`；完整结果位于 `实验/online/FULL-R001-TWO-PASS-SCAN/V001/6aae444eb0477ec41ecf404a/result.json`。
2026-09-19：R003-V001 完成 CANNJudge 线上提交，submission `6aae4506b0477ec41ecfae21`，15/15 Pass，Official Score `25.31`；完整结果位于 `实验/online/FULL-R003-PURE-ASCENDC-DIRECT/V001/6aae4506b0477ec41ecfae21/result.json`。
2026-09-19：R004-V001 CompileFix-C 完成 CANNJudge 线上提交，submission `6aae46c2b0477ec41ed0bae8`，13/15 Pass，终态 Wrong Answer；完整结果位于 `实验/online/FULL-R004-LOW-PRECISION-MIDDLE/V001/6aae46c2b0477ec41ed0bae8/result.json`。
