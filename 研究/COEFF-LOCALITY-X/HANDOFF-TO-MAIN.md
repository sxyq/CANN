# HANDOFF-TO-MAIN — COEFF-LOCALITY-X V003 完成

ROUTE=COEFF-LOCALITY-X
WORKTREE=/Users/sunyiyang/Desktop/Project/cann-m2-coeff
BRANCH=m2/coeff-locality
FROM=Route Agent（LANE M2-3）
TO=Main-2
DATE=2026-09-29
PHASE=V003 执行完成 — 本地结论已下，STOP

---

## 1. 需求与状态

Main-2 批准 NH-2（wide FP32 pass 2 split-phase 参数 MTE2 发射）作为 V003 单一假设。
要求：compile → correctness → same-binary → 交错 P/C，主靶 large-D（1x32768 / 1x16384 / 8x32768）。

**状态：V003 流程全部完成，本地结论 LOCAL_REJECTED。**

---

## 2. 本轮实际完成

1. Revision 声明（`本地实验/COEFF-LOCALITY-X/V003/DECLARATION.md`）。
2. Kernel 修改：`ProcessWideFp32FullCacheRows` pass 2 split-phase 发射。
   - tile 0 参数在循环前发射；
   - 之后每 tile：最后一个 Mul 消费 gamma slot 后发下一 tile gamma Load，最后一个 Add 消费 bias slot 后发下一 tile bias Load；
   - 两 slot 轮转 = 现有 `xBuf_`/`residualBuf_`，**零新 staging**，tileElems 钉 4096，MTE2 次数不变（每 tile 2 次）；
   - V_MTE2 SetFlag/WaitFlag 保证 V 读完再覆盖（与 lowp prefetch 同 idiom）；既有 SyncMTE2ToV 与 MTE3 store-drain 规则保留。
3. server3 编译链接：BUILD PASS（CMAKE/OBJ/LINK/PROBE RC=0）。
4. NPU 正确性：**candidate 18/18 PASS**（parent 在 1x16384/1x32768 FP32 上的 shared-golden 失败与 V001 相同，已记录）。
5. same-binary + 6 组交错 P/C（d4，warmup=45，samples=41）。
6. 本地结论：**LOCAL_REJECTED**。

## 3. 修改或操作对象

| 对象 | 路径 |
|---|---|
| V003 源码 | `本地实验/COEFF-LOCALITY-X/V003/submission.asc`（SHA 4be7c228…） |
| 声明 / diff / 元数据 / 结论 | `本地实验/COEFF-LOCALITY-X/V003/{DECLARATION,diff.patch,source-meta.json,local-result.json,SUMMARY.md}` |
| 构建/正确性/测时证据 | `本地实验/COEFF-LOCALITY-X/V003/support/`（含 results-timing-v003-20260929/ 全部 raw） |
| server3 工作区 | `/home/data4t2/lelinfeng/phase4-workspaces/COEFF-LOCALITY-X/`（submission.asc 已同步） |

## 4. 验证结果

| 阶段 | 结果 |
|---|---|
| 源码身份 | LOCAL_SHA == REMOTE_SHA == 4be7c228… |
| 编译/链接 | PASS（exe SHA 44aa8ef1…） |
| 正确性 | candidate 18/18 PASS |
| same-binary | 1x32768、8x32768 双侧 PASS；1x16384 C drift FAIL；1x4096/1x8192 P FAIL |
| 配对 delta（干净样本中位） | 1x32768 **+2.37%** favP=4/2；1x16384 +2.78% favP=5/0；8x32768 +2.06% favP=3/1 |
| 控制组 1x4096（tileCount=1，机制不触发） | +1.47% favP=3/1 → **本场测量偏置底** |

**判定：LOCAL_REJECTED。** 三个 large-D 形状干净配对全部偏 parent；超出控制组偏置仅 ~0.6–1.3%。

**证伪结论（DECLARATION 判据）**：tileElems 恒 4096 下 1x32768 |Δ|≈噪声 →
**param MTE2 时序不是 large-D 瓶颈。** V001 的混杂就此澄清：+19.2% 主要是 tile-shrink 副作用，时序机制本身至多中性。
可能的成本来源：每 tile 2 个 V_MTE2 SetFlag/WaitFlag（tileCount=8 → 14 次额外 flag 操作）抵消了预取收益。

---

## 5. 剩余工作与风险

- **下一步假设**：按 Main-2 排序，NH-1（=H3 精化，generic 单行多 tile 整行预加载）是本轴仅存的正收益候选。需 Main-2 明确批准后才建 V004，父版回 FROZEN_R31B_V011（本路线 LOCAL_BEST=NONE）。
- **共享总账**：本轮未改 `技术路线/`、`调度/`（含 lease 表）——按约束不碰。lease `R2-COEFF-V003-TIMING`（d4，2026-09-29）已在 local-result.json 记录，请 Main 同步到 `调度/服务器设备使用.tsv`。
- **风险**：无未解决正确性/构建问题。残余不确定性只在测量噪声（1x16384 C drift、1x8192 P drift），已按协议标注为 blocked/方向性，未用于强结论。

**本轮结论已下，STOP，等待 Main-2 对 NH-1/V004 的决定。**
