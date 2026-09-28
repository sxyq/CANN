# ONLINE-HANDOFF — ASYNC-OVERLAP-CHAMPION-X V001

收件：统一 Judge Owner（经 Main-2 转交）
状态：Exact Online package 已固定，**本 Agent 不执行 cannjudge:submit**。

## 提交身份（三方核对前两方已过）

| 字段 | 值 |
|---|---|
| SOURCE_PATH | `线上结果/ASYNC-OVERLAP-CHAMPION-X/V001/submission.asc` |
| 行数 | 3560 |
| 字节数 | 190430 |
| LOCAL_SHA | `0fc71e8e62e39ed3b894b7e8fb8cabb6372f802c0a4c342eeb246629f8e4efe4` |
| SIDECAR_SHA | `0fc71e8e62e39ed3b894b7e8fb8cabb6372f802c0a4c342eeb246629f8e4efe4` |
| REMOTE_SHA | **待 Judge 回填** |
| LOCAL_SHA == SIDECAR_SHA | **PASS** |

提交后请核对 `LOCAL_SHA == SIDECAR_SHA == REMOTE_SHA`；不一致记
`INPUT_IDENTITY_MISMATCH`、`formalResultEligible=false`。

## Package 内容

```text
线上结果/ASYNC-OVERLAP-CHAMPION-X/V001/
  submission.asc        exact source（190430 bytes）
  submission.sha256     sidecar（0fc71e8e…）
  source-meta.json      身份与判定元数据（remote_sha256 / official_score / decision 待回填）
  diff.patch            vs parent R31B-V011 的 OFAT 差异
```

## Revision 摘要

```text
ROUTE               ASYNC-OVERLAP-CHAMPION-X
REVISION            V001
DIRECT_PARENT       R31B-V011
PARENT_SHA          a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PARENT_SCORE        45.16
SINGLE_HYPOTHESIS   H1 Inter-pass prologue param prefetch
CONTEXT_CLASS       CHAMPION_PIPELINE_SCHEDULE
SINGLE_CHANGE_AUDIT PASS
CORRECTNESS         PASS_VS_PARENT（LP 形状 OUTHASH 与 parent 逐位一致）
LOCAL_VERDICT       LOCAL_ACCEPTED（128×16384 fp16 3/3 干净对，median −1.3%..−2.2%）
ONLINE_RECOMMENDATION  WORTHY（Main-2 2026-09-28 通过）
```

## 机制一句话

在 `ProcessWideLowPrecision` 中把 pass-2 tile-0 的 gamma/bias MTE2 提前到 invRms
标量回路之前发出，使参数 DMA 与 invRms 重叠；删除随之多余的 `SyncVToMTE2()`。
不加 buffer、不加 queue、不加 MTE3 stage。

## 提交建议

- 建议 Judge Owner 使用本 package 的 `submission.asc` 原字节提交，
  命令形态：`npm run cannjudge:submit -- --yes --source 线上结果/ASYNC-OVERLAP-CHAMPION-X/V001/submission.asc`
  （具体以 `项目规则/线上提交规范.md` 为准）。
- 提交前请再次核对本地文件 SHA 与 `submission.sha256` 一致。
- 提交后把 result.json 的 remote sha / official_score / decision 回填
  `source-meta.json` 与本文件，并按规则写入 `调度/本地线上校准.tsv`。

## 风险

1. 幅度 1–3%，对 15-case Official 均值贡献不确定；Official score 结构显示缺口
   集中在 idx 14/7/1/6/4/8/3，wide LP 机制对准其中 wide/large-D 段，但迁移幅度未知。
2. 历史校准存在小局部胜不迁移的先例（SCHED 33×100 −4.4% → Official −3.22）。
3. 测量仅覆盖 FP16/BF16 wide LP 路径；小 D / 多 batchRows / 其他 dtype 路径未做配对。
4. 32×32768 曾 `MEASUREMENT_BLOCKED`（candidate same-binary 不合格），未纳入方向统计。

## STOP

package 已固定，handoff 完毕。本 Agent 不执行 cannjudge:submit，不自行 Online。
