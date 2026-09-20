# AddRmsNormBias 源码编译说明

> 2026-09-11 更新：确认平台模板为 **CANN ≥8.3（推荐 9.0.0）的 ASC 语言直调工程**（`find_package(ASC)` + `--npu-arch`），非老式 msopgen 算子工程；8.2 及以下不支持 `--npu-arch`。详情见 `../调研/归档/调研1/research-report.md` §8 与 `../调研/归档/调研1/Agent08-linux-env.md`。

## A. 平台直调模板（当前候选为 V002）

本题提供的 `addrmsnormbias_problem_1742_template` 是一个直接调用 `run_kernel` 的本地模板。平台当前唯一明确的可编辑文件是模板根目录下的 `kernel.asc`。
- `提交/混合方案/H001-正确性优先/V001/`：历史直调版本（15 点 CE，因 `pipe_` 宏命名冲突未进入测试）；
- `提交/混合方案/H001-正确性优先/V002/`：**修复候选版本**（已处理 `DataCopyPadExtParams<T>` 尾块参数、标准归约同步和 64 位偏移，服务器已完成 CANN 编译和模板 case0 运行）；
- `提交/混合方案/H001-正确性优先/V003/`：**本地正确性候选**（两遍扫描、FP32 全中间、尾块与行组分配；真机编译和测量待补）。

在有 CANN 9.0.0 和昇腾 NPU 的机器上，按下面流程运行：

```bash
cp /path/to/cann/提交/混合方案/H001-正确性优先/V002/kernel.asc /path/to/addrmsnormbias_problem_1742_template/kernel.asc
cd /path/to/addrmsnormbias_problem_1742_template
source /usr/local/Ascend/ascend-toolkit/set_env.sh
./run.sh
```

`run.sh` 会使用模板的 `CMakeLists.txt` 编译 `main.asc`，再生成样例输入并执行 `run_kernel`。模板默认只验证 FP16、形状 `[1, 64]`；真机阶段需要另行覆盖 FP32、BF16、2D/3D/4D、跨行非对齐 D 和平台真实测试点。

本机没有 `ASCEND_HOME_PATH`、Ascend C 编译器、`kernel_operator.h` 或 NPU，因此本节命令没有在本机执行。不能把普通 C++ 编译或 CPU 参考计算当成 Ascend C 构建结果。

### A1. 真机从零到跑通 run.sh（要点，完整 11 步见 Agent08 §7）

1. 确认设备：`npu-smi info`（驱动/固件就绪）+ `source <ASCEND_HOME_PATH>/set_env.sh`；run.sh 内置这两层检查。
2. 环境变量：`ASCEND_HOME_PATH`（set_env.sh 设置）、`ASCEND_CANN_PACKAGE_PATH`、`ASCEND_OPP_PATH`、`DDK_PATH`、`NPU_HOST_LIB`、`LD_LIBRARY_PATH`（作用表见 Agent08 §6）。
3. SoC 未最终确认时以模板默认 `dav-2201`（=910B/A2 系）编译；若判题机为其它型号用 `-DNPU_ARCH=` 覆盖。**提交前必须与判题平台核对 SoC**。
4. CANN 9.0.0 ↔ Ascend HDK 26.0.RC1/25.5.2/25.5.1 配套（gitcode.com/cann/release-management）；安装顺序 toolkit→910b-ops。

### A2. 2026-09-12 调研2 补充的编译要点

> 依据 `../调研/归档/调研2/Agent08_Linux环境与真机工程.md`（真机环境）与 `../调研/归档/调研2/Agent02_官方Ascend_C_API.md`（API）。

1. **模板编译入口事实（A 级）**：`CMakeLists.txt` 用 `find_package(ASC REQUIRED)` + `project(... LANGUAGES ASC CXX)`，编译选项 `$<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>`，链接 `tiling_api register platform unified_dlog dl m graph_base`。`SOC_ARCH` 默认 `"dav-2201"`，可用 `NPU_ARCH` 覆盖。
2. **`run.sh` 的硬约束**：`timeout 120 ./add_rms_norm_bias_custom` —— 单次运行超过 120 秒即判失败。
3. **工具链自检**：`which bisheng ccec msopgen msopst cmake`；which 不到多半是 `set_env.sh` 没生效或 toolkit 装不全。
4. **驱动必须用官方 run 包**，禁止 apt 安装；设备节点自检 `/dev/davinci*`、`/dev/davinci_manager`、`/dev/devmm_svm`、`/dev/hisi_hdc`。
5. **编译失败定位**：先用**未修改的模板**原样编译一次。两次平台 CE（V001 `pipe_` 冲突、V002 首行 `return false;`）都属通道/符号问题，不是算法问题；本队**从未在本地编译成功过一次**。
6. **体量风险**：官方指南记载过大的 kernel 会触发 `out of jump/jumpc imm range`；当前 V002 2955 行 / ~20 个 `Process*` 变体正处在该风险区。首版建议收敛为参数化通用路径；必要时加 `-mllvm -cce-aicore-jump-expand=true` 兜底。
7. **`TPipe` 不要作为 kernel 类成员**（V001 `pipe_` 冲突同款），移到核函数入口、类内保存指针。

## 文件

| 文件 | 说明 |
| --- | --- |
| `源码/add_rms_norm_bias.json` | msopgen 算子原型定义（fp16/bf16/fp32；attr epsilon）——仅作源码清单参考；判题为直调模板 |
| `源码/op_kernel/add_rms_norm_bias.cpp` | Ascend C 核函数（msopgen 工程形态参考） |
| `源码/op_host/add_rms_norm_bias_tiling.h` | Tiling 数据结构定义（msopgen 工程形态参考） |
| `源码/op_host/add_rms_norm_bias.cpp` | 原型注册 / InferShape / Tiling 下发（msopgen 工程形态参考） |
| `源码/CMakePresets.json` / `源码/CMakeLists.txt` / `源码/build.sh` | 源码清单配置；真机以平台直调模板为准 |
| `提交/混合方案/H001-正确性优先/V001/kernel.asc` | V001 直调提交历史（15 点 CE，集中于 pipe_ 宏冲突） |
| `提交/混合方案/H001-正确性优先/V002/kernel.asc` | V002 修复候选版本（已处理尾块参数、归约同步和 64 位偏移，服务器模板 case0 已验证） |
| `提交/混合方案/H001-正确性优先/V003/kernel.asc` | **V003 平台直调主线正确性候选（两遍扫描、FP32 全中间、`CAST_RINT`、尾块和行组分配）** |

## 生成与编译（真机，CANN 9.0.0）

```bash
# 1) 生成工程骨架（判题 SoC 确认后替换 -c 值）
msopgen gen -i 源码/add_rms_norm_bias.json -c ai_core-<soc> -lang cpp -out /tmp/add_rms_norm_bias_gen

# 2) 用本目录实现覆盖骨架
cp 源码/op_kernel/add_rms_norm_bias.cpp     /tmp/add_rms_norm_bias_gen/op_kernel/
cp 源码/op_host/add_rms_norm_bias_tiling.h  /tmp/add_rms_norm_bias_gen/op_host/
cp 源码/op_host/add_rms_norm_bias.cpp       /tmp/add_rms_norm_bias_gen/op_host/

# 3) 编译（使用本目录脚本时传入已生成工程）
ASCENDC_GENERATED_PROJECT_DIR=/tmp/add_rms_norm_bias_gen ./源码/build.sh
```

> 本机没有 msopgen、CANN 或 NPU；上述步骤当前只能作为真机操作说明，不能在本机完成。
> 本目录的 CMakeLists.txt 只用于源码清单配置，不会生成算子库或提交包。

## 已知版本敏感点（真机落地时逐项核对）

1. `kernel.asc` 直调模板和 msopgen 算子工程是两条不同入口，不能把 `run_kernel` 文件直接当成 `op_kernel` 的 tiling 工程入口。判题上传走直调模板（提交物 = 含 `run_kernel` 的 `kernel.asc`）。
2. `Cast` 的 RoundMode：fp16/bf16 输入使用 `CAST_NONE`（精确）；输出使用 `CAST_RINT`（=RNE）。**A2 上 Add/Mul 不支持 bfloat16_t，必须在 FP32 域计算**；fp32→bf16 无 CAST_NONE。舍入行为需在 CANN 9.0.0 真机用非整数值复核。
3. `DataCopyPad` 的模板、产品支持矩阵和 `DataCopyExtParams` 以目标机 `kernel_operator.h` 及对应版本手册为准。**UB→GM 写方向是否"精确写回"（vs padding 覆盖相邻行）是提交前第一验证项**；判题机若非 A2 系需改用 DataCopyCustom/手工尾块（路线 10）。
4. **【字段顺序｜调研2 已定案】**：`DataCopyPadExtParams<T>` 顺序为 `isPad, leftPadding, rightPadding, paddingValue`（CANN 9.0.0 官方 API 页，A 级）；旧笔记写的顺序是错的，争议已消解。**代码规范不变：强制逐字段赋值或 Designated Initializers，禁止裸聚合初始化**（当前 V002 已是逐字段赋值，对该顺序歧义天然免疫）。
5. `ReduceSum` 的临时缓冲在 9.0.0 文档中名为 `sharedTmpBuffer`（旧称 workLocal）。**调研2 已裁决**：官方仅约束「≤UB」，**不存在 4096/16320 这类单一硬上限**；按社区公式核算，fp32 路径需 256 个元素、fp16/bf16 路径需 512 个元素，当前 `WORK_LEN = 1024` **足够（余量 2×）**。工程保守处置照旧：**分块 ≤ 4096**。标量读取主线用 `ReduceSum` + `SetFlag/WaitFlag<HardEvent::V_S>` + `dst.GetValue(0)`；`GetReduceRepeatSumSpr` 经裁决**不需要**引入（仅当 `ReduceSum` 内部标量同步成为性能瓶颈时才作优化候选）。
6. **【寻址防溢出红线】64 位全局行基址**：展平偏移统一声明为 `uint64_t base = static_cast<uint64_t>(row) * dim_;`。
7. **【并发防踩踏红线】多核 32B Cache Line 对齐**：$D$ 非 32B 对齐时，核间行分配必须满足 32 字节边界对齐，消除跨核无锁 DMA 踩踏。
8. 入口限定符：与模板逐字对齐为 `__global__ __vector__`。
9. 官方 CANN 9.0.X API 文档入口为
   <https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/>；本机未安装 CANN 9.0.0，无法从本机头文件确认最终签名。9.0 手册中 DataCopyPad 标 ISASI；`workLocal` 在 9.x 改名 `sharedTmpBuffer`。

## 正确性红线

- 归约与中间计算必须 FP32；输出时一次性 Cast 回原类型。
- 尾块：搬入 DataCopyPad 具名补 0；搬出 DataCopyPad 只写真实字节数，不能用对齐 DataCopy 越过行尾。
- 标量同步：必须配套显式 V_S 屏障，杜绝未定义寄存器读取。
- 全局寻址：强制 64 位整型防溢出。
- 并发安全：多核切分必须具备 32 字节缓存行对齐防护。
- 严禁写死输入/输出、Host 代算或绕过计算流程。
