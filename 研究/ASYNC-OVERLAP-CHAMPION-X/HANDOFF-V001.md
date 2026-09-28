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

## 选项 B 补测（2026-09-28，MAIN-2 裁定）

| shape | parent SB | candidate SB | P/C | 结论 |
|---|---|---|---|---|
| **128×16384 fp16** | w45 合格 | w45 合格 | 4 对已跑 | **LOCAL_ACCEPTED** |
| 32×32768 fp16 | w45/w55 合格 | w45 drift 边缘 / w55 失败 | 未跑 | `MEASUREMENT_BLOCKED` |

128×16384 关键数据：

```text
same-binary  P B1 r=0.017 B2 r=0.013 drift=1.021  PASS
             C B1 r=0.010 B2 r=0.011 drift=0.997  PASS
P/C 干净对   3/3 偏 V001   Δmed = -2.21% / -1.94% / -1.33%   (abs -0.46/-0.40/-0.28 us)
p10 快簇     4/4 偏 V001   Δp10 = -2.83% / -1.84% / -2.68% / -1.16%
```

方向稳定（全部独立交错对同向），幅度约 2%（绝对 0.3–0.5 µs / 21 µs kernel）。
收益不大：是否推进 LOCAL_BEST / 是否给 ONLINE_RECOMMENDATION，请 Main 权衡
OFFICIAL_ANCHOR=45.16 与 Official score 结构后再定。

32×32768 的六字段 BLOCK 记录见 `LOCAL-PERFORMANCE.md` 选项 B 附录与 `local-result.json`。

## 形状扩展（2026-09-28，Main Review 后，仍属 V001）

| shape | parent SB w45 | 干净对 median | p10 | 方向 |
|---|---|---|---|---|
| 128×16384 fp16（已接受） | 合格 | 3/3 偏 V001 | 4/4 | 同向 |
| 128×16384 bf16 | 合格 | 2/3 偏 V001 | 4/4 | 基本同向 |
| 96×16384 fp16 | 合格 | 3/4 偏 V001 | 4/4 | 同向 |

- **p10 全同向：12/12**；干净对 median：**8/11** 偏 V001（2 近中性，1 微反向）。
- 典型收益 1–3%（绝对 0.3–0.9 µs）。
- candidate B2 在 d4 上系统性漂移（w55 更差）；parent SB w45 合格故按规则开跑 P/C。

## ONLINE_RECOMMENDATION 材料

```text
建议   ONLINE_RECOMMENDATION = WORTHY（材料就绪）
门槛   ≥2 形状同向 —— 已满足（3 形状 p10 全同向）
依据   跨 dtype（FP16/BF16）、跨行数（96/128）、跨 kernel 长度（18/21/26 µs）方向一致；
       正确性 PASS_VS_PARENT；SINGLE_CHANGE_AUDIT=PASS。
风险   幅度 1–3% 对 15-case Official 均值贡献不确定；历史校准提示小局部胜可能不迁移
       （SCHED 33x100 −4.4% → Official −3.22）；个别干净对 median 近中性。
```

本 Agent 只提供材料；ONLINE_DECISION 与 Judge 提交由 Planning / Judge Owner 执行。

## 剩余工作与风险

- 32×32768 仍 `MEASUREMENT_BLOCKED`（RETRY_REQUIRED=YES）。
- 未覆盖小 D / 多 batchRows / BF16 其他宽度。
- 未改共享总账；未自行 Online；未开 V002；未 revert H1。

## 请求 Main-2 / Planning

- 确认形状扩展结论与 WORTHY 材料。
- 决定是否给 ONLINE_DECISION=APPROVED 并交 Judge Owner。
