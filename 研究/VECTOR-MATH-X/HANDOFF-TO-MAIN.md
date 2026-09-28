# HANDOFF-TO-MAIN — VECTOR-MATH-X（第一阶段收口）

DATE=2026-09-29
FROM=Route Agent, LANE M2-2 = VECTOR-MATH-X（worktree `/Users/sunyiyang/Desktop/Project/cann-m2-vector`，branch `m2/vector-math`）
TO=Main-2
本轮性质：只读研究 + 路线文档。**未改 Kernel、未建 V003、未跑正式 performance、未提交 Online。**

---

## 1. V002 闭环判断

**结论：V002 在 Revision 级已闭环（终局），但不是性能终局；路线本身未关。**

已闭环的部分（均有正式记录支撑）：

| 项 | 状态 | 证据 |
|---|---|---|
| 本地结论 | `MEASUREMENT_BLOCKED`（终局结论枚举，不是 NOT_COMPLETE、不是 LOCAL_REJECTED） | `技术路线/路线成绩表.tsv`、`本地实验/VECTOR-MATH-X/V002/local-result.json` |
| 资格化预算 | 3 次认真独立尝试（d6/d5/d4）全部用尽，calibration（s=81 / blocks=3 / batch_n=4）全部试过 | `本地实验/VECTOR-MATH-X/V002/support/QUALIFICATION-SUMMARY.md` |
| 包固化 | SOURCE_SHA `06095762…` frozen，不再改动 | source-meta.json + registry |
| LOCAL_BEST | **没有推进**（正确：仅 LOCAL_ACCEPTED 可推进） | 路线成绩表 `LOCAL_BEST=NONE`（batched 线仍记 V001） |
| 信号定性 | 中形状配对 -14.0%（5/5）/ -9.2%（3/3）保留为 **directional only**，不可采信 | QUALIFICATION-SUMMARY paired table |

仍缺什么才算「记录字段完整」（不阻塞路线，建议 Main 补记）：

1. `MEASUREMENT_BLOCKED` 的法定随附字段在 local-result.json 里是嵌在 `samebinary_FINAL` 叙述中的，建议 Main 在共享总账侧明确记齐 `BLOCK_REASON / DATE / SHAPE / DEVICE / SAME_BINARY_RESULT / RETRY_REQUIRED` 六字段。事实都已存在（DATE=2026-09-27；SHAPE=4x2048/8x1024/8x256；DEVICE=d6/d5/d4；SAME_BINARY_RESULT=FAIL MAD/med 0.104–0.395；RETRY_REQUIRED=NO——预算已用尽），只是格式散落。
2. **策略层未决**：短 kernel 长度分层门槛是否采纳（见下节）是 Planning 问题。在裁定前 V002 保持 frozen，**不安排重测**；若未来门槛修订，重测与否也由 Planning 决定，不由本路线自行重启。
3. 一处记录瑕疵：V001 `local-result.json` 仍有 JSON 语法错误（registry 已注明，verdict 以 registry 为准）。本 worktree 未改它（避免覆盖已产生证据），如需修复请 Main 知会。

**不把 V002 的 inadmissible 信号写成 LOCAL_BEST**——已遵守。

## 2. DIV_FEASIBILITY_PROBE 状态

**结论：probe 已实施且 PASS，不是「待批准实施」。任务书假设已过时，以正式记录为准。**

时间线（记录 + server3 实物核对一致）：

1. `研究/VECTOR-MATH-X/MAIN-APPROVAL-SEQ-FUSE-2-PROBE.md`（2026-09-27）：Main 已批准 probe（范围仅 feasibility，不是 performance revision）。
2. `研究/VECTOR-MATH-X/DIV-FEASIBILITY-PROBE.md`（2026-09-27）：VERDICT **PASS**。
3. server3 实物：`/home/data4t2/lelinfeng/phase4-workspaces/DIV-PROBE/`，probe binary SHA256 = `17be3095f5e11c078dad4b2bb29822bd8977ff3206d6231152c52fcdd4e94bd1`（与结论文档一致），`logs/P1..P5.log` 五份原始日志与结论文档逐条相符。
4. 本轮已把原始日志归档进路线证据目录：`本地实验/VECTOR-MATH-X/SEQ-FUSE-2-PROBE/probe-raw-logs.txt`（P1–P5 全文 + build 关键行 + 四文件 SHA）。

关键事实（写进 V003 实现约束）：

- **安全形**：`Div(dst, ones, denom, 8或1)`，dst/ones/denom 三个互异 slot，全部 32B 对齐基址，无 4B 偏移子切片。
- **Div 精度**：精确 IEEE 单精度除法，P1/P5 全 sweep **max_ulp=0.000**（vs double 参考），与 Rsqrt 的 fast-approx 性质完全不同。
- **507035 根因**：4B 偏移操作数（P3 复现，`ACL_FAIL sync=507035`）。
- **第二类危险**：`dst==src1` 不报错而是**静默数据损坏**（P2，ulp 达 5.8e10），V003 必须 dst 与两源互异。
- 证据解读注意：P4 原始日志 `max_ulp=4.8e11` 是探针把 count=1 未写入的 slot 1–7 也读出来比对的**探针伪影**；按规格只看 element 0，它是精确的（ulp=0）。结论文档的处理正确。

**因此：DIV probe 不需要再实施；V003（SEQ-FUSE-2）实现的前置条件已满足，待 Main-2 批准即可进入实现。**

## 3. 下一轮假设与优先级建议

详见 `研究/VECTOR-MATH-X/TRACK-B-HYPOTHESES-NEXT.md`（4 条，全部数学路径内）。

| 顺位 | 假设 | 成熟度 | 一句话 |
|---|---|---|---|
| **1（建议批准）** | **VMX-N1 SEQ-FUSE-2** | READY_FOR_MAIN_REVIEW | 分母 tail 纯 V 链 + broadcast apply，端到端消 2×V/S round-trip + 标量除法 |
| 2 | VMX-N2 BATCHED-DENOM-CHAIN | NEEDS_MORE_EVIDENCE | batched 路径 B 行分母单链，需 N1 先落地 |
| 3 | VMX-N3 ZERO-PULL-BCAST | NEEDS_MORE_EVIDENCE | 消 Duplicate/pull 构造，需独立 feasibility probe |
| 4 | VMX-N4 PACKED-RECIPROCAL | NEEDS_MORE_EVIDENCE | 1 元素 tail → count=8 打包，UB 布局前提 open |

**建议 Main-2 本轮只批 N1 = SEQ-FUSE-2（V003，DIRECT_PARENT=FROZEN_R31B_V011，草稿声明已在 spec）。** N2–N4 排后，任何情况下不与 N1 合并。

## 4. 测量策略裁定请求（影响 V003 能否拿到可采信证据）

短 kernel（5–6µs）same-binary MAD/med ≤ 0.10 门槛在 d6/d5/d4 三次认真尝试下不可达（最好 0.104）；而本路线主目标形状恰在该带。不改测时规范的前提下，Track-B 文档第 1.2 节给出四条合规策略：

- **S1 形状加长（首选）**：换 ≥10–12µs 的行密集形状（如 16x2048 / 32x1024 档）做正式判定，机制不变。
- **S2 控制锚定**：1x32768 作 same-binary PASS 控制；中形状 delta 只作方向证据。
- **S3 tail-cost micro-probe**：仿 DIV probe，合成 kernel 只跑分母 tail 链，长循环 device-event 计时给出 µs/row 机制上界。
- **S4 不自改门槛**：长度分层门槛归 Planning 裁定。

**请 Main-2 决定：**
(a) 是否把 S1+S3 写入 V003 的 LOCAL_MEASUREMENT_PLAN（推荐：是）；
(b) 长度分层门槛问题是否提交 Planning（本路线不自改规范）。

## 5. 本轮边界遵守声明

- 未改 Kernel / 未创建 V003 / 未跑正式 performance / 未提交 Online。
- 未改共享总账（`技术路线/*.tsv`、`调度/*`）；只写本路线 `研究/VECTOR-MATH-X/` 与 probe 证据归档 `本地实验/VECTOR-MATH-X/SEQ-FUSE-2-PROBE/`。
- 未触碰其他 lane worktree；Rsqrt 未重提；dtype 特化未触碰。
- OFAT 纪律保留：V003 实现时单变量 = denominator-tail placement。

**STOP。等待 Main-2 批准。**
