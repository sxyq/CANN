# W4-R10 ACTIVE-CORE-D-AWARE-X — V001 REVISION DECLARATION

## Rule refresh

`RULE_REFRESH_RECEIPT` 已在本轮发出，六项入口逐项实读：

- `AGENTS.md`
- `.agents/skills/cann-route-executor/SKILL.md`
- `项目规则/实验总则.md`
- `项目规则/执行约定.md`
- `项目规则/服务器实验规范.md`
- `项目规则/本地性能测试规范.md`

ACTIVE_PORTFOLIO = W4；ROUTE = W4-R10 ACTIVE-CORE-D-AWARE-X；REVISION = V001；
CURRENT_PARENT = R31B V011；CURRENT_LOCAL_BEST = R31B V011。

规则正文写的是 `ACTIVE PORTFOLIO W3`，而 `worktrees/w4/` 与 `w4/r0X-*` 分支已存在。
本 Route 按 Planning 的显式指令以 W4 执行，此项差异记为 `STATE_SYNC_GAP`，
交 Record Owner 与 Planning 处理，Route Agent 不改共享记录。

## Parent identity

| item | value |
|---|---|
| parent source | `归档/历史工作区/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc` |
| parent sha256 | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| online sidecar | `线上结果/R31B/V011/submission.asc` |
| online sidecar sha256 | `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3` |
| online result | 15/15 PASS, Official 45.16 |

PARENT_CONFLICT = NO。

## DUPLICATE_AUDIT

```text
DUPLICATE_AUDIT
ROUTE = W4-R10
RELATED_OLD_ROUTES = SCHED-CHAMPION-X V001/V002/V003; SCHED-ROWGROUP-X V001;
  R31B V008 (D-slice); R31B V014 (tiny/deep batch); EXT-ASCEND-X V001
  (ChooseTiling); CASE47-SMALL-CLUSTER-CHAMPION-X V001; TINY-FIXED-OVERHEAD (Track-B)
SAME_MECHANISM_ALREADY_TESTED = NO
WHAT_IS_DIFFERENT = 历史路线都没有在 R31B V011 上落一个 D-dependent blockCount 上限：
  父版本 `run_kernel` 只有 `min(availableCoreNum, rowCount)` 与 `UINT32_MAX`，全程不含 D；
  SCHED-ROWGROUP-X 的 `rowWidth > 16384 → requestedBlocks = 8` 挂在 R016 父版本上，
  同一版还改了 `rowsPerTask` 分档和 row ownership，只能算旁证；
  SCHED-CHAMPION-X V002 由 `rowBytes`（含 D）派生 `rowGroup`，改的是核内行归属，
  `requestedBlocks` 一字未动。
NEW_INFORMATION_EXPECTED = 长 D（> 8192，wide full-y 路径）且 rowCount 大于核数时，
  在保持 blockCount = min(核数, rowCount) 这一并行度上限不变的前提下，
  观察「每核行数随 D 增长」是否让更少的核反而更快，为 D-aware blockCount 提供第一份
  在 45.16 基线上的本地数据。
```

ROUTE_DUPLICATE_BLOCKED = NO。

## 机制与唯一改动

机制：D-aware 的 active-core / block-count 策略。

父版本现状（`run_kernel`，`归档/历史工作区/R31B/R31B-V011-LP-ROW-PIPELINE_kernel.asc:3548-3556`）：

```cpp
uint64_t requestedBlocks = availableCoreNum > 0 ? (uint64_t)availableCoreNum : 1;
if (requestedBlocks > rowCount) { requestedBlocks = rowCount; }
if (requestedBlocks > UINT32_MAX) { requestedBlocks = UINT32_MAX; }
const uint32_t blockCount = (uint32_t)requestedBlocks;
```

`blockCount` 只依赖 `availableCoreNum` 与 `rowCount`，与 D 无关。wide 路径
（`rowWidth > kCacheElems = 8192`）的 `ChooseWideFullYRows` 在 UB 预算内选
`wideFullYRows_`：D=32768 FP32 只能选 1，D=12288 FP16 可以选 2，D=8192 FP16 可以选 4。
也就是说 D 越大，每核承担的完整 y 行越少、per-core 串行行数越多，而核数始终是满的。

V001 = **恰好一个 D-dependent cap**：在 `rowWidth > kCacheElems`（即已走 wide full-y 路径）
时把 `requestedBlocks` 上限降到 `rowCount / wideFullYRows`，其中 `wideFullYRows` 是
同一套 `ChooseWideFullYRows` UB 预算公式在 Host 侧的镜像计算。

三点约束，逐条对齐任务书：

1. **cap 方向向下，只减不增**，任何形状的 blockCount 都不会超过父版本值，
   因此不会出现「coarse row grouping 把核数撑爆」的反向问题，也不会把并行度撑高。
2. **不引入 rowGroup**：cap 只看 `rowCount` 和 `wideFullYRows`，不引入
   `32/gcd(rowBytes,32)` 之类的行分组，行归属顺序与 per-row kernel math 一字不动。
3. **短 D 完全不受影响**：cap 的激活条件是 `rowWidth > kCacheElems = 8192`，
   8192 及以下走的还是父版本原有路径。

改动位置：`本地实验/W4-R10/V001/submission.asc` 的 `run_kernel`，
在 `const uint32_t blockCount` 之前插入 cap，其余字节与父版本相同。

## Local 矩阵（全部来自仓库既有证据，不猜 shape）

| 角色 | shape / dtype | 来源 |
|---|---|---|
| 长 D 目标 | `48x16384` FP16 | `MULTIROW-DMA-CHAMPION-X V001` 的 primary 探针（`技术路线/技术路线图.md:379`），48 > 16 且 D > 8192，cap 生效 |
| 长 D 目标（FP32） | `16x32768` FP32 | `研究/主代理/MAIN-1-W2/campaign-status.md:530` 记录的本地探针 `2x/8x/16x32768` FP32 |
| 短 D 对照 | `12x8192` FP32 | `本地实验/DTYPE-SPECIAL-X/V001/logs/unified-dtype-*` 的 12x8192 计时对；D = 8192 = `kCacheElems`，cap 恰好不激活，是最干净的边界 |
| 短 D 对照 2 | `2x8192` FP32 | `本地实验/REDUCE-HIER-X/V001/local-result.json`、`本地实验/STORE-EPILOGUE-X/V002/support/results-timing-v002-20260927/SUMMARY.md`（sb gate PASS，可作对照） |
| 边界形状 | `2x12288` FP16 | `SYNC-TOPOLOGY-CHAMPION-X V001` 的 Correctness 形状（`技术路线/全版本记录.tsv:102`），D 刚过 8192 门槛，行数只有 2，cap = `min(2/2, 2) = 2` 恒等 |
| 边界形状 | `48x12288` BF16 | `MULTIROW-DMA-CHAMPION-X V001` 的辅助方向，mixed-in-noise，D 变化而 M 受控 |

矩阵覆盖任务书要求的三类：长 D 目标（cap 激活）、短 D 对照（cap 不激活）、
边界（D 恰在 8192 门槛两侧）。D 受控而 M 变化的对照由 `12x8192`（cap 不激活）
对 `2x12288`/`48x12288`（cap 激活但 `wideFullYRows` 随 dtype 不同）承担。

MEASUREMENT_DESIGN_BLOCKED = NO。

## 执行顺序

```text
ONE CHANGE → server3 COMPILE → CORRECTNESS → LOCAL → RESULT → COMMIT → VERSION_RECORD_EVENT
```

编辑后的下一项实验动作即 server3 Compile。
