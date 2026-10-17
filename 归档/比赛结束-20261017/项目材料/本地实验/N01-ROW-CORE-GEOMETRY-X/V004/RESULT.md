# N01 ROW-CORE-GEOMETRY-X V004

## HYPOTHESIS

NONE。V004 评估的独立方向是“尾行感知的不均匀行归属/空闲核利用”，要求证据显示现有分配在尾行或少行形状上留下可利用的空闲 core。

## EVIDENCE REVIEW

- 父源码入口：`本地实验/N01-ROW-CORE-GEOMETRY-X/V003/parent/R31B-V011-LP-ROW-PIPELINE_kernel.asc`。
- 现有 launch 几何：`requestedBlocks = min(availableCoreNum, rowCount)`；Rows 小于可用 core 时，已经没有多余的空闲 core 可被再次利用；Rows 达到或超过可用 core 时，已经使用可用 core 上限。
- 现有设备侧行归属：`baseRows = rowCount / blockCount`、`extraRows = rowCount % blockCount`，前 `extraRows` 个 core 各处理一行尾差，`beginRow` 和 `localRows` 连续覆盖全部行。这已经是当前实现的不均匀尾行分配。
- V001 两行/核、V002 四行/核、V003 三行/核均只改变 `blockCount`，宽行改变几何的配对差值持续为正：V001 D=32768 Rows=2/4/8 的 FP16 为 `+20.720/+7.120/+4.320 us`、BF16 为 `+10.680/+10.620/+9.660 us`；V002 FP16 为 `+6.900/+17.560/+20.660 us`、BF16 为 `+10.920/+30.840/+31.520 us`；V003 新增 Rows=3/6/12 的 FP16 为 `+13.840/+13.920/+6.640 us`、BF16 为 `+21.760/+21.340/+19.920 us`。

## DECISION

没有证据支持一个独立于两行/核、三行/核、四行/核的尾行感知分配机制。继续改 `blockCount` 会重复前三版研究，改变设备侧行归属还会超出当前只研究分核几何的范围。

- V004 Candidate：未创建。
- V004 Runner/CMake/脚本：未创建、未修改。
- 数学、归约、DMA、参数驻留：未修改。

## COMPILE / CORRECTNESS / LOCAL

V004 没有候选机制，故未执行 Compile、Correctness 或 Local；没有用估算替代真实结果。V003 的真实三段结果保存在 V003 的 `RESULT.md`、`qualify-v011.log` 和 `paired.log`。

## COMMIT

本记录允许本地保存，PUSH=NO；不送 Official。

## NEXT

NONE。停止 ROW-CORE-GEOMETRY-X 的重复行组几何扩展，等待新的独立证据。
