# N01 当前候选设备记录

日期：2026-10-14

## 源码与构建

- Clone 起点与直接 Parent 提交：`3f5e67d4a3a2a8a39a99399e9a226ec39e847bf8`
- Parent：R31B V011，源码 SHA256 `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`
- Candidate：`本地实验/N01-REGISTER-LIVE-RANGE-X/candidate.asc`，SHA256 `e5ebabc1d68e0d1b12a7de80a7940c48a4abe6d641fcd07ae7a60c498f611773`
- 单因素变化：在 `ProcessWideFp32FullCacheRows` 归约并读出 `squareSum` 后，改用 `residualLocal` 承载 `Duplicate/Sqrt/GetValue`，替代 `xLocal`；数学操作和 Vector 发射顺序未变。
- CANN 8.5 Full-link：PASS，`2026-10-14T05:55:46Z`；未重复编译。
- 依赖文件指向本目录的 Candidate 与 Champion Parent。对象 SHA256：Parent `a8d7738508bf4d6a61eb6651104396ce85be36a8834e7505aaf8fbb77d200b14`，Candidate `8cf2a657c28ea8ed573280fe99bcbc6ae63c579b259df599d59c90cea7c220b8`。链接库 SHA256：Parent `89a86ee883bb96259de41595e3deb05a148c22bc9d59576a4fb780fb04dea47f`，Candidate `9895c5f31f4baa046c6950ea1ab31cd977b2710bc7c52bd3f46fc791fa14ddfb`。
- Runner SHA256：`44bf6754a345a6d78e356259ea5534e85532a57a7bfc8f059f9ac00ef8973ae8`。Runner 内路线标签不作为源码身份依据。

## Correctness 与 Local

| 输入 | Parent | Candidate | 说明 |
|---|---|---|---|
| rows=128、D=12288、FP32 | FAIL，max abs `8.46536255`，1,258,202 mismatches | FAIL，max abs `4.97458458`，1,258,856 mismatches | 两者 nonfinite=0，Runner rc=1 |

该 Runner 固定 Rows=128；N01 已知 Official 目标为 Rows=2、D=12288，因此这次只覆盖相同 D 与 FP32 路径，不能替代 Rows=2 的正确性证明。Parent 与 Candidate 均未通过该诊断参考，当前无法据此把误差归因到单因素变化。N01 V021 留存资料也报告 Rows=2、D=12288 的 Parent/Candidate 对参考比较失败；需先确认既有正确性参考与设备实现差异。

设备前快照 `2026-10-14T05:57:11Z`：NPU5 HBM `59876/65536 MB`，AICore `0%`；VLLMWorker_TP PID `3355594` 使用 `56354 MB`。结束快照 `2026-10-14T06:00:16Z`：HBM `59877/65536 MB`，AICore `0%`，同一进程仍在；Runner 精确进程查询返回 1。进程未被触碰。

Correctness 原始 stdout/stderr/TSV、设备前后快照、构建日志和身份记录均保存在 `本地实验/N01-REGISTER-LIVE-RANGE-X/logs/`。Same-binary 与 paired Local 未运行；`LOCAL_SCORE=NONE`，没有性能结论。当前状态：`INVESTIGATE_NEXT`。
