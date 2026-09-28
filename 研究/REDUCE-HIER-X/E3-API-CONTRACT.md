# E3 — ReduceSum / BlockReduceSum / WholeReduceSum API 契约核对（只读）

ROUTE=REDUCE-HIER-X
DATE=2026-09-29
PURPOSE=N3 立项前置：核对内建向量输出归约 primitive 是否存在、契约如何、相对当前
`ReduceSum` 是否有优势。
METHOD=只读 CANN 8.5.0.alpha002 工具链头文件（server3
`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/tikcpp/tikcfw/`）。
无设备窗口、无编译、无改码。

---

## 1. API 一览（interface/kernel_operator_vec_reduce_intf.h）

| API | 签名要点 | 语义（头文件注释） | 输出形态 | work buffer |
|---|---|---|---|---|
| `BlockReduceSum(dst,src,repeatTime,mask,dstRepStride,srcBlkStride,srcRepStride)` | Level-0 | "Sum all elements in each block" | 向量：每 block 一个和 | **无** |
| `WholeReduceSum(dst,src,mask,repeatTime,dstRepStride,srcBlkStride,srcRepStride)` | Level-0 | "Sum of all effective elements in each repeat" | 向量：每 repeat 一个和 | **无** |
| `PairReduceSum(...)` | Level-0 | 相邻对求和 | 向量 | 无 |
| `RepeatReduceSum(...)` | Level-0 | 每 repeat 归约 | 向量 | 无 |
| **`ReduceSum(dst,src,sharedTmpBuffer,count)`** | **Level-2（现用）** | "sum all input elements" | 1 元（dst[0]） | **需要 sharedTmpBuffer** |

dtype 约束：src/dst 同为 half 或 float（FP32 可用）。`BlockReduceSum` 的 repeatTime
范围限制 `[0, 255]`。`ReduceSum` Level-2 的 count 范围 `[1, TOTAL_UB_SIZE/sizeof(T)]`
（FP32 上限约 49152），**4096 元 tile 完全在范围内**。

## 2. dav-2201 实现层证据（impl/dav_c220/kernel_operator_vec_reduce_impl.h）

### 2.1 当前 `ReduceSum` 的真实开销（N3 的瓶颈假设 → 已证实）

`__NPU_ARCH__ == 2201` 分支的 `ReduceSumImpl(dst, src, count)`（L146–168）逐指令：

```cpp
set_mask_count(); set_vector_mask(0, count);
vcadd(dst, src, 1, ...);                    // 硬件整段求和 → 累加到 acc 寄存器
SetFlag<V_S>; WaitFlag<V_S>;                // (1) V→S 全管线排空
int64_t accVal = get_acc_val();             // (2) 标量侧读 acc 寄存器
*(dst) = *(T*)(&accVal);                    // 结果写回 UB
SetFlag<S_V>; WaitFlag<S_V>;                // (3) S→V
SetFlag<S_MTE3>; WaitFlag<S_MTE3>;          // (4) S→MTE3（把归约与 store 管线耦合）
```

**每次 `ReduceSum` 调用 = 1 条 vcadd + 3 次硬同步 + 1 次标量寄存器读。**
这正是 V001–V004 研究记录里"ReduceSum performs its internal V/S handoff"的实现层
实锤（不是推测）。额外发现：`S_MTE3` 同步把归约与 store 管线耦合，multi-tile 路径上
每 tile 都付一次。

### 2.2 Level-0 原语 = 纯 V 指令

| API | 硬件指令 | 同步 | 标量 |
|---|---|---|---|
| `BlockReduceSum` | `vcgadd` | 无 | 无 |
| `WholeReduceSum` | `vcadd(..., mode=0)` | 无 | 无 |
| `RepeatReduceSum` | `vcadd(..., mode=0)` | 无 | 无 |
| `PairReduceSum` | `vcpadd` | 无 | 无 |
| `ReduceSum` (Level-2) | `vcadd(..., mode=1)` | V/S + S/V + S/MTE3 | `get_acc_val` |

`vcadd` 的末参数：mode=1 → 结果进硬件 acc（须标量读回）；mode=0 → 结果直接写
dst（每 repeat 一值，纯向量）。Level-0 `WholeReduceSum` 用 mode=0。

### 2.3 span 适用性

`ReduceSumImpl` 用 mask-count 模式 + repeat=1 处理任意 count（含 4096 与非 2 幂尾块）。
`WholeReduceSum` 同样支持 mask-count + repeatTime，语义为"每 repeat 一个和"——
repeatTime=1、mask=count 时即"整段一个和写 dst[0]"，span 覆盖与当前 `ReduceSum`
一致。`BlockReduceSum`（vcgadd）产生每 block-group 一个部分和，需二级收尾，不如
`WholeReduceSum` 直接。

## 3. E3 结论

**PASS — N3 可行且有实现层证据支撑的预期优势。**

| 契约问题 | 答案 |
|---|---|
| 是否存在向量输出的内建归约？ | 是：`WholeReduceSum`（推荐）/ `BlockReduceSum` |
| 输出是否留在 UB 向量侧？ | 是，mode=0 直接写 dst，无标量回路 |
| 当前 `ReduceSum` 是否有标量 handoff？ | **是，已实锤**：3 硬同步 + `get_acc_val` + S_MTE3 耦合 |
| 4096 元 tile 是否适用？ | 是：mask-count 模式 count 上限 ≈49152（FP32） |
| 与 N1（V004 手写 Add 分箱）区别？ | N1 手工分解出数百条 Add（指令数成本，已证伪）；N3 仍是**一条 vcadd**，只去掉 vcadd 之后的同步/标量链。不重蹈 V004 |

### N3 预期收益模型（multi-tile，tileCount=T）

每行归约侧硬同步次数：`(T+1)×(V/S + S/V + S/MTE3)` → `1×(V/S)`（仅保留既有的
行尾 GetValue 一次）。T=8（1x32768）时从 27 次硬同步降到 1 次；T=2 时从 9 到 1。
同时解除归约与 MTE3 store 管线的耦合。指令数不变（仍是 T+1 条 vcadd）。

### 实现注意（留给 V005，不在本轮做）

1. 选 `WholeReduceSum` 的 mask-count 重载，repeatTime=1、dstRepStride/srcBlkStride/
   srcRepStride 按 API 要求给 1；count 与现行 `ReduceSum` 调用点完全一致（含尾块
   任意 count）。
2. 行尾 GetValue 链不动（sync-removal 不在本路线）。
3. 若 mask-count 重载对非 2 幂 count 行为与 `ReduceSumImpl` 不一致，正确性矩阵会
   在 1x100/1x65 捕获；届时仅作 correctness fix，不加第二性能变量。
4. OFAT：只换归约 primitive（同站点、同 span、同两级拓扑），S1/S2/S3 先落。

## 4. 证据来源

- `.../tikcfw/interface/kernel_operator_vec_reduce_intf.h`（L25–195 声明与注释）
- `.../tikcfw/impl/kernel_operator_vec_reduce_intf_impl.h`（L974–1060 Level-2 分支；
  L669–699 Level-1 分支）
- `.../tikcfw/impl/dav_c220/kernel_operator_vec_reduce_impl.h`（L20–60 原语映射；
  L146–168 `ReduceSumImpl`；L254–258 `WholeReduceSumImpl`）
