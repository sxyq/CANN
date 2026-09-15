# 提交核对单（Submission Checklist）

> 使用方式：每项完成打 [x] 并写日期/备注。真机验证前置条件未满足的条目保持 [ ] 并记录 blocker。

## 1. 源码文件清单（../源码/ 与 ../提交/）

| 文件 | 状态 | 备注 |
| --- | --- | --- |
| ../源码/add_rms_norm_bias.json | [x] 已提供 | msopgen 原型定义（dtype 列表含 fp16/bf16/fp32；attr epsilon） |
| ../源码/op_kernel/add_rms_norm_bias.cpp | [x] 已提供 | Ascend C 核函数（Kernel 模板） |
| ../源码/op_host/add_rms_norm_bias_tiling.h | [x] 已提供 | Tiling 数据结构 |
| ../源码/op_host/add_rms_norm_bias.cpp | [x] 已提供 | 算子注册/InferShape/Tiling |
| ../源码/CMakeLists.txt / CMakePresets.json / build.sh | [x] 已提供 | 源码清单与真机构建入口；CANN 工程需由 msopgen 生成 |
| source-build.md | [x] 已提供 | 生成与编译步骤 |
| ../提交/混合方案/H001-正确性优先/V001/kernel.asc | [x] 历史版本 | 15 点 Compile Error（pipe_ 宏冲突阻断） |
| ../提交/混合方案/H001-正确性优先/V002/kernel.asc | [x] 修复候选 | 已处理尾块参数、V_S/S_V 同步及 64 位偏移；服务器 case0 已通过 |
| ../提交/混合方案/H001-正确性优先/V003/kernel.asc | [ ] 本地正确性候选 | 两遍扫描、FP32 全中间、`CAST_RINT`、尾块和行组分配；真机待补 |
| 提交包目录 ../提交/ | [ ] 未确认 | CANNJudge 上传字段和封装方式仍需登录提交页确认 |

## 2. 编译入口（真机）

```bash
# 1) 设置 CANN 环境
source /usr/local/Ascend/ascend-toolkit/set_env.sh        # 按实际安装路径
export ASCEND_CANN_PACKAGE_PATH=/usr/local/Ascend/ascend-toolkit/latest
export DDK_PATH=$ASCEND_CANN_PACKAGE_PATH
export NPU_HOST_LIB=$ASCEND_CANN_PACKAGE_PATH/{arch}-linux/devlib

# 2) 工程骨架：用 msopgen 按判题 SoC 生成
msopgen gen -i 源码/add_rms_norm_bias.json -c ai_core-<soc> -lang cpp -out <target>   # <soc> 待确认
#    然后把 源码/op_host、源码/op_kernel 覆盖到生成工程对应目录

# 3) 编译生成工程
ASCENDC_GENERATED_PROJECT_DIR=<target> ./源码/build.sh
# 产物位置和名称以 msopgen 生成工程为准
```

- 编译必须通过的动作：`check_supported`/`clang++`（Ascend C kernel 侧）、host 侧算子库链接。
- SoC 待确认：判题模板默认 `dav-2201`（cannbot-skills 映射 = Ascend 910B1~B4/910B2C/910_93，即 A2 系形态，B 级推断）。`dav-1001`=老 910/910A 训练系、`dav-2002`=310P 推理、`dav-3002`=200I/500 A2、`dav-3510`=950 系。**判题机具体型号未知，先按模板默认 dav-2201 编译，提交前与平台核对**（DataCopyPad 支持矩阵以判题机为准）。

## 3. 输入输出接口（与题面对齐）

- 输入：x[(...D)] fp16/bf16/fp32、residual[(...D)] 同型、gamma[(D)] 同型、bias[(D)] 同型；attr epsilon(float, 默认1e-5)。
- 输出：output[(...D)] 同型。
- 内部实现：Kernel 收 GM_ADDR x/r/g/b/output/workspace/tiling；epsilon、outer、D、dtype 通过 tiling 下传。
- 严禁：Host 计算输出、写死常量表。

## 4. 本地验证项目（真机阶段）

- [x] CPU 语义模拟（第一轮）：117 组中 115 组通过，2 组 BF16 D=32768 边界差异；见 `../调研/归档/调研1/validation-host-notes.md`。
- [x] CPU 同链验证（第二轮，Agent06，2026-09-11）：fp16/fp32 必过；bf16 需分块 ≤4096 + 正确舍入 sqrt（脚本 /tmp/agent06_numerics/，不在仓库）。
- [x] **【源码核对】`DataCopyPadExtParams<T>` 具名赋值**：V002 已显式指定 `.isPad`、`.paddingValue`、`.leftPadding` 和 `.rightPadding`；仍需真机确认 API 与尾块语义。
- [x] **【源码核对】64 位全局行偏移**：V002 的行基址与搬运偏移使用 `uint64_t`；仍需真机覆盖大尺寸输入。
- [ ] **【并发防线核对】多核 32B Cache Line 边界对齐**：$D$ 非 32B 对齐时，核间行分配必须满足 $k \times D \times \text{sizeof}(T) \equiv 0 \pmod{32}$，防止跨核 DMA 写回踩踏撕裂。
- [x] **【源码核对】标准 ReduceSum 与标量同步**：V002 使用 `ReduceSum` + `SetFlag/WaitFlag<HardEvent::V_S>` + `dst.GetValue(0)`，并补充 `S_V` 同步；仍需真机编译确认。
- [ ] 真机第一验证批（按 `../调研/归档/调研1/research-report.md` §14 顺序）：① SoC/DataCopyPad 支持；② 尾块搬出语义（D=67/129/1000 逐字节核对相邻行）；③ ReduceSum count 上限（2048/4096/8192 扫描）；④ 向量 Sqrt vs 标量 sqrtf vs rsqrt 的 bf16 精度；⑤ CAST_RINT 舍入。
- [ ] 用 msopst 或自建用例跑通 15 个测试点（15 点=官方明文，全部通过才计分，A 级确认）。
- [ ] 先在平台直调模板中跑通默认 FP16 `[1,64]` 用例。
- [ ] 数值对照基准：在 NPU 上按题面实际参考实现确认 FP32、FP16、BF16 的误差口径。
- 覆盖矩阵（记录到对应 V00N 目录）：

| 维度 | 取值 |
| --- | --- |
| dtype | fp32 / fp16 / bf16 |
| rank | 2D / 3D / 4D |
| D | 64, 67(非32倍数), 96, 129, 192, 576, 1000, 1024, 4096, 32768 |
| outer | 1, 8, 512, 8192（与 D 组合控制总量 ≤ 判题上限） |
| 特殊值 | 全 0、NaN、±Inf、|y|≥256（fp16 溢出）、混合大/小值（防溢出验证） |

## 5. 15 个测试点记录位置

- `../提交/混合方案/H001-正确性优先/V001/结果.md`：逐点记录 dtype/shape/误差(相对+绝对)/耗时/通过；后续版本使用所属方案下对应 `V00N/结果.md`。
- 模板（每点一行）：

```markdown
| 测试点 | shape | dtype | 相对误差 | 绝对误差 | 耗时(us) | 通过 |
| --- | --- | --- | --- | --- | --- | --- |
| T01 | [2,4] fp32 | fp32 | ... | ... | ... | ✅/❌ |
```

## 6. 上传前内容核对

- [ ] 源码内无凭据/Token/Cookie/授权头（全局 grep：`password|token|secret|api[_-]?key|cookie|authorization`，大小写不敏感）。
- [ ] 仅含平台要求的必要文件：本题为直调模式，提交物 = 含 `run_kernel` 的 `kernel.asc`（`code_template=npu_kernel_dev`，2026-09-11 API 确认）；旧 skill 的 `tiling_h`/`tiling_key_h`/`host_cpp`/`kernel_cpp` 四字段属旧 msopgen 工程格式、不适用；实际表单待登录提交页最终确认。
- [ ] 无写死测试输入/输出；无 Host 代算代码。
- [ ] **无聚合初始化 `DataCopyPadExtParams`**（必须显式具名赋值）。
- [ ] **无 32 位全局内存行偏移寻址**（必须 64 位整型）。
- [ ] **多核切分具备 32B Cache Line 跨核并发防踩踏约束**。
- [ ] 入口限定符与模板逐字对齐 `__global__ __vector__`。
- [ ] 无本机绝对路径/临时产物（不含 build_out/.git）。

### 6.1 调研2（2026-09-11）裁决后的新增核对项

- [ ] **归一化改为「先除」**：不使用 `Muls(y, 1/rms)`，改用 `Divs(y, rms)`（或等价先除写法），使舍入路径与官方 golden 一致。依据：调研2 独立复现证明 `Muls` 路径会在 bf16 上产生 1~3 ulp 量化分界翻转，而改「先除」后与 golden **逐位一致**（零成本改进）。
- [ ] **`TPipe` 移出 kernel 类**：官方最佳实践明确「避免 `TPipe` 在对象内创建和初始化」；当前 V002 把 `TPipe tpipe;` 作为类成员，应移到核函数入口、类内保存指针，并验证是否影响编译。
- [ ] **入口限定符与模板逐字对齐** `__global__ __vector__`（已确认 `__vector__` 与 `__aicore__` 均合法，但以模板为准）。
- [ ] **不使用旧枚举名 `CAST_RND`**（9.0.0 应为 `CAST_ROUND` / `CAST_RINT`）；确认代码只出现 `CAST_NONE` / `CAST_RINT`。
- [ ] **提交内容四项核对**（防 V002 式上传截断）：① 首行为 `#include <cmath>`；② 末行为 `}`；③ `wc -l` ≈ 405（与本地文件一致）；④ 含 `extern "C" void run_kernel`。**必须全选替换，不得只贴片段。**
- [ ] **判题 SoC 未确认风险已登记**：题面 API `soc_version: null`，模板默认 `dav-2201` 不是平台约束；若判题机非 A2/A3 系（如 950/A5 需 `dav-3510`）需改用对应架构与降级路线。
- [ ] **真机第一验证批（修订版，按 P0 优先）**：① 用完整文件做一次**干净编译**（V002 从未被真正编译过）；② UB→GM 非对齐搬出的 dummy 是否真丢弃（D=67/129/1000 逐字节核对相邻行）；③ 判题精度判定口径（是否允许少量元素失配）；④ `CAST_RINT` 是否等于 RNE；⑤ `workLocal` 在 fp16/bf16 D=4096 单块下的实际需求。

### 6.2 调研2（2026-09-12 重做）新增核对项

> 对应 `../调研/归档/调研2/research-report.md` 与 `../调研/归档/调研2/方案矩阵.md`。「调研2」在 2026-09-12 重做了一轮十代理调研，以下为本轮新增/升级的核对项。

- [ ] **fp32 自测阈值改为 1e-4**（题面 §五：fp32 相对/绝对 `< 1e-4`；本地 `verify_result.py` 只有 1e-3，会漏判 fp32）。
- [ ] **优先 `Divs` + `Sqrt`，暂不采用 `Muls(1/rms)` 与 `Rsqrt`**：主代理独立复核 36 组配置，`Divs` 与 golden **逐位一致（最大绝对差恒为 0）**；`Muls` 在「大 outer 小 D」（如 `(8192,64)`）下 bf16 最大绝对差达 **1.562e-02**（约 15.6 倍阈值）并出现 1–3 个真实失配元素，fp16 达 **1.953e-03**。详见 `../调研/归档/调研6/validation-host-notes.md` §3.2。
- [ ] **epsilon 必须在 `mean(y²)` 之后、开方之前**（外置实测 bf16 0.0549% 失配）。
- [ ] **bf16 全程 Cast→fp32→Cast**：A2 上 `Add`/`Mul`/`Muls`/`Div`/`ReduceSum` **均不支持 bf16**（9.0.0-beta.2 明文）。
- [ ] **体量控制**：收敛为参数化通用路径，**不按 shape 档位写专用分支**（当前 V002 2955 行 / ~20 个 `Process*` 变体，命中官方记载的 `out of jump/jumpc imm range` 编译风险）。
- [ ] **裸 `DataCopy` 禁止写非 32B 对齐尾块**（会写 -1 污染邻居）；尾块一律 `DataCopyPad` + 真实有效 blockLen。
- [ ] **`ReduceSum` 的 `srcInnerPad=true`**，且平方和必须在 FP32 域（fp16 在 `|y|≥256` 时 `y*y=inf`）。
- [ ] **提交通道五项核对**（防 V002 式截断）：行数、`md5`、首行 `#include`、末行 `}`、含 `extern "C" void run_kernel`。
- [ ] **提交策略**：每天 50 次且**取最后一次成绩** → 先提交「仅 FP16 单路径、无分支」的最小可编译版验证通道，再增量加 dtype / 尾块 / 多核；**不把未验证的大改直接提交**。
- [ ] **真机第一动作**：用**未修改的模板原样编译并运行一次**，以此区分「平台/模板/打包问题」与「我们手写 kernel 的问题」（V001/V002 两次 CE 均非算法问题）。

## 7. 提交前人工确认项

- [ ] 15 点全部通过且误差达标（fp32<1e-4；f16/bf16<1e-3，双误差；判题 isclose rtol/atol=0.001 + 失配 ≤0.1% 语义待真机确认）。
- [ ] 真机第一验证批闭环：SoC 型号、DataCopyPad 尾块搬出语义、ReduceSum count 上限、sqrt 精度、CAST_RINT 舍入（`../调研/归档/调研1/research-report.md` §14）。
- [ ] 性能记录完成（题面要求优化；得分公式 `100/(1+log₁.₅(t/T))`，性能是唯一计分维度）。
- [ ] 用户已阅毕汇报且**明确同意上传/提交**；
- [ ] 当日提交次数余量 > 0（每日 50 次额度无公开页面依据，按用户提供信息保守管理）。

## 8. 提交后结果记录方式

- 记录到 `../提交/混合方案/H001-正确性优先/V001/历史.md`：提交时间、包版本 `V001`、子账号、判题结果、错误信息截图/文本和后续处理；后续版本记录到所属方案下对应 `V00N/历史.md`。
- 若失败：先看判题返回的用例/错误类型（编译错误/精度/超时/崩溃），再定向修复；不盲目重复提交浪费额度。

## 9. 每日 50 次额度记录

| 日期 | 已用 | 剩余 | 备注 |
| --- | --- | --- | --- |
| 2026-09-10 | 0 | 50 | 尚未首次提交 |
| （真机启动后逐日维护） | | | |

> 提醒：额度每天重置；提交命中 50 次上限后当天停止，次日再试。
