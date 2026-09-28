# TRACK-B HYPOTHESES — EPILOGUE-ARITH-CHAMPION-X

- ROUTE=EPILOGUE-ARITH-CHAMPION-X
- BRANCH=m1/epilogue-arith-champion
- WORKTREE=/Users/sunyiyang/Desktop/Project/cann-m1-epi-arith
- DIRECT_PARENT=R31B-V011（exact source seed）
- PARENT_SOURCE_SHA=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
- PARENT_SCORE=Official 45.16
- OFFICIAL_BEST=无（新路线）
- CONTEXT_CLASS=CHAMPION_SEEDED_EPILOGUE_ARITH
- TRACK=Track-B 只读研究；本文件不改 kernel
- 记录日期：2026-09-29

---

## 0. SCOPE LOCK（变更域）

只研究 RMS 完成之后、store 之前的算术链表达式：

```text
invRms（标量，求得之后的使用方式与应用时机）
  → norm   = y * invRms（或等价形式 y / rms）
  → gamma  = norm * gamma
  → bias   = … + bias
  → （到 Store 之前为止）
```

允许：表达式重排、融合形态、标量应用时机与粒度、指令选择（Mul / Div / Axpy 类）、
算术顺序对精度与指令数的影响。

禁止（本路线一律不碰）：

| 禁区 | 归属 |
|---|---|
| store transaction / 回写形态 / MTE3 事件 | STORE-EPILOGUE-X |
| DMA、tile policy、row scheduling、dtype specialization、reduction topology | 其他路线 |
| invRms 的求值指令序列（Rsqrt / Div 求倒数 / 向量化分母 / 广播 Mul / 批量分母） | VECTOR-MATH-X |
| SCALE-FOLD（s 折入 gamma 刻度瓦片）、MulAddDst/VMLA 仿射融合、CONVERT-ONCE、MASK-TAIL、ROW-WIDE-AFFINE | EPILOGUE-FUSE-X |

---

## 1. SEED 算术链盘点（证据基础）

行号均指 `线上结果/R31B/V011/submission.asc`（3554 行）。

### 1.1 invRms 标量尾巴（每条路径同一形态）

```text
ReduceSum → SyncVToS → squareSum = GetValue(0)
meanSquare = squareSum * invRowWidth + epsilon      // 标量 mul + add
SyncSToV → Duplicate(xFp32, meanSquare, 1) → Sqrt(…, 1)
SyncVToS → invRms = 1.0f / xFp32.GetValue(0)        // 标量 div
SyncSToV
```

出现位置：`Process` L351–367、`ProcessNarrowMidOverlap` L565–574、
`ProcessFp32FullRowOutputPipelined` L1192–1201、`ProcessBf16FullRowOutputPipelined` L946–953、
`ProcessFp16FullRowOutputPipelined` L1064–1071、`ProcessWideFp32FullCacheRows` L2139–2156、
`ProcessWideFp32CachedRows` L1894–1904、批量路径 L685–703 等。

### 1.2 第二遍算术链（invRms 之后）

| 路径 | norm | gamma | bias | 备注 |
|---|---|---|---|---|
| 宽 FP32 全缓存 `ProcessWideFp32FullCacheRows`（L2194–2207） | 每 tile 每行 `Muls(valueRow[col], invRms[row])` | 每 tile `Mul(valueRow, gammaLocal)` | `Add(valueRow, biasLocal)` | y 整行驻留 `valueLocal[batchRow*rowWidth+col]`；gamma/bias 每 tile 从 GM 流入，一片参数喂 batchRows 行 |
| 宽 LP `ProcessWideLowPrecision` 第二遍（L3300–3321） | FP32 y：每 tile 每行 `Muls`；FP16 y：`ToFloat→Muls→FromFloat` | `Mul`（FP16 半域 / BF16 FP32 域） | `Add` | BF16 的 y 整行 FP32 驻留；FP16 的 FP32 工作区仅一 tile |
| 中窄 `Process` L427–461 | `Muls(valueTile, invRms)` | `Mul(…, gamma)` | `Add(…, bias)` | D≤8192 时 y 整行驻留 `valueFp32[col]` |
| 全行输出流水 `ProcessFp32FullRowOutputPipelined` L1210–1218 | 每 4096-tile `Muls` | `Mul` | `Add` | kCacheElems=8192，两 tile |
| 批量路径 L721–725 等 | 每行 `Muls` | 每行 `Mul` | 每行 `Add` | 值行驻留 |

### 1.3 结构事实

1. 第二遍固定是三条**互相依赖**的向量运算 `Muls(invRms) → Mul(gamma) → Add(bias)`，
   每条都整片写回 value 瓦片；这是表达式层面的融合/重排面。
2. `invRms` 是每行标量；`gamma`/`bias` 是每通道向量。
   宽 FP32 路径上一片 gamma/bias 参数瓦片喂 2 行（kWideFp32FullCacheRows=2），
   两行的 `invRms` 不同——把 s 折进 gamma 就必须每行一份刻度（那是 EPILOGUE-FUSE 的 SCALE-FOLD）。
3. 在宽 FP32 路径上 `Muls` 被放在 gamma/bias 的 `SyncMTE2ToV` **之后**（L2193→L2197），
   但 normalize 只依赖 y 与标量 s，完全不依赖 gamma 瓦片——
   当前表达式应用时机把标量乘绑在了参数 DMA 等待后面。
4. y 整行驻留的路径（宽 FP32 全缓存、宽 BF16 LP、D=8192 全行）里，
   `Muls` 按 tile 切成 tileCount 次标量乘；换成整行一次标量乘不改变任何元素的运算序列。
5. FP16 宽路径的 FP32 工作区只有一个 tile，整行 FP32 标量乘在缓冲上不可行
   （除非改缓冲预算——那是 UB/tile 政策，禁止）。

---

## 2. HYPOTHESES（4 条，互相不重复）

---

### EA-H1 — NORM-HOIST：驻留 y 上的整行一次 normalize（应用时机与粒度）

**SINGLE_HYPOTHESIS**
把 invRms 的应用从「每 tile 每行一次 `Muls`」改为「invRms 求得后、参数瓦片循环之前，
对驻留 y 整行每行一次 `Muls`」。算术表达式不变（仍是 (y·s)·g + b），只改标量乘的
应用时机与粒度。

**MECHANISM**
宽 FP32 全缓存第二遍，今日：

```text
for tile: Load(gamma,bias); SyncMTE2ToV
  for row: Muls(valueRow[col], invRms[row])     // tileCount*batchRows 次
  for row: Mul(valueRow[col], gamma); Add(…, bias)
  for row: Store
```

改为：

```text
for row: Muls(valueLocal[row*rowWidth], invRms[row], rowWidth)   // batchRows 次
PipeBarrier
for tile: Load(gamma,bias); SyncMTE2ToV
  for row: Mul(valueRow[col], gamma); Add(…, bias)
  for row: Store
```

**BOTTLENECK**
标量乘被切成 tileCount×batchRows 次指令发射与同等数量的 PIPE_V 屏障，
并且每次发射都排在当趟 gamma/bias 的 MTE2 等待之后。D=32768、tile=4096、2 行批次时
是 16 次 `Muls`+16 道屏障 → 2 次 `Muls`+1 道屏障；normalize 同时移出参数 DMA 关键路径。

**WHY_NOT_DUPLICATE**

| 对照 | 差异 |
|---|---|
| R31B V011 现有链 | V011 的 LP 是 MTE2 深度与行流水；第二遍算术表达式与 `Muls` 时机未动 |
| VECTOR-MATH-X（VM-H1~H4、SEQ-FUSE-2） | 他们改 invRms **求值**（Rsqrt / 向量分母 / 广播 / 批量尾巴 / Div 求倒数）；本假设不碰 Duplicate+Sqrt+GetValue 任何一行 |
| EPILOGUE-FUSE-X H1 SCALE-FOLD | 他们把 s 折进 gamma 刻度瓦片，表达式变 y·(s·g)+b；本假设表达式保持 (y·s)·g+b |
| EPILOGUE-FUSE-X H4 ROW-WIDE-AFFINE | 他们把整个仿射（s·g·b 三步）在 D=8192 驻留参数路径上整行发射；本假设只动宽路径（gamma/bias 仍按 tile 流入）上的 normalize 一步，gamma/bias 循环逐字不动 |
| EPILOGUE-FUSE-X V002/V003 MulAddDst 系 | 指令形态与融合对象不同；本假设不用任何融合指令 |
| STORE-EPILOGUE-X | 只改输出回写形态；本假设 Store 与事件逐字不动 |

**MINIMAL_OFAT_DIFF**
仅 `ProcessWideFp32FullCacheRows` 第二遍：删 L2194–2199 的逐 tile `Muls` 小循环及其屏障，
在 invRmsValues 就绪后（L2156 附近）加一个逐行整行 `Muls` + 一道屏障。其余字节相同。

**EXPECTED_SHAPES**
宽 FP32（rowWidth > 8192）：1×16384、1×32768、2×16384、2×32768、4×12288 等。
tileCount 越大收益越大；1×8192 不走此路径（作零差控制）。

**EXPECTED_RISK=零精度风险（见下）**

**PRECISION_RISK**
无。`Muls` 逐元素乘同一标量，整行一次与分 tile 多次对每个元素的运算序列完全相同，
结果按位一致。本假设是四条中唯一不改浮点结合序的。

**ASCEND_FEASIBILITY**
`Muls(dst, src, scalar, calCount)` 的 calCount 用 rowWidth（至 32768）由库内部多 repeat
展开，与既有 `Muls(valueRow, valueRow, invRmsValues[batchRow], width)` 同一调用形态
（L721 已有整行 width 调用先例）。valueLocal 行布局连续（L2123–2124 已按
`batchRow*rowWidth+col` 索引）。

**UB/CORE/DMA_IMPACT**
UB 不增；CORE 不变；DMA 不变（gamma/bias 流入节奏不动）。屏障数下降。

**SYNC_IMPACT**
标量乘移出 `SyncMTE2ToV` 之后的依赖区；整行一次屏障替代逐 tile 屏障。不改任何事件 ID。

**EXPECTED_LOCAL_PROBES**
same-binary + 交错 P/C：1×32768_fp32、1×16384_fp32、2×16384_fp32；控制 2×8192_fp32（零差）。

**成熟度：READY_FOR_MAIN_REVIEW**（首选 V001）

---

### EA-H2 — GAMMA-FIRST-AXPY：乘加重排 + 标量乘折进 bias 加（Axpy 形态）

**SINGLE_HYPOTHESIS**
把表达式从 `(y·s)·g + b` 重排为 `(y·g)·s + b`，并用 `Axpy` 把标量乘与 bias 加合成一步：

```text
今日 3 V 趟：Muls(value, y, s) → Mul(value, value, g) → Add(value, value, b)
改为 2 V 趟：Mul(value, y, g) → Axpy(out, value, s)   // out 预置 b：out = value*s + out
```

即 `out = b + (y·g)·s`。同一机制在多行批次上允许把各行的 `Mul(gamma)` 收成相邻发射
（共享参数瓦片连续读）。

**MECHANISM**
`Axpy(dst, src, scalar)` 语义 `dst[i] = src[i]*scalar + dst[i]`（标量融合乘加，
`kernel_operator_vec_ternary_scalar_intf_impl`）。先 `Mul` 得 `y·g`，再 `Axpy` 一步完成
`·s + b`。bias 作为累加底，多行共享 bias 瓦片时把 bias 复制进输出暂存（或逐 tile
Load bias 进输出暂存）后累加，参数本身不被破坏。

**BOTTLENECK**
第二遍三条依赖 V 趟、value 瓦片三次写回。本形态砍掉一趟 V 与一次 value 写回；
标量乘从「单独一趟」变成加法指令的操作数组合。

**WHY_NOT_DUPLICATE**

| 对照 | 差异 |
|---|---|
| EPILOGUE-FUSE-X V002/V003 `MulAddDst` 系 | 他们用 `MulAddDst`（向量·向量 + dst 累加）融合的是 **gamma 乘 + bias 加**，保留独立 `Muls`，表达式 `(y·s)·g + b`。本假设融合的是 **invRms 标量乘 + bias 加**，表达式 `(y·g)·s + b`，指令是 `Axpy`（标量乘加），不使用 MulAddDst |
| EPILOGUE-FUSE-X SCALE-FOLD | 不建 s·g 刻度瓦片 |
| VECTOR-MATH-X | invRms 求值不动 |
| STORE-EPILOGUE-X | store 不动 |

近边界如实记录：两者同属「仿射压缩成更少 V 趟」大家族，表达式结合序与融合指令都不同；
若 Main 判 CONCEPT_COLLISION，则本条让位，H1/H3/H4 不受影响。

**MINIMAL_OFAT_DIFF**
逐 tile 第二遍：`Muls;Mul;Add` 三步 → `Mul;Axpy` 两步（输出暂存预置 bias），
其余不动。

**EXPECTED_SHAPES**
中窄与宽路径的 FP32/BF16 第二遍（2×8192、4×8192、1×8192、1×32768）；
多行共享参数的宽路径额外获得 gamma 连续读的发射分组收益。

**PRECISION_RISK**
中。`(y·s)·g` 与 `(y·g)·s` 是不同的乘法结合序，逐元素可能差 1 ulp；
`b + t·s` 与 `(t·s) + b` 加法可交换，舍入相同。必须用全形状正确性对 max_abs 逐项对照父版。

**ASCEND_FEASIBILITY**
`Axpy` 在本 toolkit 已被 EPILOGUE-FUSE-X 核对存在。需先确认 `Axpy` 对
`Tuple<float,float>` 与 `float` 标量的 calCount 形态，以及 dst 与 src 可分离
（DIV 探针曾见 dst==src 的静默踩踏类风险，Axpy 需同样的分离槽位纪律）。

**UB/CORE/DMA_IMPACT**
输出暂存多一次 bias 预置（Load 进输出暂存可与既有 Load 合并）；UB 不增；DMA 不变。

**SYNC_IMPACT**
每 tile 少一道 PIPE_V 屏障（三趟两屏障 → 两趟一屏障）。

**EXPECTED_LOCAL_PROBES**
2×8192_fp32、1×32768_fp32 的 P/C；正确性必须26 形状全过并与父版 max_abs 对照。

**成熟度：NEEDS_MORE_EVIDENCE**（Axpy 形态先做可行性探针；与 EPILOGUE-FUSE 近边界待 Main 裁定）

---

### EA-H3 — NORM-AS-DIV：norm 写成 y / rms，去掉标量倒数

**SINGLE_HYPOTHESIS**
norm 的表达形式从 `y * invRms`（invRms = 1/rms 经标量除法求得）改为 `y / rms`
（rms 即 Sqrt 结果，保留在 UB 槽位），第二遍用 `Div` 应用，整条 `1.0f /` 标量除法消失。

**MECHANISM**

```text
今日：… Sqrt(slot) → SyncVToS → invRms = 1.0f/GetValue(0) → SyncSToV → 每 tile Muls(y, invRms)
改后：… Sqrt(rmsSlot) → 每 tile Div(value, value, rmsBroadcast, valid)   // 或先 Duplicate 广播 rms
```

标量尾巴少一次 GetValue+除法+一次 S/V 往返；norm 的数学形式从乘倒数变成除以 rms
（IEEE 意义上更准：一次舍入）。

**BOTTLENECK**
每行标量倒数 + 往返；以及 `y·(1/rms)` 的两次舍入。

**WHY_NOT_DUPLICATE**

| 对照 | 差异 |
|---|---|
| VECTOR-MATH-X VM-H1/H2/SEQ-FUSE-2 | 他们保留/优化**倒数的求值**（Rsqrt、向量分母、Div(ones,denom) 求 invRms）；本假设**不求倒数**，把 norm 的表达式整体改写成除法，应用侧换 `Div` |
| VECTOR-MATH-X VM-H3 | 他们的应用是广播 `Mul`；本假设应用是 `Div` |
| EPILOGUE-FUSE-X | 不动融合形态；本假设改 norm 的等价代数形式 |

**MINIMAL_OFAT_DIFF**
标量尾巴删 `1.0f/GetValue` 一段；第二遍 `Muls` → `Div`（rms 广播槽位，与 DIV 探针同样的
32B 对齐、dst 与除数分离纪律）。gamma/bias 两步不动。

**EXPECTED_SHAPES**
全部形状（norm 在每条路径上）；短行多核形状标量尾巴占比高，收益可能更大。

**PRECISION_RISK**
低偏正：`y/rms` 比 `y*(1/rms)` 少一次舍入，max_abs 通常不劣于父版；但若参考实现
按乘倒数生成 golden，需按容差判定而非逐位。

**ASCEND_FEASIBILITY**
DIV_FEASIBILITY_PROBE 已证 `Div` 在 dav-c220 上是精确 IEEE 除法，安全形态为
三分量分离、32B 对齐、count=8 或整倍数。风险在**吞吐**：逐元素 `Div` 通常比 `Muls`
慢一个量级，宽行大 tile 上可能是负收益——这正是「Div/Rsqrt/Mul 吞吐最佳组合」要测的。
应用形态若用 `Div(value, ones, rmsSlot)` 变体反而绕回求倒数，禁止；必须是真除法 `y/rms`。

**UB/CORE/DMA_IMPACT**
rms 保留一个标量/一元素槽；不增缓冲；DMA 不变。

**SYNC_IMPACT**
标量尾巴少一次 S/V 往返；应用侧屏障数不变。

**EXPECTED_LOCAL_PROBES**
2×8192_fp32（中等宽度）与 1×256_fp32（短行）对照；若宽行显著变慢则判定 Div 吞吐不划算。

**成熟度：NEEDS_MORE_EVIDENCE**（吞吐风险高，适合小额探针后决定）

---

### EA-H4 — CONST-FOLD-MEANSQ：invRms 标量表达式常数折叠

**SINGLE_HYPOTHESIS**
`meanSquare = squareSum * invRowWidth + epsilon` 与 `invRms = 1.0f / sqrt(…)` 的标量
表达式按代数等价式改写，把 `invRowWidth`、`epsilon` 折成宿主常数：

```text
今日：meanSquare = squareSum * invD + ε            （标量 mul + add）
      invRms    = 1.0f / sqrt(meanSquare)          （标量 div）
改后：shifted    = squareSum + εD                  （标量 add，εD = ε*D 宿主常数）
      invRms    = sqrtD * (1.0f / sqrt(shifted))   （sqrtD = sqrt(D) 宿主常数）
      或 invRms = sqrtD / sqrt(shifted)
```

因为 `1/sqrt(s/D + ε) = sqrt(D)/sqrt(s + εD)`。每行标量 mul+add+div → add+div（或 add+mul）。

**MECHANISM**
宿主侧预计算 `epsilonTimesRowWidth = epsilon * rowWidth` 与 `sqrtRowWidth = sqrt(rowWidth)`
（均为每 kernel 调用常数），设备侧标量表达式缩短一趟乘法。

**BOTTLENECK**
标量尾巴每行一次 mul+add+div；多行短 kernel 时标量占用可观（VECTOR-MATH 记录的 5–12µs 短核）。

**WHY_NOT_DUPLICATE**

| 对照 | 差异 |
|---|---|
| VECTOR-MATH-X | 他们改标量尾巴的**指令域与序列**（搬到 V、Rsqrt、批量、广播）；本假设标量域内改**代数表达式**，Duplicate+Sqrt+GetValue+Sync 骨架一字不动 |
| EPILOGUE-FUSE / STORE | 不相交 |

近边界如实记录：epsilon 与 mean 字眼在 VECTOR-MATH 的 SCOPE 里；本假设只做常数折叠
的表达式改写，不动他们的求值架构。若 Main 判重，本条让位。

**MINIMAL_OFAT_DIFF**
每处 invRms 尾巴两行标量式改写 + Init/host 侧两个常数；不碰向量指令。

**EXPECTED_SHAPES**
行数多、单行短的形状（rows 大、D 小到中）收益最大；单行宽形状收益趋近于零。

**PRECISION_RISK**
中。`s·invD + ε` 与 `(s + εD)/D` 在 sqrt 参数内部结合序不同，sqrt 与倒数的输入值可能差
1 ulp，进而 invRms 差 1 ulp，输出 max_abs 需按容差验证。`fma(squareSum, invD, ε)`
形态（一次舍入）可作为替代写法一并对比。

**ASCEND_FEASIBILITY**
纯标量 C++ 算式，宿主常数随 kernel 参数传入，无 API 风险。

**UB/CORE/DMA_IMPACT**
零。SYNC_IMPACT 零。

**EXPECTED_LOCAL_PROBES**
8×256_fp32、32×256_fp32 等多行短形状的 P/C；2×8192 作零差控制。

**成熟度：NEEDS_MORE_EVIDENCE**（收益小、精度需对照；适合与 H1 解耦的后续轮）

---

## 3. SUMMARY

| ID | 单一变量 | 精度 | 预期 | 去重风险 | 成熟度 |
|---|---|---|---|---|---|
| **EA-H1 NORM-HOIST** | invRms 应用时机与粒度（逐 tile→整行一次，移出参数等待） | 零（按位一致） | 宽 D 指令数↓、屏障↓ | 低 | **READY，首选** |
| EA-H2 GAMMA-FIRST-AXPY | 乘法结合序 + 标量乘折进 bias 加（Axpy） | 中（1 ulp 级） | V 趟 3→2 | 中（近 EPILOGUE-FUSE 融合族） | NEEDS_MORE_EVIDENCE |
| EA-H3 NORM-AS-DIV | norm 等价形式 y/rms，删标量倒数 | 低偏正 | 尾巴↓，Div 吞吐待证 | 低 | NEEDS_MORE_EVIDENCE |
| EA-H4 CONST-FOLD-MEANSQ | invRms 标量表达式常数折叠 | 中（1 ulp 级） | 标量趟↓（短行） | 低偏中 | NEEDS_MORE_EVIDENCE |

## 4. RECOMMENDATION

**V001 = EA-H1 NORM-HOIST。**

理由：
1. 四条中唯一按位一致、零浮点结合序风险——Champion 种子上第一个 Revision 选最低精度风险者。
2. 变更域与 STORE-EPILOGUE-X（回写）、VECTOR-MATH-X（求值）、EPILOGUE-FUSE-X（融合形态/整行仿射）三面都不交叉。
3. 直接作用于 case 14 一类宽 D 缺口路径（rowWidth>8192 的 FP32 全缓存第二遍）。
4. 最小差异一处函数一段循环，SINGLE_CHANGE_AUDIT 可逐字核对。

## 5. REQUEST_MAIN_APPROVAL

REQUEST_MAIN_APPROVAL：以 EA-H1 NORM-HOIST 作为 EPILOGUE-ARITH-CHAMPION-X V001 的单一假设，
DIRECT_PARENT = R31B-V011（SHA `a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3`）。
EA-H2 / EA-H3 / EA-H4 留在积压，按上表成熟度推进。
