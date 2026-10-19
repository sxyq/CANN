# Agent 8 调研报告：Linux / 真机工程环境（CANN 挑战赛·西南赛区·AddRmsNormBias）

> 报告人：Agent 8（Linux / 真机工程环境，平台独占）
> 访问日期：2026-09-12
> 主题边界：真机环境准备、编译/运行错误定位、容器与远程 NPU、平台 CE 与本地 CE 差异、最小验证顺序
> 越界说明：本题面/提交规则、Ascend C API 文档、源码仓库、GPU/Triton、编译器代码生成、数值精度、性能/UB、竞赛失败案例均不在本代理范围，已回避。
> 红线声明：本报告所有 URL、标题、作者、日期均来自检索到的公开页面；**凡无法核验的命中帖一律标注「未命中/检索受限」，绝不编造。**

---

## 1. 执行摘要

我们当前最大的痛点是：两次平台提交（V001、V002）均为 15/15 Compile Error，但**本地从未真正编译过一次**（开发机是 macOS，无 CANN 工具链、无昇腾 NPU、无 Docker）。本代理聚焦"拿到一台 Linux + CANN 9.0.0 真机后，如何把模板工程跑起来、如何定位编译错误、kernel 崩溃/超时如何查"。

核心交付三块：
1. **真机环境准备清单**（按执行顺序，可直接照做）—— 驱动/固件、CANN 9.0.0 安装与版本选择、环境变量、工具链确认、容器/远程 NPU 可行性、最小自检。
2. **编译与运行错误「症状 → 原因 → 处置」表（≥10 行）**—— 覆盖 `find_package(ASC)` 失败、`kernel_operator.h` 找不到、`--npu-arch` 不匹配、V001/V002 的 `unknown type name 'pipe_'` / `cannot use dot operator on a type`、`aclInit`/`aclrtSetDevice` 非 0、`ACL_DEV_ATTR_VECTOR_CORE_NUM` 取不到、kernel 超时、device-side 越界/对齐、`GetValue` 未定义、链接期 `tiling_api`/`register` 缺失。
3. **Linux DO 与 V2EX 真实命中帖**—— 见第 4、5 节。**结论：linux.do 未检索到任何真实昇腾帖子；V2EX 仅检索到 1 条真实帖子（t/1207741），未达到"各 ≥3 条"的硬验收**，其余检索结果均为 CSDN / hiascend 官方论坛 / 百度百科等，非这两个站点主站帖子。已如实记录，未编造。

> 重要提示（结合 V001/V002）：本挑战赛判题形态为 **A 级 Direct Invocation 单文件**，`kernel.asc` 被 `#include` 进 `main.asc`，构建用 CMake（`find_package(ASC REQUIRED)`、`project(... LANGUAGES ASC CXX)`、`set(SOC_ARCH "dav-2201")` 可被 `NPU_ARCH` 覆盖、`target_compile_options(... --npu-arch=${SOC_ARCH}>)`）。这意味着编译 kernel 的不是普通 GCC，而是 **毕昇编译器 `bisheng` / `ccec`**（通过 `LANGUAGES ASC` 触发）。V001 的 `unknown type name 'pipe_'` 与 `cannot use dot operator on a type` 大概率是**平台侧模板/打包/版本错位**导致的解析异常，而非我们手写的算法问题——这类错误必须靠"真机本地复现一次"才能确认，本地没有编译链路正是盲区。

---

## 2. 真机环境准备清单（可执行步骤）

> 适用前提：一台装有昇腾 NPU 的 Linux 物理机/云主机（赛事判题用 `dav-2201` 架构，对应 Atlas 200T A2 / 推理卡类 davinci 架构；实际机型以赛题下发为准）。本清单以 **CANN 9.0.0 社区版**为基准（CANN 开源后社区版下载不受限，商用版需申请权限）。

### 步骤 0：确认硬件与系统（在装任何东西之前）
```bash
# 0.1 看 NPU 是否物理在位（昇腾卡 PCIe 设备）
lspci | grep -i -E 'd801|d802|Processing accelerators'
#   老架构多为 d801/d802；新架构以实际为准，出现即代表卡在位

# 0.2 系统/架构
uname -m                 # x86_64 或 aarch64（务必确认，驱动/包架构必须匹配）
cat /etc/os-release | grep PRETTY_NAME

# 0.3 是否已装驱动
npu-smi info             # 能回显设备即驱动已装；命令不存在则需装驱动
```

### 步骤 1：安装驱动与固件（必须使用官方 run 包，**禁止 apt 安装驱动**）
> 驱动/固件必须走华为官方 run 包或官方镜像，不能用 OS 包管理器。先查版本配套表（CANN 版本 ↔ 驱动版本 ↔ 固件版本必须严格匹配），再下载对应包。
```bash
# 1.1 装依赖（Debian/Ubuntu）
sudo apt-get update
sudo apt-get install -y gcc g++ make cmake zlib1g zlib1g-dev \
  openssl libsqlite3-dev libssl-dev libffi-dev unzip pciutils net-tools dkms \
  linux-headers-$(uname -r)
#   （openEuler/CentOS 用 yum install -y gcc gcc-c++ make cmake dkms \
#     kernel-headers-$(uname -r) kernel-devel-$(uname -r) 等）

# 1.2 创建运行用户（推荐 HwHiAiUser；若用其他名，安装驱动固件时必须显式指定）
sudo groupadd -g 1000 HwHiAiUser
sudo useradd -g HwHiAiUser -u 1000 -d /home/HwHiAiUser -m HwHiAiUser -s /bin/bash

# 1.3 安装驱动（run 包文件名以实际下载为准；先 --check 校验一致性）
chmod +x Ascend-hdk-<soc>-npu-driver_<ver>_linux-<arch>.run
chmod +x Ascend-hdk-<soc>-npu-firmware_<ver>.run
./Ascend-hdk-<soc>-npu-driver_<ver>_linux-<arch>.run --full --install-for-all
./Ascend-hdk-<soc>-npu-firmware_<ver>.run --full
#   注意：覆盖安装时顺序是 先固件后驱动；首次安装是 先驱动后固件（以官方文档为准）
sudo reboot            # 驱动生效通常需要重启

# 1.4 自检
npu-smi info           # 期望：列出 Device 0..N，Health OK
ls /dev/davinci* /dev/davinci_manager /dev/devmm_svm /dev/hisi_hdc
#   期望设备节点存在；这些是后续容器挂载与运行时初始化的关键节点
```

### 步骤 2：安装 CANN 9.0.0 开发套件（toolkit）
> 社区版可在昇腾社区下载；商用版需申请。判题用 `dav-2201` 属推理/开发类，装 **Toolkit**（含编译器 `bisheng`/`ccec`、Ascend C 头文件、tiling、runtime）。
```bash
# 2.1 装 Python 依赖（CANN 要求特定 Python 版本区间，通常 3.7.5~3.13.x，具体看配套表）
python3 --version
pip3 install attrs numpy decorator sympy cffi pyyaml pathlib2 psutil protobuf scipy requests absl-py wheel typing_extensions

# 2.2 安装 toolkit（run 包）
chmod +x Ascend-cann-toolkit_9.0.0_linux-<arch>.run
./Ascend-cann-toolkit_9.0.0_linux-<arch>.run --check
./Ascend-cann-toolkit_9.0.0_linux-<arch>.run --install
#   可选：安装 kernels 算子包（单算子 API/二进制 kernel，部分场景提升编译性能）
./Ascend-cann-kernels-<soc>_9.0.0_linux-<arch>.run --install

# 2.3 激活环境变量（**关键**：判题 run.sh 第一行就是它）
source /usr/local/Ascend/ascend-toolkit/set_env.sh
echo $ASCEND_HOME_PATH          # 期望输出 /usr/local/Ascend/ascend-toolkit
```
> 环境变量要点（易踩坑）：
> - 正确变量名是 **`ASCEND_HOME_PATH`**（不是旧版的 `ASCEND_HOME`）。CMake 的 `find_package(ASC)` 会读它。
> - `set_env.sh` 会一并设置 `PATH`（含 `bisheng`/`ccec`/`msopgen`/`msopst`）、`LD_LIBRARY_PATH`、`PYTHONPATH` 等。
> - 把 `source .../set_env.sh` 写入 `~/.bashrc` 以免每次新开终端丢环境。

### 步骤 3：工具链确认（拿到机器第一件事）
```bash
which bisheng ccec msopgen msopst cmake
#   bisheng / ccec：毕昇编译器，负责把 Ascend C 的 .asc 编译成 device 侧二进制
#   msopgen：算子工程脚手架；msopst：算子 UT/ST 测试
#   若 which 不到 → 多半是步骤 2.3 的环境变量没生效，或 toolkit 没装全
ls $ASCEND_HOME_PATH/compiler/ccec_compiler/bin/   # bisheng 所在目录
```

### 步骤 4：CANN 多版本切换（若机器上有多个 CANN 共存）
- 官方 `set_env.sh` 按"安装路径最新软链"加载；多版本并存时，手动指向具体版本路径即可：
```bash
# 例：机器同时有 8.x 与 9.0.0，显式指定
source /usr/local/Ascend/ascend-toolkit/9.0.0/set_env.sh
# 或用 ASCEND_HOME_PATH 显式覆盖
export ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/9.0.0
```
- **版本严格配套**是头号杀手：驱动/固件/CANN/（若用 torch_npu 则还有 PyTorch 版本）必须匹配，跨版本混用会出现 `aclInit` 非 0、`ascendrt_xxx failed`、ABI 符号找不到等。

### 步骤 5：跑通模板自带 `run.sh` 的最小前置条件
判题 `run.sh` 流程：`source set_env.sh` → `cmake .. && make -j4` → `python3 ../scripts/gen_data.py` → `timeout 120 ./add_rms_norm_bias_custom`。本地最小自检需满足：
1. 步骤 1~3 全部通过（NPU 可见、CANN 9.0.0 已装、`bisheng`/`ccec` 可用）。
2. `cmake` ≥ 3.14（模板用 `LANGUAGES ASC CXX` 需要 CMake 能识别 ASC 语言模块，来自 CANN toolkit 提供的 `FindASC.cmake`）。
3. `python3` 与 `numpy` 可用（`gen_data.py` 生成测试数据）。
4. 模板目录结构完整（`CMakeLists.txt`、`main.asc`、`kernel.asc`、`scripts/gen_data.py`）。
5. 若机器无 NPU 但有 toolkit，可验证**编译**能否通过；但 `./add_rms_norm_bias_custom` 运行需要真实 NPU（或模拟运行，视 toolkit 是否带 CPU 仿真）。**判题是直调单文件，必须有 NPU 才能跑运行期。**

---

## 3. 编译与运行错误处置表（≥10 行，硬性验收）

> 符号约定：`*_soc` = 实际 SoC 型号；`<arch>` = x86_64 / aarch64。

| # | 症状（报错/现象） | 可能原因 | 处置建议 | 关联我们的 V001/V002 |
|---|---|---|---|---|
| 1 | `CMake Error: Could NOT find ASC (missing: ASC_COMPILER)` 或 `find_package(ASC REQUIRED)` 失败 | 未 `source set_env.sh`；CANN 没装或装错版本；`ASCEND_HOME_PATH` 指向空 | 先 `source /usr/local/Ascend/ascend-toolkit/set_env.sh`；`echo $ASCEND_HOME_PATH` 非空；重装 toolkit。`FindASC.cmake` 由 toolkit 提供，CMAKE_PREFIX_PATH 需覆盖它。 | 非 V001/V002 现象 |
| 2 | `kernel_operator.h: No such file or directory` | toolkit 头文件路径未进 include；环境变量未生效；使用了错误 include 路径 | `source set_env.sh` 后，头文件在 `$ASCEND_HOME_PATH/include`；CMake 里 `include_directories(${ASCEND_HOME_PATH}/include)`。不要手写绝对旧路径。 | 非 |
| 3 | `--npu-arch=xxx: invalid / unrecognized` 或 device 端编译报架构不匹配 | `SOC_ARCH` 与机器不一致；`NPU_ARCH` 覆盖值写错 | 判题模板默认 `dav-2201`；本地真机若是 910B/310 等，用 `-DNPU_ARCH=<真实soc>` 覆盖，或改 `CMakeLists.txt` 的 `SOC_ARCH`。架构字符串必须 toolkit 支持。 | 非 |
| 4 | `unknown type name 'pipe_'; did you mean 'pipe_t'?` | V001 平台日志现象。**高度疑似平台侧模板/打包/版本错位**导致 ASC 源被错误地当成普通 C/C++ 解析，或 `pipe_` 是某内部类型被错误展开/截断。本地同文件若用 `bisheng` 编译正常，则证明非算法问题 | 拿到真机后用模板原样 `cmake && make`，确认 `bisheng` 能否正常吃 `.asc`；若本地 OK，则向赛事方反馈"平台 CE 不可本地复现"，附本地编译日志。 | **= V001** |
| 5 | `cannot use dot operator on a type` | V001/V002 同类：**ASC/C++ 解析阶段**对非类型使用 `.` 操作符。多为宏/模板展开异常、或源文件被以错误语言/编译器解析（如被当成纯 C++）。本地用 `bisheng` 正常则说明是平台侧问题 | 同上：真机本地复现比对；确认 `.asc` 被 `LANGUAGES ASC` 正确交给 `bisheng` 而非 host 的 g++。 | **= V001（及 V002 同源）** |
| 6 | `kernel.asc` 首行是 `return false;`（V002 平台实际收到内容） | **上传内容被截断/异常**，不是算法问题。平台收到的文件与本地 2955 行快照完全不同 | 立即核对提交打包链路：是否被二次处理、是否被压缩/编码损坏、文件名是否被改名导致 `#include "kernel.asc"` 错位。重新生成 `kernel.asc` 并校验字节数/首行。 | **= V002** |
| 7 | `aclInit / aclrtSetDevice` 返回非 0（如 507001/107001/507011） | 设备未初始化：驱动未装/未生效；`ASCEND_RT_VISIBLE_DEVICES` 设错；运行用户无 `/dev/davinci*` 权限；环境变量未 source | 重跑 `npu-smi info`；`ls -l /dev/davinci0` 属主应为 `HwHiAiUser`；用 `sudo` 或加入 `HwHiAiUser` 组；检查 `ASCEND_RT_VISIBLE_DEVICES`。错误码含义以官方错误码文档为准（如 507011 与 ACL 内部错误相关，507008 与 soc version 相关）。 | 非 |
| 8 | 取不到 `ACL_DEV_ATTR_VECTOR_CORE_NUM`（返回 0/默认/-1） | 查询设备属性接口调用方式不对；驱动/toolkit 版本不匹配；在 CPU 仿真或错误设备上查询 | 用 `aclrtGetDeviceCount`/`aclrtGetDeviceProperties` 正确流程；确认在 `aclrtSetDevice` 之后查询；若用 `aclrtDeviceGetAttribute`，属性枚举值用对。多版本配套问题参见步骤 4。 | 非 |
| 9 | kernel 超时（`aicitlesure` / `aicore timeout` / `timeout 120` 被 kill） | kernel 死循环、UB 溢出导致硬件挂死、队列未平衡、`GetStartIndex`/`GetTileShape` 计算 tile 数异常导致无限循环、DMA 等待永不就绪 | 先减小数据规模复现；用 `printf`/日志在 Init/Process 中打点；检查 tiling 的 `totalLength`/`tileNum`；设置 `ASCEND_GLOBAL_LOG_LEVEL=3` 看 device 侧日志；真机上可用 `msprof` 抓 kernel 执行。 | 非 |
| 10 | device-side 越界 / `EZ999` 类 device assert / 计算结果 NaN | UB（`Unified Buffer`）分配超过容量（通常 ≤ 几十 KB~MB 级，按核与 UB 规格）；`AllocTensor` 大小算错；`DataCopy` 越界读 GM | 核对 `BLOCK_SIZE`/`TILE` 与 UB 容量；用 `GetValue` 取回长度做边界判断；开启 `ASCEND_GLOBAL_LOG_LEVEL=3` 看 device 报错位置。 | 非 |
| 11 | DMA 地址对齐错误（`GM_ADDR` 未 32 字节对齐等） | `GlobalTensor` 起始地址/长度未按 Ascend C 要求的对齐（如 32B） | 确保 tensor 长度与起始偏移对齐；`SetGlobalBuffer` 的偏移对齐；用 `DataCopy` 时长度取对齐后值。 | 非 |
| 12 | `GetValue` 读回未定义/错误值（host 读 tiling 结果错） | Tiling 计算在 host 侧，`GetValue` 取的是 `TilingData` 里的字段，字段名/偏移/类型不匹配；或 tiling 结构体未对齐 pack | 用 `AddToList`/`GetValue` 严格对应字段；检查 `TilingData` 的 `SetData` 顺序与 `GetValue<type>("field")` 一致；host/device 两侧结构定义必须一致。 | 非 |
| 13 | 链接期找不到 `tiling_api` / `register` / `platform` / `unified_dlog` | `target_link_libraries` 漏链（模板已链 `tiling_api register platform unified_dlog dl m graph_base`，若手改 CMake 误删） | 恢复模板原 CMake 的 link 列表；这些库来自 toolkit，`LD_LIBRARY_PATH` 需覆盖 `lib64`。缺失 `dl` 也会报 `undefined reference to dlsym` 类错误。 | 非（但若手改需警惕） |
| 14 | `ImportError: libascendcl.so: cannot open shared object file` | CANN 环境变量未 source，`LD_LIBRARY_PATH` 没含 toolkit `lib64` | `source set_env.sh` 或显式 `export LD_LIBRARY_PATH=$ASCEND_HOME_PATH/lib64:$LD_LIBRARY_PATH`。 | 非 |

> 对 V001/V002 的专项判断：**第 4、5、6 行**正是我们两次 CE 的现场。它们的共同特征是"本地无法复现"——因为本地根本没有 CANN/`bisheng` 编译链路。要在真机上**用模板原样**编译一次，才能区分"平台侧模板/打包问题"还是"我们手写 kernel 真有问题"。

---

## 4. Linux DO 命中帖清单

> **检索结论：未命中（受限）。linux.do 上未检索到任何与「昇腾 / Ascend / 华为 NPU / CANN」相关的真实帖子。**
>
> 已使用的检索方式与证据：
> 1. 搜索引擎 `site:linux.do 昇腾`、`site:linux.do Ascend`、`site:linux.do NPU CANN 安装`、`"linux.do" Ascend OR "linux.do" NPU`、`linux.do 华为 NPU 帖子` —— 返回结果均为 hiascend 官方论坛、CSDN、百度百科、掘金介绍文等，**没有 linux.do 主站帖子**。
> 2. 直接抓取站内搜索页 `https://linux.do/search?q=昇腾`、`?q=Ascend`，以及 Discourse JSON 接口 `https://linux.do/search.json?q=昇腾` / `?q=Ascend` —— **均 `fetch failed`（网络/站点阻断，无法访问）**，无法核验是否存在帖子。
> 3. 背景补充：linux.do 是 2024-01 上线的 Discourse 中文技术社区（域名虽叫 linux.do，但内容以 AI/大模型/Agent 为主，见掘金介绍文 [S217]），其公开资料（DeepWiki [S218]）列举的"社区话题"覆盖 AI 服务、独立开发、开源项目等，未提昇腾算子开发类内容。综合判断：该站以大模型讨论为主，**昇腾自定义算子 / CANN 环境搭建类帖子极少见或不存在**。
>
> **依据红线，本代理不编造任何 linux.do 标题/作者/日期/链接。** 若后续需要，建议在能直连 linux.do 的网络环境下，用站内搜索 `Ascend` / `昇腾` / `NPU` 二次确认。

---

## 5. V2EX 命中帖清单

> **检索结论：部分命中，但未达"≥3 条"硬验收。** 搜索引擎仅检索到 **1 条**真实 V2EX 主站帖子；其余以"昇腾/华为 NPU"为关键词命中的均为 CSDN、hiascend 官方论坛、百度百科、博客园、爱企查等，**不是 V2EX 帖子**，故不计入。已如实标注，不编造。

### 真实命中（V2EX 主站，1 条）

| 标题 | 作者/楼主 | 发帖日期 | 完整 URL | 要点 |
|---|---|---|---|---|
| 中国的算力缺口这么大嘛?看到 2025 华为昇腾出货 81 万块,又看到各家 coding plan 不是停售就是限流 | 主题帖（V2EX 节点"硬件"/"程序员"类，作者以帖内楼层讨论呈现，未提供单一署名） | 帖子 URL 含 `/t/1207741`，检索页未回显精确发帖日（楼层讨论围绕 2025 出货与 2026 适配） | https://www.v2ex.com/t/1207741 | 讨论华为昇腾出货量、软件生态短板（"写算子特费劲""CUDA+Triton 那套好用"）、国产替代采购现状。**与本任务直接相关度低（偏宏观/生态吐槽，非环境搭建实操）**，但确为 V2EX 真实帖子，作为"真实命中"记录。 |

> 说明：该帖作者为匿名社区帖（V2EX 帖子以主题呈现，回帖者包括 `@Laobai`、`@lynn1su`、`@EasonYan` 等楼层用户，无单一"楼主署名"字段），发帖日期检索结果未直接回显（URL 编号 t/1207741，结合内容提及 2025 出货与 2026 适配，属 2025–2026 时段）。本代理**未虚构作者名与精确日期**，仅如实描述。

### 检索到但**不计入**（非 V2EX 主站，仅列证避免误用）
- 昇腾社区官方论坛（hiascend.com/forum）多篇"昇腾全栈解析""打破 CUDA 枷锁""A800 双机集群推理变慢"等 —— 属官方论坛，非 V2EX。
- CSDN / 博客园 / 51CTO / ai6s.net / 百度百科 / 爱企查 等关于 910B、CANN 安装、torch_npu 迁移的文章 —— 非 V2EX。
- 华为开发者论坛 `developer.huawei.com/home/forum/ascend/thread-02178214133377043079`「昇腾 NPU 环境下 PyTorch 项目迁移」—— 华为官方论坛，非 V2EX，但其经验（环境组成、版本必须匹配、CANN 环境变量、`torch_npu.npu.is_available()` vs `torch.cuda.is_available()`）**对本任务有参考价值**，作为来源 [S216] 收录于来源清单。

> **诚实声明**：V2EX 在"硬件""程序员""问与答"等节点可能存在更多昇腾相关讨论，但受检索手段（搜索引擎索引不全 + 站内搜索 `https://www.v2ex.com/search?q=昇腾` 同样 `fetch failed`）限制，本代理无法核验更多真实帖子。其余检索尝试（`site:v2ex.com 昇腾 NPU 华为`、`site:v2ex.com 华为 910B 部署`、`site:v2ex.com/t 华为 算力 国产`、`site:v2ex.com torch_npu`、`site:v2ex.com 华为 NPU 训练 推理`）返回均非 V2EX 主站帖。**不编造第 2、3 条。**

---

## 6. 容器与远程 NPU 可行性

### 6.1 物理机直装（推荐用于本赛事真机验证）
- 最稳：按第 2 节在物理机/云主机直接装驱动+固件+CANN toolkit，无容器层干扰，编译/运行期报错最贴近判题环境。
- 判题 `run.sh` 本身就是在裸机 shell 里 `cmake && make && ./add_rms_norm_bias_custom`，**不建议套容器**，避免 device 挂载/权限差异引入新变量。

### 6.2 Docker + 昇腾 runtime（Ascend Docker Runtime）
- 适用：多用户共享服务器、需要干净环境、或跑 MindIE/vLLM 类推理服务。
- 关键组件：**Ascend Docker Runtime**（`Ascend-docker-runtime_<ver>_linux-<arch>.run`），安装后会改写 `/etc/docker/daemon.json`，`systemctl restart docker` 生效。
- 设备节点（必须挂载）：`/dev/davinci0..N`、`/dev/davinci_manager`、`/dev/devmm_svm`、`/dev/hisi_hdc`。
- 目录挂载（缺一不可，否则 `aclInit` 失败）：`-v /usr/local/Ascend/driver:/usr/local/Ascend/driver:ro`、`-v /usr/local/Ascend/add-ons`、`-v /usr/local/sbin`、`-v /usr/local/dcmi`。
- 环境变量：`-e ASCEND_RT_VISIBLE_DEVICES="0,1,..."`、`-e ASCEND_DEVICE_ID=0`（容器内编号从 0 起，即使宿主是 davinci6 也写 0）。
- 验证：`docker exec -it <c> npu-smi info` 能在容器内看到卡。
- 注意：容器里**仍需 source 宿主同版本 CANN 的 `set_env.sh`**（或镜像内置），否则 `bisheng`/`kernel_operator.h` 同样找不到——容器只解决设备/库挂载，不解决编译器。

### 6.3 远程 NPU（云算力 / DevCloud）
- 华为昇腾提供在线开发环境（如 ModelArts DevCloud、HiDevLab），预置 CANN + 镜像，适合临时验证；但赛事判题环境是下发模板在本地真机编译，**远程仅用于"提前跑通编译链路、验证 kernel 逻辑"**，不能完全替代判题机。
- 租用注意：确认分配的 CANN 版本与赛事 `dav-2201`/9.0.0 一致；远程机多为 910B/310，架构字符串不同，要用 `NPU_ARCH` 覆盖。

### 6.4 可行性结论
- **物理机直装最贴近判题，优先**。
- **容器可行**，但仅用于共享/干净环境，且必须正确挂载设备与 driver 目录，并在容器内 source 同版本 CANN 环境变量。
- **远程 NPU 仅作辅助验证**，不能替代判题真机的本地复现（这正是我们当前盲区的解药：去借/租一台能跑 `bisheng` 的机器，把模板原样编译一次）。

---

## 7. 拿到真机后的最小验证顺序

> 目标：用最少步骤，把"能否复现平台 CE"这件事查清楚。

1. **第一步：跑通模板样例（不换任何代码）**
   - 按第 2 节装好环境 → `cd template && mkdir build && cd build && cmake .. && make -j4`。
   - 确认 `make` 能否通过、`bisheng` 是否真的在吃 `.asc`。
   - 若 `make` 就报 V001 那种 `unknown type name 'pipe_'` / `cannot use dot operator on a type`，**记录完整日志**——这说明平台 CE 可在本地复现，问题在模板/工具链，而非我们算法。
   - 若 `make` 通过，继续 `python3 ../scripts/gen_data.py && timeout 120 ./add_rms_norm_bias_custom`，确认能跑出结果（哪怕结果不对，至少运行期通）。

2. **第二步：换入我们自己的 kernel（仅替换 `kernel.asc`，保留模板 `main.asc`/`CMakeLists.txt`）**
   - 用 V001/V002 的 `kernel.asc` 顶替模板的 kernel，重编译运行。
   - 若此时才出现 `pipe_` / dot operator 类错误 → 说明是我们 kernel 写法触发了 `bisheng` 解析异常（但仍要先排除平台侧差异）。
   - 若仍复现"平台收到的首行是 `return false;`"这类截断 → 问题在**提交打包**，不是代码。

3. **第三步：逐项验证算法正确性**
   - 先小数据（tile 数=1、长度对齐 32B）跑通；再放大到判题规模。
   - 用 `gen_data.py` 生成输入，比对 `add_rms_norm_bias` 的黄金值（host 侧用 numpy 实现一份参考）。
   - 逐步加 tiling 分支、vector-core 拆分、精度处理，每加一项重编重跑。

4. **第四步：定位平台 CE 与本地 CE 的差异（见下节）**
   - 把"本地 `make` 日志"与"平台判题日志"逐行对比，确认是否同一工具链、同一 CANN 版本、同一架构字符串。

> 关键纪律：**每一步只改一个变量**。先证明"模板原样能编能跑"，再证明"换入我的 kernel 也能编能跑"，最后才调算法。这是把"从没编译过"变成"每一步都可解释"的最短路径。

---

## 8. 如何定位「平台编译错误」与「本地编译错误」的差异

我们两次平台 CE（V001、V002）都没能在本地复现，根因是本地无编译链路。要区分两者，按以下清单比对：

1. **工具链是否一致**
   - 平台用 `bisheng`/`ccec`（经 `LANGUAGES ASC` 触发）；本地也必须用同一编译器。若本地用 g++ 去编 `.asc`，必然得到与平台不同的错误，**不要这样比**。
   - 确认：`which bisheng`、`bisheng --version` 与平台日志里的编译器版本号对照（若平台日志给出版本）。

2. **CANN 版本是否一致**
   - V001/V002 平台大概率是赛事固定镜像（可能非 9.0.0，或 9.0.0 但补丁不同）。本地尽量对齐同一 CANN 版本；版本差会导致 `FindASC`、头文件、`--npu-arch` 默认值不同。

3. **架构字符串是否一致**
   - 模板默认 `dav-2201`；本地真机若不是该架构，必须用 `NPU_ARCH` 覆盖。架构不匹配会让 `bisheng` 报不同的解析/指令错误，**与平台错误不可直接比较**。

4. **源文件是否真的一致**
   - V002 已经证明：平台收到的 `kernel.asc` 首行是 `return false;`——**文件内容被污染/截断**。这种情况"本地复现"毫无意义，因为本地文件是对的。先校验提交产物字节数、首行、MD5。

5. **比对方法**
   - 把平台判题日志（含编译器命令行、`#include` 展开、`pipe_` 上下文）与本地 `make VERBOSE=1` 输出并排看。
   - 若两者编译器命令行一致但本地无 `pipe_` 错误 → 平台侧模板/打包异常，向赛事方反馈。
   - 若本地也出现 `pipe_` / dot operator → 是我们 kernel（或模板 include 方式）触发了 `bisheng` 的解析边界，按第 3 节表第 4/5 行处置。

> 一句话：**"平台 CE 不能本地复现"的根因是缺编译链路；补上"一台能跑 bisheng 的真机/远程机 + 模板原样编译"这一步，差异就能定位。**

---

## 9. 来源清单（S211–S240）

> 等级：A=官方题面/API/仓库；B=官方样例·源码·测试·原作者资料；C=社区文章·论坛·个人仓库；D=仅搜索摘要或未能核验。
> 状态：verified=已核验内容；partial=部分核验；unavailable=无法访问/受限；contradicted=被证伪。

- [S211] 昇腾社区·快速安装CANN（含驱动/固件/CANN 7.0 示例） | https://www.hiascend.com/doc_center/source/zh/canncommercial/700/quickstart/quickstart/quickstart_18_0002.html | 平台 hiascend(官方) | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：步骤1/2 驱动固件与CANN安装、npu-smi info自检、set_env.sh | 可支持的结论：驱动/固件+CANN安装顺序、npu-smi自检、环境变量激活方式为官方标准流程
- [S212] hiascend·CANN社区版软件安装指南（选择安装场景/软件包清单） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/800alpha001/softwareinst/instg/instg_0001.html | 平台 hiascend(官方) | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：软件包清单（driver/firmware/toolkit/nnae/nnrt/kernels）、社区版vs商用版 | 可支持的结论：CANN分社区版/商用版；toolkit含算子编译与runtime；kernels包用途
- [S213] hiascend·NPU Driver and Firmware Installation（英文，PM/容器/VM 场景） | https://www.hiascend.com/document/detail/en/canncommercial/800/softwareinst/instg/instg_0005.html | 平台 hiascend(官方) | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：驱动/固件安装命令、`--check`、`--full --install-for-all`、HwHiAiUser 运行用户、重启用 npu-smi info 验证 | 可支持的结论：驱动固件必须用run包；首次/覆盖安装顺序不同；安装后reboot并npu-smi验证
- [S214] hiascend·编译选项（毕昇编译器 bisheng/ccec） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/83RC1alpha003/opdevg/BishengCompiler/atlas_bisheng_10_0010.html | 平台 hiascend(官方) | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：bisheng/ccec位置（`${INSTALL_DIR}/compiler/ccec_compiler`）、set_env.sh激活、--npu-arch概念 | 可支持的结论：毕昇编译器随CANN发布；通过set_env.sh或PATH激活；ccec_compiler/bin含bisheng
- [S215] hiascend·快速安装FAQ（9.x 合一包、whitelist、apt/yum在线安装） | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/920beta2/softwareinst/instg/instg_0050.html | 平台 hiascend(官方) | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：9.x版本在线/离线安装、whitelist=driver/toolkit、apt-get/conda安装 | 可支持的结论：CANN 9.x支持合一包与组件白名单安装；在线安装需OS源支持
- [S216] 华为开发者论坛·昇腾 NPU 环境下 PyTorch 项目迁移与 torch_npu 配置踩坑 | https://developer.huawei.com/home/forum/ascend/thread-02178214133377043079-1-1.html | 平台 华为开发者论坛 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：CANN与torch_npu环境组成、版本必须匹配、set_env.sh、NPU可用性验证、常见报错(libascendcl.so等) | 可支持的结论：驱动/固件/CANN/PyTorch/torch_npu版本严格配套；CANN环境变量未source会致动态库找不到
- [S217] 掘金·linux.do：一个被严重低估的中文技术社区（linux.do 背景介绍） | https://juejin.cn/post/7627405638134972462 | 平台 掘金(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：解释 linux.do 是什么、内容以AI/大模型为主、基于Discourse、2024-01上线 | 可支持的结论：linux.do并非Linux发行版论坛，主聊AI大模型，昇腾算子开发类帖子极少见
- [S218] DeepWiki·Linux Do Wiki·Services Documentation（linux.do 社区构成） | https://deepwiki.com/chenyme/Linux-Do-Wiki/3-services-documentation | 平台 DeepWiki(社区索引) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：linux.do社区服务/AI服务/知识库构成，无昇腾开发板块 | 可支持的结论：linux.do公开资料中无昇腾/CANN环境搭建类板块，佐证"未命中"
- [S219] CSDN·CANNBot Ascend C 直调开发指南（环境变量/CMake/find_package(ASC)） | https://blog.csdn.net/gitblog_00909/article/details/151507556 | 平台 CSDN(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：ASCEND_HOME_PATH(非ASCEND_HOME)正确名、bisheng路径、find_package(ASC)自动发现、直调工程配置 | 可支持的结论：环境变量正确名为ASCEND_HOME_PATH；CMake经find_package(ASC)发现bisheng；直调开发需host侧算tiling
- [S220] CANN开发者社区·昇腾平台环境搭建（apt 在线装 toolkit、set_env.sh） | https://cann.csdn.net/6a38e45c10ee7a33f280baa8.html | 平台 CSDN(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：apt-cache search ascend-cann-toolkit、source set_env.sh、echo $ASCEND_HOME_PATH | 可支持的结论：CANN社区版可apt在线安装(默认/usr/local/Ascend)；set_env.sh激活后ASCEND_HOME_PATH生效
- [S221] ai6s.net·昇腾 910B 服务器初始化（驱动/固件/CANN/容器挂载设备清单） | https://ai6s.net/691d5df70e4c466a32e92168.html | 平台 ai6s.net(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：Ascend Docker Runtime安装、docker run挂载/dev/davinci*与driver目录、ASCEND_RT_VISIBLE_DEVICES | 可支持的结论：容器用昇腾NPU必须装Ascend Docker Runtime并正确挂载设备与driver；容器内编号从0起
- [S222] 51CTO·基于Aarch64 openEuler 的 910B 离线部署（驱动固件安装实操） | https://blog.51cto.com/u_16099278/14255405 | 平台 51CTO(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：--check校验、--full --install-username=root、npu-smi info、驱动DKMS模式 | 可支持的结论：非HwHiAiUser用户安装驱动需显式--install-username/--install-usergroup；校验包完整性用--check
- [S223] ai6s.net·Ascend C 入门实战：从零构建昇腾 AI 加速算子（环境/编程模型/编译） | https://ai6s.net/693f91f40800f3458b825754.html | 平台 ai6s.net(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：kernel_operator.h作用、GM/UB三级存储、AllocTensor/FreeTensor、UB溢出、CMake示例 | 可支持的结论：Ascend C基于C++17扩展；UB容量有限需控制AllocTensor大小；E40021为算子编译失败类错误
- [S224] CSDN·快速安装/升级/卸载 CANN/Ascend 配套软件包（社区版/商用版、run包参数） | https://blog.csdn.net/m0_37605642/article/details/137511287 | 平台 CSDN(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：run包--check/--install/--install-path/--full/--uninstall、社区版不受限 | 可支持的结论：Ascend配套软分社区版/商用版；CANN含runtime/compiler/opp/toolkit；安装路径可自定义
- [S225] ascend.readthedocs.io·快速安装昇腾环境（系统要求/驱动自检） | https://ascend.readthedocs.io/zh-cn/latest/sources/ascend/quick_install.html | 平台 昇腾开源readthedocs(官方镜像) | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：lspci确认卡、uname/cat os-release、npu-smi info验证、创建HwHiAiUser | 可支持的结论：开源安装文档的系统要求与驱动安装后npu-smi验证流程
- [S226] CSDN·hwcomputing·昇腾 910B NPU 大模型部署实践（vLLM/容器设备映射/驱动挂载不全报错） | https://hwcomputing.csdn.net/69a055dd0a2f6a37c593d81b.html | 平台 CSDN(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：容器内ASCEND_RT_VISIBLE_DEVICES设置、aclInit 107001 Invalid device ID、fwkacllib必须挂载 | 可支持的结论：容器内NPU设备ID从0起；驱动库挂载不全致aclInit失败；bridge模式-p与--network=host冲突
- [S227] ai6s.net·小模型在昇腾NPU上的推理部署（torch_npu 版本配套表） | https://ai6s.net/698ed5d454b52172bc5b75e9.html | 平台 ai6s.net(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：CANN/PyTorch/torch_npu三者版本必须匹配、Docker容器示例 | 可支持的结论：torch_npu依赖CANN，版本严格配套；推荐用官方镜像避免环境错配
- [S228] 百度百科·华为昇腾NPU（架构/产品/软件生态概述） | https://baike.baidu.com/item/%E5%8D%8E%E4%B8%BA%E6%98%87%E8%85%BENPU/67703028 | 平台 百度百科 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：达芬奇架构Cube/Vector/Scalar、CANN开源(2025-08)、950系列 | 可支持的结论：昇腾NPU采用达芬奇架构；CANN于2025年开源开放（背景信息）
- [S229] V2EX·中国的算力缺口这么大嘛?看到 2025 华为昇腾出货 81 万块… | https://www.v2ex.com/t/1207741 | 平台 V2EX(社区，真实命中) | 访问日期 2026-09-12 | 等级 C | 状态 partial | 用途：反映昇腾软件生态短板与写算子难度（社区真实讨论） | 可支持的结论：社区侧对"昇腾写算子费劲、CUDA/Triton生态好用"有广泛共鸣；**仅1条真实V2EX命中，未达≥3条验收**
- [S230] 百度百科·华为昇腾（产品系列/950PR-DT/970路线） | https://baike.baidu.com/item/%E5%8D%8E%E4%B8%BA%E6%98%87%E8%85%BE/68598325 | 平台 百度百科 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：昇腾型号参数、CANN开源、Atlas系列形态 | 可支持的结论：昇腾950PR/DT分别面向推理prefill/decode；CANN已开源开放（背景）
- [S231] hiascend·昇腾社区官方论坛首页（论坛性质，非V2EX/linux.do） | https://www.hiascend.com/forum | 平台 hiascend(官方) | 访问日期 2026-09-12 | 等级 A | 状态 verified | 用途：说明"昇腾论坛"主要指hiascend官方论坛，与V2EX/linux.do区分 | 可支持的结论：昇腾相关讨论主阵地是hiascend官方论坛，非V2EX/linux.do
- [S232] hiascend·打破CUDA枷锁：昇腾如何重塑大模型时代的国产AI算力版图（论坛帖，非V2EX） | https://www.hiascend.com/forum/thread-02178215697926099297-1-1.html | 平台 hiascend(官方论坛) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：达芬奇架构vs CUDA、CANN生态 | 可支持的结论：作为"非V2EX帖子"的证据，说明检索到的昇腾讨论多集中在hiascend官方论坛
- [S233] CSDN·昇腾社区·AI推理的NPU加速:cann-recipes-harmony-infer实战（鸿蒙NPU，非Linux服务端） | https://harmonyosdev.csdn.net/6a11d44a10ee7a33f274ae10.html | 平台 CSDN(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：说明鸿蒙NPU推理与Linux/Android NPU推理接口不兼容（不同环境） | 可支持的结论：鸿蒙NPU推理用NAPI/零拷贝，与Linux服务端CANN/ccec路线不同，本题不采用
- [S234] Tom's Hardware·Huawei Ascend NPU roadmap(950/960/970, FP8/FP4, UnifiedBus) | https://www.tomshardware.com/tech-industry/artificial-intelligence/huawei-ascend-npu-roadmap-examined-company-targets-4-zettaflops-fp4-performance-by-2028-amid-manufacturing-constraints | 平台 Tom's Hardware(媒体) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：950系列SIMD+SIMT架构、FP8/MXFP4/HiF4、UnifiedBus互联 | 可支持的结论：950起走SIMD+SIMT（类GPGPU），算子开发更高效，可匹配CUDA算子（背景，非实操）
- [S235] 21世纪经济报道·昇腾破局 国产算力不再低调（DeepSeek/CloudMatrix 384） | https://www.21jingji.com/article/20250624/4308cca8d1655447800e83b326361246.html | 平台 21财经(媒体) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：昇腾超节点、系统工程、CANN开源背景 | 可支持的结论：昇腾通过超节点/系统工程补单芯片差距；生态仍落后CUDA（背景）
- [S236] 华尔街见闻·华为AI芯片来时的路（《昇腾崛起》整理，CANN灵魂论） | https://wallstreetcn.com/articles/3779151 | 平台 华尔街见闻(媒体) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：CANN被称为"昇腾的灵魂"、Ascend C由Tik演进、全下沉调度 | 可支持的结论：CANN/毕昇是算子开发核心；Ascend C于2022定名（背景，解释为何写算子是生态重点）
- [S237] 海昆鹏·基于鲲鹏CPU推理服务器部署DeepSeek 70B（CANN 8.1.RC1、kernels、nnal） | https://www.hikunpeng.com/developer/techArticles/20250602-1 | 平台 海昆鹏(鲲鹏社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：CANN toolkit/kernels/nnal安装、set_env.sh、torch_npu版本、绑核优化 | 可支持的结论：CANN安装含toolkit+kernels+nnal；set_env.sh激活；版本配套
- [S238] CSDN·Ascend 910B NPU 部署文档（驱动/固件安装实操，社区版） | https://ascendai.csdn.net/69d4c82c72111d255bf7ca27.html | 平台 CSDN(社区) | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：HwHiAiUser创建、driver/firmware下载与安装、npu-smi info | 可支持的结论：910B部署前必须先装驱动固件并npu-smi验证（与步骤1一致）
- [S239] 百度百科·华为集群计算（CANN开源、灵衢UnifiedBus、950/960/970路线） | https://baike.baidu.com/item/%E5%8D%8E%E4%B8%BA%E9%9B%86%E7%BE%A4%E8%AE%A1%E7%AE%97/58964236 | 平台 百度百科 | 访问日期 2026-09-12 | 等级 C | 状态 verified | 用途：CANN开源(2025-12)、灵衢协议、集群计算方案 | 可支持的结论：CANN开源开放已成定局，社区版下载不受限（支撑步骤2版本选择）
- [S240] Agent 8 检索受限声明（linux.do/V2EX 站内搜索接口 fetch failed） | 站内检索 https://linux.do/search.json?q=Ascend 与 https://www.v2ex.com/search?q=昇腾 均 fetch failed | 平台 检索工具(受限) | 访问日期 2026-09-12 | 等级 D | 状态 unavailable | 用途：记录 linux.do/V2EX 站内搜索不可达，佐证"未命中/受限"结论 | 可支持的结论：linux.do/V2EX 真实帖子须以可访问页面为准；当前无法直连核验，故仅以搜索引擎命中为准，未编造

---

## 附：本次检索方法与限制（透明度记录）

- 搜索引擎（site 限定）：`site:linux.do 昇腾`、`site:linux.do Ascend`、`site:linux.do NPU CANN 安装`、`"linux.do" Ascend OR "linux.do" NPU`、`linux.do 华为 NPU 帖子`；`site:v2ex.com 昇腾`、`site:v2ex.com 华为 昇腾 算力`、`site:v2ex.com 华为 910B 部署`、`site:v2ex.com/t 华为 算力 国产`、`site:v2ex.com 华为 NPU 训练 推理`、`site:v2ex.com torch_npu`。
- 站内直连：linux.do 与 v2ex.com 的 `/search` 与 `search.json` 接口均 `fetch failed`（网络/站点阻断）。
- 结果分布：昇腾/Ascend 相关公开内容主要集中在 **hiascend 官方论坛/文档、CSDN、百度百科、ai6s.net、51CTO、博客园、媒体站**；**linux.do 未见任何真实帖子；V2EX 仅 1 条真实帖子（t/1207741）**。
- 红线遵守：本报告未编造任何 linux.do/V2EX 的标题、作者、日期或链接；对未达验收的站点如实标注"未命中/受限"。
