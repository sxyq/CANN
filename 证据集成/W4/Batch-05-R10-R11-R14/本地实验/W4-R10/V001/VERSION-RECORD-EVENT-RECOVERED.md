# W4-R10 V001 既有结果事件恢复

本事件只恢复已经提交的 V001，不新增性能版本，不重跑旧实验，不改写旧证据。
证据提交为 `2ca51316eb4bb0ba53365897bba802e0e887eaf1`。

```text
ROUTE_EVENT
ROUTE=W4-R10
REVISION=V001
LAST_ACTION=EXISTING_RESULT_RECOVERY
NEXT_ACTION=DUPLICATE_AUDIT_FOR_ONE_D_AWARE_BLOCK_COUNT_POLICY
CHANGE=D>8192 且满核时按 residentRows 限制 blockCount
COMPILE=PASS
CORRECTNESS=FAIL_VS_REFERENCE_ON_PART_OF_OLD_MATRIX
FREE_HBM_MB=60948
DEVICE_ID=1
LOCAL_SCORE=11.055
LOCAL_DELTA=+11.055%
CURRENT_LOCAL_BEST=R31B V011
GIT_COMMIT=2ca51316eb4bb0ba53365897bba802e0e887eaf1
PUSH=NO
BLOCKER=NONE

VERSION_RECORD_EVENT
ROUTE=W4-R10
REVISION=V001
EVENT_KIND=EXISTING_RESULT_RECOVERY
NEW_PERFORMANCE_REVISION=NO
DIRECT_PARENT=R31B V011
SINGLE_CHANGE=D>8192 且原 blockCount<rowCount 时，按 HostWideFullYRows 推导 residentRows，将 blockCount 限至 max(1,rowCount/residentRows)
FOCUS_AXIS=active_core_count_block_count
FOCUS_VALUE=D>8192; blockCount=min(parent,rowCount/residentRows)
COMPILE=PASS
CORRECTNESS=FAIL_VS_REFERENCE_ON_PART_OF_OLD_MATRIX
LEGACY_CORRECTNESS_LABEL=PASS_VS_PARENT
LOCAL_SCORE=11.055
LOCAL_DELTA=+11.055%
LOCAL_SCORE_DEFINITION=48x12288 BF16 四组配对 delta 的中位数；正数表示更慢
LOCAL_BEST=R31B V011
OFFICIAL_SCORE=NONE
ONLINE_STATE=PAUSED
GIT_COMMIT=2ca51316eb4bb0ba53365897bba802e0e887eaf1
PUSH=NO
BRANCH=w4/r10-active-core-d-aware-x
STATUS=LOCAL_REJECTED
MEASUREMENT_QUALITY=MEASUREMENT_BLOCKED_FOR_ACCEPTANCE
STAGNATION_3_CONTRIBUTION=0
EVIDENCE_NOTE=本地实验/W4-R10/V001/local-result.json、compile-evidence.txt、diff.patch、各形状 stats 和 raw.tsv
```

## 已有数值与范围

`48x12288 BF16` 的 Parent/Candidate 对 runner reference 均为 `bad=0`。
四组 Parent 中位数为 `[19.580, 19.240, 17.340, 19.800] us`，Candidate 为
`[23.380, 19.760, 23.420, 19.180] us`；旧记录的配对 delta 为
`[19.41, 2.70, 35.06, -3.13]%`，中位数 `+11.055%`。
完整 raw samples 保留于 `pc-r48-d12288-t2/p1a-raw.tsv` 至 `p4b-raw.tsv`，
其中每组 a/b 顺序按 `PC, CP, PC, CP` 解释。

方法：`runner_ref.inc`，device event 主计时、host wall 辅助计时；`batch_n=1`，
配对 `warmup=60, samples=41, blocks=1, gap=0`，顺序 `P C C P` 两轮。
设备 1，Ascend910B3；记录空闲 HBM 60948 MB，AICore 9%，AIVector 11%；
host load 起始 `50.99/51.56/50.66`，结束 `57.17/59.16/56.09`。

Parent same-binary 三次尝试全部保留：`MAD/median` 为 `0.232, 0.057, 0.161`，
block drift 为 `0.313, 0.085, 0.126`。第二次达到旧条件，但后续 P/C cell 的
`MAD/median=0.15–0.32`，不足以将该次比较当作稳定改善证据。保持 V001 未接受；
不能由这一结果宣布整个 R10 方向无效。

## Reference 精度限制

来源：`local-result.json.correctness.shapes_parent_vs_candidate` 及对应
`parent_r*-stats.txt`、`cand_r*-stats.txt`。

- `48x16384 FP16` 两者 `bad=8`；`48x12288 FP16` 两者 `bad=11`。
- `41x16384 FP16`、`40x16384 FP16` 两者 `bad=4`；`64x12288 FP16` 两者 `bad=15`。
- 宽 D FP32 形状存在大量且随运行变化的 bad count，例如 `48x32768`：
  Parent `1413682`，Candidate `1422781`。此证据不能支持严格正确。
- `48x12288 BF16`、`12x8192 FP32`、`2x8192 FP32`、`64x8192 FP32`、
  `2x12288 FP16`、`1x32768 BF16` 的已存结果均为两者 `bad=0`。

下一性能实验必须先声明有来源的适用 domain，保持 reference 比较有效；不得把
两者同时对 reference 失败写成全部正确。当前 Local Best 仅沿用已有 Parent 记录，
不新增其全域精度或 Official 结论。前一版本事件已由本文件和本轮消息恢复。
