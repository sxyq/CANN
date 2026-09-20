# Agent08 调研报告：Linux 与真机工程（CANN 9.0.0 / AddRmsNormBias）

> 主题边界：工具链安装、设备识别、环境变量、构建系统、常见编译/链接/运行错误及其处置。
> 不覆盖：API 语义（Agent02）、数值测试（Agent06）、性能 tiling（Agent07）、竞赛判题失败语义（Agent09）。
> 调研日期：2026-09-11（访问日期同）。本机 macOS，无 CANN 工具链、无 NPU、无 Docker，仅做资料核对，未安装/编译。

---

## 1. 结论摘要（≤12 条）

1. **目标版本是 CANN 9.0.0**，配套驱动为 `Ascend HDK 25.7.RC1 / 26.0.RC1 / 25.5.X / 25.3.X / 25.2.X / 25.0.X` 系列；Toolkit/ops/NNAL 已解耦为独立子包，可独立升级（来源：官方 9.0.0 版本说明，等级 A）。
2. **9.0.0 安装路径与 8.x 不同**：9.0.0 默认装在 `/usr/local/Ascend/cann/`（含 `set_env.sh`），而 8.x 多在 `/usr/local/Ascend/ascend-toolkit/`。判题模板 `run.sh` 的回显提示 `source /usr/local/Ascend/ascend-toolkit/set_env.sh` 是 **8.x 路径**，在 9.0.0 上应改为 `/usr/local/Ascend/cann/set_env.sh`（等级 A/B，版本风险）。
3. **`ASCEND_HOME_PATH` 必须指向 CANN toolkit 根目录**，由 `set_env.sh` 自动设置；`ASCEND_OPP_PATH` 严禁手动 `export`，否则会漏掉其它变量导致 561107（等级 A/B）。
4. **判题机编译靠 `find_package(ASC REQUIRED)`**，它依赖 `set_env.sh` 注入的 `ASCEND_HOME_PATH`/`CMAKE_PREFIX_PATH` 来定位 `ASCConfig.cmake`；这两个变量未正确设置时直接 CMake 配置失败（等级 A）。
5. **`--npu-arch=dav-2201` 对应 Atlas A2 训练/推理（Ascend 910B1~B4、910B2C）与 Atlas A3 训练/推理（Ascend 910_93，运行时 SocVersion 映射到 ASCEND910B）**；950/A5 系列用 `--npu-arch=dav-3510`（等级 A）。
6. **工具链核心**：毕昇编译器 `ccec_compiler`（`.run` 包内含 `bisheng`，命令名 `ccec_compiler`）负责把 Ascend C 编译成 NPU 二进制；`msopgen` 生成算子工程（已集成进 toolkit，可 `pip install mindstudio_opgen` 独立更新）；`msopst` 做 ST 上板测试（等级 A/B）。
7. **V001 的 `unknown type name 'pipe_'` + `cannot use dot operator on a type` 是编译期类型不可见症状**，最可能是「代码按 8.x 写法、但判题机用 9.0.0 头文件」或「ASCEND_HOME_PATH 指错版本导致 `kernel_operator.h` 不匹配」：`TPipe` 在当前可见作用域内未定义，并非变量名本身的问题（等级 B，需 Agent02 最终确认 API 形态）。
8. **常见运行期崩溃**：AICore 超时错误码 `507014`（底层 watchdog，单次约 18 分钟），常由 `SetFlag/WaitFlag` 不匹配、越界访问、死循环引起；`507011` 表示 stream 执行失败（等级 A/B）。
9. **NPU 设备识别**：`npu-smi info` 看 `Health/Name/AICore(%)/Memory-Usage`；容器里 `docker exec` 看不到 `Product Name`、`Serial Number`；芯片名（如 910B3）**不能**判定是 A2 还是 A3，需用 `dmidecode` 或 `npu-smi info -t product` 看整机型号（等级 A）。
10. **Docker/远程调试**：宿主机装驱动+固件，容器只需映射 `/dev/davinciN`、`/dev/davinci_manager`、`/dev/devmm_svm`、`/dev/hisi_hdc` 并挂载 `/usr/local/Ascend/driver`、`/usr/local/dcmi`、`npu-smi`（等级 A）。
11. **多版本共存**：不要混用 `LD_LIBRARY_PATH`，通过 `source /usr/local/Ascend/ascend-toolkit/<ver>/set_env.sh`（或 `cann/<ver>/set_env.sh`）切换；`latest` 是软链接，锁版本用具体版本目录（等级 B/C）。
12. **9.0.0 工具链依赖**：GCC ≥ 7.3.0、CMake ≥ 3.16、Python 3.7–3.11.4（但 3.7/3.8 官方已停止维护，9.0 推荐 ≥3.9）；安装前需 `gcc make dkms linux-headers-$(uname -r)` 等（等级 A/B）。

---

## 2. 环境搭建步骤清单

### 2.1 裸机（物理机/云 ECS，有昇腾卡）

> 顺序：装 OS 依赖 → 装驱动+固件（HDK，运行态必需）→ 装 CANN Toolkit（编译+运行必需）→ 装 ops 包（运行态）→ source 环境 → 验证。

```bash
# 0) 确认架构与 OS（9.0.0 支持 aarch64 / x86_64；注意内核与 OS 兼容性）
uname -m                       # aarch64 或 x86_64
cat /etc/os-release

# 1) 安装编译/驱动源码依赖（Ubuntu/Debian 示例；openEuler/CentOS 用 yum 等价包）
sudo apt-get update
sudo apt-get install -y make dkms gcc linux-headers-$(uname -r) python3 python3-pip
# 9.0.0 推荐 gcc>=7.3.0、cmake>=3.16、python3>=3.9

# 2) 安装驱动与固件（HDK，默认路径 /usr/local/Ascend）
chmod +x Ascend-hdk-*-npu-driver_*_linux-*.run
chmod +x Ascend-hdk-*-npu-firmware_*.run
sudo ./Ascend-hdk-*-npu-driver_*_linux-*.run --full --install-for-all
sudo ./Ascend-hdk-*-npu-firmware_*.run --full
# 若非 root 安装驱动，需与 toolkit 同一运行用户组；必要时重启

# 3) 安装 CANN 9.0.0 Toolkit（合一包或单独包；--install-path 可选）
chmod +x Ascend-cann-toolkit_9.0.0_linux-*.run
sudo ./Ascend-cann-toolkit_9.0.0_linux-*.run --install --install-path=/usr/local/Ascend
# 9.0.0 新增 apt/pip/yum 在线安装：sudo apt-get install ascend-cann-toolkit=9.0.0

# 4) 安装 ops 算子包（运行态依赖，与 toolkit 同路径）
sudo ./Ascend-cann-910b-ops_9.0.0_linux-*.run --install --install-path=/usr/local/Ascend

# 5) 加载环境（9.0.0 路径）
source /usr/local/Ascend/cann/set_env.sh            # 9.0.0
# 8.x 为：source /usr/local/Ascend/ascend-toolkit/set_env.sh

# 6) 验证
npu-smi info                                          # 驱动/设备就绪
cat /usr/local/Ascend/cann/*-linux/ascend_toolkit_install.info   # toolkit 版本
which atc ccec_compiler msopgen                        # 工具可见
```

> 关键坑：9.0.0 默认目录是 `cann/`（不是 `ascend-toolkit/`）。若脚本仍写 `ascend-toolkit/set_env.sh` 会找不到文件。安装驱动必须 `--install-for-all` 或保证 toolkit 运行用户与驱动用户同组，否则出现 `Check owner failed`/`561107`。

### 2.2 容器 / 远程（有卡机器 + 容器或 SSH）

```bash
# A. 宿主机先装好驱动+固件（容器内只映射驱动，不再装驱动）

# B. 启动容器并映射 NPU 设备（以第 0 张卡为例）
docker run -it --name cann_dev --net=host --shm-size=1g \
  --device=/dev/davinci0 \
  --device=/dev/davinci_manager \
  --device=/dev/devmm_svm \
  --device=/dev/hisi_hdc \
  -v /usr/local/Ascend/driver:/usr/local/Ascend/driver:ro \
  -v /usr/local/dcmi:/usr/local/dcmi:ro \
  -v /usr/local/bin/npu-smi:/usr/local/bin/npu-smi:ro \
  -v /etc/ascend_install.info:/etc/ascend_install.info:ro \
  -v /usr/local/Ascend/driver/version.info:/usr/local/Ascend/driver/version.info:ro \
  <ascendhub-cann-image> bash

# C. 容器内再装/挂载 CANN toolkit + ops，然后
source /usr/local/Ascend/cann/set_env.sh
npu-smi info                    # 容器内应能看到映射的 davinci0
ls /dev | grep davinci

# D. 远程 SSH 到有卡机器：ssh user@host 后按裸机流程 source 环境并编译运行
```

> 多卡映射：按 `npu-smi info` 列出的 `NPU 0/1/...` 增加 `--device=/dev/davinciN`。vNPU 场景设备名是 `/dev/vdavinci100`，容器内需重命名为 `davinci100`。`--privileged` 可省，按需映射即可。

---

## 3. 环境变量与 SoC 架构名对照表

| 变量 | 作用 | 取值示例（9.0.0） | 官方出处 | 等级 |
|---|---|---|---|---|
| `ASCEND_HOME_PATH` | CANN toolkit 根目录，被 `find_package(ASC)`、`ccec_compiler`、运行时定位库/头文件 | `/usr/local/Ascend/cann` | hiascend 文档；set_env.sh（A） | A |
| `ASCEND_OPP_PATH` | 算子包（opp）路径，**由 set_env.sh 自动设置，禁止手动 export** | `/usr/local/Ascend/cann/opp` | cannbot-skills 指南（B）；hiascend FAQ（A） | A/B |
| `LD_LIBRARY_PATH` | 运行时动态库搜索路径（acl/runtime/opp vendors） | `$ASCEND_HOME_PATH/lib64:...` | set_env.sh 内容；多篇教程（B） | A/B |
| `DDK_PATH` | 设备侧（Device）开发路径，给 Host 侧算子包编译用 | `/usr/local/Ascend/ascend-toolkit/<ver>` | hiascend 文档（A）；HDK 教程（B） | A |
| `NPU_HOST_LIB` | Host 侧桩库（stub）路径，ST/单算子调用编译链接用 | `$DDK_PATH/runtime/lib64/stub` | hiascend msOpST 文档（A） | A |
| `PYTHONPATH` | ACL Python 包、TBE 实现等 | `$ASCEND_HOME_PATH/python/site-packages:...` | set_env.sh 内容（A） | A |
| `PATH` | 使 `atc`/`ccec_compiler`/`msopgen`/`npu-smi` 等可见 | `$ASCEND_HOME_PATH/bin:$ASCEND_HOME_PATH/compiler/ccec_compiler/bin:$PATH` | set_env.sh 内容（A） | A |
| `CMAKE_PREFIX_PATH` | CMake `find_package(ASC)` 的搜索基路径（set_env.sh 注入） | 含 `$ASCEND_HOME_PATH` | 由 ASCConfig.cmake 位置推断（A） | A |
| `ASCEND_GLOBAL_LOG_LEVEL` | 日志级别（1=error，3=info） | `1` | 训练营教程（C） | C |
| `TASK_QUEUE_ENABLE` / `PTCOPY_ENABLE` / `COMBINED_ENABLE` | 性能/任务队列优化开关 | `1` | 训练营教程（C） | C |
| `HCCL_CONNECT_TIMEOUT` / `HCCL_EXEC_TIMEOUT` | 集合通信超时（秒） | `1836` / `1980` | MindSpore 案例（C） | C |

> 注：`set_env.sh`（9.0.0）会一次性设置上述大部分变量；手动拼 PATH/LD_LIBRARY_PATH 容易漏项，优先 `source set_env.sh`。`ASCEND_OPP_PATH` 手动 export 是社区高频坑（→ 错误码 561107）。

---

## 4. `dav-*` 与 SoC 型号对应表（含 `dav-2201` 核对）

| `--npu-arch` | `__NPU_ARCH__` | NpuArch | SocVersion | 产品系列 / 芯片型号 | 备注 |
|---|---|---|---|---|---|
| `dav-1001` | 1001 | DAV_1001 | ASCEND910 | Atlas 训练系列（初代 Ascend 910） | 达芬奇 1.0 |
| `dav-2002` | 2002 | DAV_2002 | ASCEND310P | Atlas 推理系列（Ascend 310P1/P3） | 达芬奇 2.0 |
| `dav-2201` | 2201 | DAV_2201 | ASCEND910B | **Atlas A2 训练/推理**（910B1~B4、910B2C） | 达芬奇 2.2 增强 |
| `dav-2201` | 2201 | DAV_2201 | ASCEND910B | **Atlas A3 训练/推理**（Ascend 910_93） | 运行时 SocVersion 映射到 ASCEND910B（非独立枚举） |
| `dav-3002` | 3002 | DAV_3002 | ASCEND310B | Atlas 200I/500 A2 推理（310B1~B4） | 达芬奇 3.0 |
| `dav-3510` | 3510 | DAV_3510 | ASCEND950 | Atlas A5 训练（950DT）/ 推理（950PR） | 达芬奇 3.5（最新） |

**`dav-2201` 核对结论（高置信，等级 A）**：
- 官方 9.0.0 英文文档 `__NPU_ARCH__` 映射表明确：`Atlas A3 training/inference → 2201`、`Atlas A2 training/inference → 2201`。
- cannbot-skills 架构知识表进一步细化：`DAV_2201` 既覆盖 **A2 系列（Ascend 910B1~B4、910B2C）**，也覆盖 **A3 系列（Ascend 910_93）**；且 `Ascend910_93` 的 SocVersion 在运行时映射到 `ASCEND910B`。
- 因此判题模板 `CMakeLists.txt` 默认 `dav-2201` 对 **A2/A3（910B/910_93）** 是正确的；若判题机为 **950/A5，则必须改成 `dav-3510`**，否则生成的二进制与硬件指令集不兼容。
- 设备侧代码可用 `__NPU_ARCH__ == 2201` 做条件编译（官方示例）。

---

## 5. 错误 → 处置对照表（≥8 条）

| # | 错误文本 / 现象 | 触发场景 | 根因 | 处置 | 来源 | 等级 |
|---|---|---|---|---|---|---|
| 1 | `unknown type name 'pipe_'`；`cannot use dot operator on a type`（V001 现象） | 判题机编译学生 `kernel.asc` 报 15/15 Compile Error | `TPipe` 在当前可见头文件作用域中未定义，最可能是**代码按 8.x 写法、判题机用 9.0.0 头文件**，或 `ASCEND_HOME_PATH` 指错版本导致 `kernel_operator.h`/`AscendC` 命名空间不匹配（非变量名问题） | 确认 `ASCEND_HOME_PATH` 指向 9.0.0 的 `cann/`；核对 `TPipe`/`InitBuffer` 形态对齐 9.0.0（交 Agent02）；不要混用 8.x 样例 | V001 结果.md；8.x→9.0 头文件差异（社区）（B） | B |
| 2 | `CMake Error: Could NOT find ASC (missing: ASC_DIR)` / `find_package(ASC REQUIRED)` 失败 | `run.sh` 执行 `cmake ..` 即失败 | `set_env.sh` 未 source，或 `ASCEND_HOME_PATH` 指错/缺失，CMake 找不到 `ASCConfig.cmake` | 先 `source $ASCEND_HOME_PATH/set_env.sh`（确保指向 9.0.0）；检查 `find $ASCEND_HOME_PATH -name ASCConfig.cmake` | 判题模板 run.sh/CMakeLists；hiascend 文档（A） | A |
| 3 | `fatal error: kernel_operator.h: No such file or directory` | 编译 Ascend C 源文件 | 头文件路径未进 include（-I 缺失），通常是 env 未加载或 toolkit 版本错 | `source set_env.sh`；确认 `ls $ASCEND_HOME_PATH/include/kernel_operator.h` 存在 | 教程（B/C） | B |
| 4 | `error while loading shared libraries: libascendcl.so: cannot open shared object file` | 运行可执行文件 | `LD_LIBRARY_PATH` 未配置或配置错误 | `source set_env.sh`；或 `export LD_LIBRARY_PATH=$ASCEND_HOME_PATH/lib64:$LD_LIBRARY_PATH` | 搭建避坑指南（C） | C |
| 5 | `ModuleNotFoundError: No module named 'acl'` | Python 调用 ACL | ACL Python 包未加入 `PYTHONPATH` | `source set_env.sh`；或 `pip3 install /usr/local/Ascend/cann/*/python/site-packages/...` | 搭建避坑指南（C） | C |
| 6 | 错误码 `561107`（手动 export `ASCEND_OPP_PATH` 后） | 运行自定义算子 | 手动设置 `ASCEND_OPP_PATH` 遗漏了 set_env.sh 一并设置的其它必需变量 | 删除手动 export，改用 `source /usr/local/Ascend/cann/set_env.sh` | cannbot-skills 指南（B） | B |
| 7 | 错误码 `561003`（Kernel 查找失败） | 运行自定义算子包 | 自定义算子包库路径未加入 `LD_LIBRARY_PATH`（`opp/vendors/<vendor>/op_api/lib`） | `export LD_LIBRARY_PATH=$ASCEND_HOME_PATH/opp/vendors/$vendor/op_api/lib:$LD_LIBRARY_PATH` | cannbot-skills 指南（B） | B |
| 8 | `aclError: 507014`（`ACL_ERROR_RT_AICORE_TIMEOUT`，`timeout or trap error`） | `aclrtSynchronizeStream(WithTimeout)` 返回 | AICore 执行超时（watchdog 单次约 18 分钟）：死循环、`SetFlag/WaitFlag` 不配对、越界访问、计算量过大 | 缩小 tiling/分块；检查同步配对与地址边界；异常后等待超时自动复位（或重启）；调试时调大超时 | hiascend FAQ（A）；CSDN 卡死案例（C） | A/C |
| 9 | `aclrtSynchronizeStreamWithTimeout ... 507011` / `event wait timeout` | 同步等待任务完成 | 任务在 device 上挂死（`EVENT_WAIT` 超时），常见于核异常/同步原语错误 | 见 #8；断点调试时把超时从 3000ms 调大或改用 `aclrtSynchronizeStream`（避免调试误判） | MindSpore 案例（C）；MindStudio 文档（A） | A/C |
| 10 | `HBM: 65536/65536 MB` 满但 `npu-smi info -t proc` 无进程 | 进程被 SIGKILL/OOM 杀掉，未释放 NPU 资源 | 僵尸占用：HBM 权重未释放 | `npu-smi set -t device-reset -i N` 重置该卡（root），不影响其它卡；或重启 | asc-tools 实战手册（C） | C |
| 11 | `npu-smi` 显示卡 `Health=Error` / `dmesg | grep ascend` 有 timeout/panic | 运行期 NPU 掉线 | ECC 错误累积、驱动崩溃、PCIe 链路降速（Gen2 x8 等） | 查 `npu-smi info -t ecc`；`lspci -vv -d 1a2e:` 看 LnkSta；必要时重置/重启、检查散热与槽位 | NPU 掉线定位（C） | C |
| 12 | `Check owner failed, please check env ASCEND_TOOLKIT_HOME or ASCEND_NNAE_HOME` | 装 NNAL/多包时 | 安装器子进程校验目录 owner≠UID/GID（toolkit 用 root 装、NNAL 用其它用户） | `chown -R root:root /usr/local/Ascend` 统一 owner 后再装 | 8.2.RC1 排障（C） | C |
| 13 | `TPipe::InitBuffer` 参数个数不匹配（编译报错） | 跨 8.x 小版本升级 | 8.0.RC1→8.0.RC2 起 `InitBuffer(TBuf&, num, len)` 形参变化，旧写法 `InitBuffer(buf, len)` 才能编过 | 以目标版本官方 API 文档为准对齐形参（典型 8.x 版本风险，9.0 需 Agent02 确认） | CANN 训练营博客（C） | C |
| 14 | `error: unknown type name "DTYPE_N" / undeclared identifier "DTYPE_M"` | 自定义算子含多输入宏 | 只有 2 个输入时不会生成 `DTYPE_M`/`DTYPE_N` 宏 | 用 `#if defined(DTYPE_M) && defined(DTYPE_N)` 条件隔离 | 华为开发者联盟 FAQ（A） | A |

> 版本风险标注：#1、#3、#13 均与「8.x 写法 vs 9.0.0 头文件/API」强相关，是本次竞赛（目标 9.0.0）最高频的环境类陷阱。

---

## 6. Linux DO 命中表 与 V2EX 命中表

### 6.1 Linux DO 命中表

**结论：无命中。**

实际使用的检索式（均通过 WebSearch 执行，日期 2026-09-11）：
1. `site:linux.do 昇腾 CANN 安装 报错`
2. `site:linux.do npu-smi`
3. `linux.do 昇腾 NPU CANN 讨论`

检索结果数：上述三项检索**返回的结果均来自其它域名**（`www.hiascend.com`、`blog.csdn.net`、`support.huawei.com`、`hwcomputing.csdn.net` 等），**未返回任何 `linux.do` 域名下的帖子**。即针对 linux.do 的命中数为 **0**。

说明：可用搜索引擎对 `site:` 操作符的支持有限，未能索引/返回 linux.do 上关于「昇腾/CANN/npu-smi」的讨论帖；不排除该站相关内容较少或未被收录。未编造任何命中条目。

### 6.2 V2EX 命中表

**结论：无命中。**

实际使用的检索式（WebSearch，日期 2026-09-11）：
1. `site:v2ex.com 昇腾 CANN`
2. `site:v2ex.com npu-smi`
3. `v2ex 昇腾 NPU CANN npu-smi 讨论`

检索结果数：三项检索**返回的结果均来自其它域名**（同上，`hiascend.com`/`csdn.net`/`deepwiki.com` 等），**未返回任何 `v2ex.com` 域名下的帖子**。即针对 v2ex.com 的命中数为 **0**。

说明：同 6.1，未检索到 V2EX 上相关帖子，未编造。如需进一步确认，可人工访问 v2ex.com 站内搜索「昇腾」「CANN」「npu-smi」（需登录/人工浏览，本次未做人工登录核验）。

> 备注：两个论坛均无命中，但社区经验性结论（CSDN 等非官方博客，等级 C）已纳入第 5 节与第 9 节，并明确标注等级，不与官方文档（A）混同。

---

## 7. 判题模板 `run.sh` / `CMakeLists.txt` 的真机适配要点与潜在坑

> 已读模板文件：`run.sh`、`CMakeLists.txt`、`main.asc`、`kernel.asc`、`data_utils.h`、`scripts/`（AddRmsNormBias.py、gen_data.py、verify_result.py）。

### 7.1 `run.sh` 适配要点
- 开头强校验 `ASCEND_HOME_PATH`，未设置即退出并提示 `source /usr/local/Ascend/ascend-toolkit/set_env.sh`。**坑**：提示路径是 **8.x**；9.0.0 应为 `/usr/local/Ascend/cann/set_env.sh`。判题机若按 9.0.0 装，该提示具有误导性（但脚本实际 `source ${ASCEND_HOME_PATH}/set_env.sh`，只要变量指对即可，提示只是文案）。
- `source "${ASCEND_HOME_PATH}/set_env.sh"`：依赖变量事先正确指向 9.0.0 的 `cann/` 目录，否则后续 `cmake` 的 `find_package(ASC)` 失败（见第 5 节 #2）。
- 流程：`rm -rf build` → `cmake ..` → `make -j4` → `python3 gen_data.py` → `timeout 120 ./add_rms_norm_bias_custom` → `verify_result.py`。**真机前提**：必须有可见 NPU（`npu-smi info` 非空、`/dev/davinci*` 存在），否则 `main.asc` 里 `aclrtSetDevice`/`aclrtGetDeviceInfo(ACL_DEV_ATTR_VECTOR_CORE_NUM)` 会失败返回 1。
- `timeout 120` 是**宿主进程级**超时（120 秒）；而 `main.asc` 内部 `aclrtSynchronizeStreamWithTimeout(stream, 3000)` 是 **3 秒**流同步超时。若算子真的挂死，3 秒先触发 507014，不至于拖到 120 秒。调试时若想延长内部超时，应改 `main.asc`（但 `kernel.asc` 才是提交物，`main.asc` 是本地验证用，判题以 `kernel.asc` 入口为准）。

### 7.2 `CMakeLists.txt` 适配要点
- `find_package(ASC REQUIRED)` + `project(... LANGUAGES ASC CXX)`：标准 9.0.0 Ascend C CMake 用法（官方同款示例，等级 A）。
- `NPU_ARCH` 默认 `dav-2201`：对 A2/A3（910B/910_93）正确；**若判题机为 950/A5，必须外部传 `-DNPU_ARCH=dav-3510` 或改默认**（等级 A，见第 4 节）。
- `target_compile_options(... $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>)`：只作用于 ASC 语言源，正确。
- 链接库：`tiling_api register platform unified_dlog dl m graph_base`。这是 9.0.0 ASC 直接调用（Direct Invocation）工程典型的 Host 侧链接集；**8.x 同名工程常见链接 `ascendcl`/`acl_runtime` 等**，迁移到 9.0.0 时若照抄 8.x 的 `target_link_libraries` 会报 **undefined reference**（版本风险）。
- `add_executable(add_rms_norm_bias_custom main.asc)`：`main.asc` 以 `#include "kernel.asc"` 方式把算子实现纳进来，`kernel.asc` 内已含 `extern "C" void run_kernel(...)` 入口 —— 与判题「单文件 `kernel.asc` 含 `run_kernel`」一致。注意：**学生提交的 `kernel.asc` 必须保留 `run_kernel` 入口签名**，否则链接找不到入口。

### 7.3 其它潜在坑
- 模板默认架构 `dav-2201` 与判题机真实 SoC 必须匹配，否则编译通过但上板指令集不兼容（等级 A）。
- 若判题机是容器环境，需保证 `/dev/davinci*`、`/dev/davinci_manager`、`/dev/devmm_svm`、`/dev/hisi_hdc` 已映射，且 `/usr/local/Ascend/driver` 已挂载，否则 `aclrtSetDevice` 失败（等级 A）。
- `set -euo pipefail` + `make -j4`：任一编译/链接失败脚本立即非零退出，判题判为 Compile Error（与 V001 的 15/15 Compile Error 形态一致，说明错误发生在 `make` 阶段而非运行阶段）。

---

## 8. 与 `提交/V001` 平台报错现象（`pipe_` 冲突）的关联分析

- **现象**：15 个测试点全为 `Compile Error`，平台信息为 `unknown type name 'pipe_'; did you mean 'pipe_t'?` 与 `cannot use dot operator on a type`，定位在 `kernel.asc` 的 `pipe_` 成员声明（`TPipe pipe_;`）和 `InitBuffer` 调用（`pipe_.InitBuffer(...)`）。
- **已核对**：提交物 `提交/V001/kernel.asc` 第 255 行确有 `TPipe pipe_;`，第 33–42 行有 `pipe_.InitBuffer(...)`；模板 `kernel.asc` 本身不含 `pipe_`（仅是 stub），说明这是学生实现触发的编译错误。
- **环境/编译角度归因**（本主题职责内）：
  1. 报错文本 `unknown type name 'pipe_'` 出现在成员声明处，说明编译器把 `pipe_` 当作「类型名」而非变量 —— 根子是 **`TPipe` 这一标识符在可见作用域中不可见**（即 `AscendC::TPipe` 未被正确引入）。
  2. 因为模板 `main.asc` 已 `#include "kernel_operator.h"` 且学生代码有 `using namespace AscendC;`，能走到「解析类成员」阶段而非 `kernel_operator.h: No such file`，说明**头文件被找到了**，但其中 `TPipe` 的形态与代码预期不符 —— 这是 **8.x 写法 vs 9.0.0 头文件差异** 的典型症状（8.x 经典写法 `TPipe pipe_; pipe_.InitBuffer(...)` 在 9.0 头文件下若命名空间/声明变化即触发）。
  3. 也可能是 `ASCEND_HOME_PATH` 指向了错误版本（例如本地残留 8.x toolkit）导致 `kernel_operator.h` 内容不匹配 —— 属于**环境版本错配**。
- **与判题环境的关系**：V001 报错全部停在编译阶段，意味着判题机确实执行了 `cmake + make`（`find_package(ASC)` 成功，否则会报 CMake 错而非 C++ 类型错），即 ASCEND 工具链本身可用；问题局限于**算子代码与 9.0.0 头文件/API 不匹配**。
- **边界**：API 形态的最终确认（如 9.0.0 中 `TPipe` 是否改名、`InitBuffer` 重载签名）属于 Agent02 的 API 语义范畴，本主题只给出「环境/版本错配」这一高概率根因与验证路径，不直接改 API 结论。

---

## 9. 未确认事项与访问受限记录

1. **9.0.0 中 `TPipe`/`InitBuffer` 的精确形态**：官方 9.0.0 Ascend C API 文档需登录或站内检索，本次仅通过 8.x 文档（等级 A）与社区案例（等级 C）侧面确认存在版本差异；**最终以 9.0.0 官方 API 文档（转 Agent02）为准**。
2. **判题机真实 SoC 型号**：未知。默认 `dav-2201` 假设判题机为 A2/A3（910B/910_93）；若实为 950/A5，则 `NPU_ARCH` 应为 `dav-3510`。需向竞赛方/team-lead 确认。
3. **判题机是裸机还是容器**：影响设备节点映射与驱动挂载方式（见第 2.2、第 7.3 节）。
4. **访问受限记录**：
   - 华为企业支持站 `support.huawei.com/enterprise`：部分页面需授权/登录，本次以搜索摘要+公开片段为准（标注等级 A 的引用为公开文档页）。
   - `linux.do`、`v2ex.com`：搜索引擎未返回该站帖子（见第 6 节），未做人工登录核验，记为「无命中」而非「确认无内容」。
   - 官方 9.0.0 部分深层 API/样例仓库（如 gitcode.com/cann 子目录）需登录或站内检索，本次以可访问的公开文档与社区镜像为准。
5. **未安装/未编译**：本机无 CANN、无 NPU、无 Docker，所有结论均为资料核对，未执行任何安装/编译命令，符合任务约束。

---

## 10. 来源表

| 编号 | 来源 URL | 访问日期 | 类型/等级 |
|---|---|---|---|
| S1 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/releasenote/release-notes.md | 2026-09-11 | 官方版本说明（A） |
| S2 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/softwareinst/instg/instg_0107.html | 2026-09-11 | 官方安装指南（A） |
| S3 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/softwareinst/instg/instg_0090.html | 2026-09-11 | 官方安装指南（A） |
| S4 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/900/programug/Ascendcopdevg/atlas_ascendc_10_00039.html | 2026-09-11 | 官方 Ascend C CMake 编译（A） |
| S5 | https://www.hiascend.com/document/detail/en/CANNCommunityEdition/900/programug/Ascendcopdevg/atlas_ascendc_10_10053.html | 2026-09-11 | 官方 `__NPU_ARCH__` 映射（A） |
| S6 | https://gitcode.com/cann/asc-devkit/blob/master/examples/01_simd_cpp_api/.../matmul_basic_api/README.md | 2026-09-11 | 官方样例仓库（A） |
| S7 | https://support.huawei.com/enterprise/zh/doc/EDOC1100079287/10dcd668 | 2026-09-11 | 官方 npu-smi 命令参考（A，部分需登录） |
| S8 | https://www.hiascend.com/document/detail/zh/mindie/20RC2/quickstart/mindiesd_quickstart_0003.html | 2026-09-11 | 官方容器启动（A） |
| S9 | https://support.huawei.com/enterprise/en/doc/EDOC1100349463/52841d71 | 2026-09-11 | 官方驱动/容器挂载（A） |
| S10 | https://www.hiascend.com/document/detail/zh/canncommercial/83RC1/devaids/optool/atlasopdev_16_0026.html | 2026-09-11 | 官方 msOpGen/msOpST（A） |
| S11 | https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/81RC1alpha002/softwareinst/instg/instg_0045.html | 2026-09-11 | 官方依赖列表（A） |
| S12 | https://www.hiascend.com/document/detail/zh/mindstudio/70RC1/mscommandtoolug/mscommandug/atlasopdev_16_0020.html | 2026-09-11 | 官方 aclrtSynchronizeStreamWithTimeout（A） |
| S13 | https://developer.huawei.com/consumer/cn/doc/harmonyos-guides/cannkit-faqs-operator-development | 2026-09-11 | 官方算子 FAQ（A） |
| S14 | https://www.hiascend.com/developer/blog/details/0270187598516520036 | 2026-09-11 | 官方高频问答 FAQ（A） |
| S15 | https://blog.csdn.net/gitblog_00594/article/details/147115098 | 2026-09-11 | cannbot-skills 环境配置（B，官方社区整理） |
| S16 | https://gitcode.com/cann/asc-devkit 相关 README/quick_start | 2026-09-11 | 官方样例/工具（B/A） |
| S17 | https://deepwiki.com/mindstudio-docs/master/4.3-msopgen-operator-project-generator | 2026-09-11 | 官方工具说明镜像（B） |
| S18 | https://llamafactory.readthedocs.io/en/latest/multibackend/npu/npu_installation.html | 2026-09-11 | 第三方教程（B） |
| S19 | https://ascendai.csdn.net/692fd8352087ae0db79e92fa.html | 2026-09-11 | 训练营教程（C） |
| S20 | https://cann.csdn.net/6a473ffe10ee7a33f28753b5.html | 2026-09-11 | 社区环境变量帖（C） |
| S21 | https://blog.csdn.net/futao1229/article/details/144127705 | 2026-09-11 | 社区 AICore 超时案例（C） |
| S22 | https://hwcomputing.csdn.net/6a140cc0662f9a54cb76eea9.html | 2026-09-11 | 社区 NPU 掉线/内存失败（C） |
| S23 | https://blog.csdn.net/2501_94186029/article/details/161311386 | 2026-09-11 | 社区 asc-tools 手册（C） |
| S24 | https://2048ai.net/6892c01f080e555a88d5a565.html | 2026-09-11 | 社区 8.2.RC1 排障（C） |
| S25 | http://www.chinadongda.com/j/?weixin_45510013/article/details/140037862 | 2026-09-11 | 社区 TPipe InitBuffer 版本差异（C） |
| S26 | https://discuss.mindspore.cn/t/topic/1156/1 | 2026-09-11 | 社区 HCCL 超时案例（C） |
| S27 | 本地：`提交/V001/结果.md`、`提交/V001/kernel.asc`、`Downloads/.../run.sh|CMakeLists.txt|main.asc|kernel.asc|data_utils.h` | 2026-09-11 | 任务给定材料（A-本地） |

> 等级约定：A=官方文档/官方仓库；B=官方教程/官方样例；C=社区帖子/博客；D=仅搜索摘要（本报告未使用 D 级作为主结论，社区结论均标 C）。

---

*本报告仅覆盖「Linux 与真机工程」主题，API 语义细节（如 9.0.0 `TPipe` 精确形态）已标注转 Agent02 确认。*
