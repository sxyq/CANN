# HANDOFF-TO-MAIN — ASYNC-OVERLAP-CHAMPION-X V001

收件：Main-2
状态：V001 本地流程走完，**STOP**。不提交 Online。

## 当前需求与状态

- 目标：在 FROZEN R31B-V011 上验证 H1 inter-pass prologue param prefetch。
- 阶段：V001 声明 → 源码 → 编译链接 → 正确性 → same-binary → 交错 P/C → 本地结论。
- 完成状态：全部走完，本地结论 `NEEDS_ONE_MORE_LOCAL`。

## 本轮实际完成

1. V001 Revision 声明（`本地实验/ASYNC-OVERLAP-CHAMPION-X/V001/REVISION-DECLARATION.md`）。
2. OFAT 源码：只动 `ProcessWideLowPrecision`——把 pass-2 tile-0 gamma/bias Load 提前到
   invRms 回路之前，并删掉随之多余的 `SyncVToMTE2()`。SHA `0fc71e8e…`。
3. server3 编译链接 PASS（device.alink / submission.alink）。
4. NPU 正确性：LP 路径全部形状 OUTHASH 与 parent 逐位一致（`PASS_VS_PARENT`）。
5. same-binary + 4 组交错 P/C（32×16384 fp16 为主测）。
6. 证据与本地结论已写入并分 commit push。

## 修改或操作对象

| 对象 | 路径 |
|---|---|
| V001 源码 | `本地实验/ASYNC-OVERLAP-CHAMPION-X/V001/submission.asc` |
| 声明 / 元数据 | `…/REVISION-DECLARATION.md`, `source-meta.json`, `diff.patch` |
| 构建正确性 | `…/BUILD-CORRECTNESS.md`, `build.log`, `correctness.log` |
| 本地性能 | `…/LOCAL-PERFORMANCE.md`, `local-result.json`, `results/` |
| server3 工作区 | `/home/data4t2/lelinfeng/phase4-workspaces/ASYNC-OVERLAP-CHAMPION-X/{V001,PARENT-CORR,timing}` |

## 验证结果

| 阶段 | 结论 |
|---|---|
| BUILD | PASS |
| CORRECTNESS | `PASS_VS_PARENT`（LP 形状 OUTHASH 全等；golden FP16 容差偏差为父版已有） |
| SINGLE_CHANGE_AUDIT | PASS（仅 issue 位置移动 + 对应 sync 收窄） |
| LOCAL | `NEEDS_ONE_MORE_LOCAL` |

## 本地结论与依据

- p10 快簇 3/4 对偏 V001（约 4–5%），1/4 反向；干净对 median 一正一负。
- parent same-binary 在 32×16384 的 B2 反复 MAD/med 超阈。
- 约 10 µs 短 kernel 的离群点把 median 与 CV 顶爆，信号落在噪声内。
- 不记 LOCAL_ACCEPTED，不推进 LOCAL_BEST；保留 Candidate 待重测。

## 补测（2026-09-28，MAIN-2 指令）

| shape | parent SB | 结果 |
|---|---|---|
| 64×16384 fp16 | 两次均 B2 超阈 + drift>1.10 | `MEASUREMENT_BLOCKED`，未硬测 P/C |
| 8×12288 fp16 | drift 1.179/1.594；a2 B2 超阈 | `MEASUREMENT_BLOCKED`，未硬测 P/C |

六字段 BLOCK 记录见 `LOCAL-PERFORMANCE.md` 补测附录与 `local-result.json`。

## 剩余工作与风险

- 三个 LP 形状均拿不到「parent SB 双块合格 + 一致 P/C」完整证据链。
- 下一步选项（不无限测）：A 安静窗口重测 64×16384；B 换更长 kernel 形状（128×16384 / 32×32768）；C 交 Main 定夺保留 Candidate。
- H1 预期收益是低个位数 µs 级，对短 kernel 测量层极敏感。
- 未改共享总账；未自行 Online；未开 V002；未 revert H1。

## 请求 Main-2

- 确认 `NEEDS_ONE_MORE_LOCAL` + 两形状 `MEASUREMENT_BLOCKED` 的归类。
- 在选项 A/B/C 中定夺下一步；本路线不自行无限补测。
