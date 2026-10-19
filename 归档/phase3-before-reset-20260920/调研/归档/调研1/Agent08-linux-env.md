# Agent08 · Linux/环境/真机工程调研报告（CANN 9.0.0 · AddRmsNormBias 直调模板）

> 调研日期：2026-09-11（Asia/Shanghai）
> 角色：Linux/环境/真机工程调研（Agent 8）
> 目标：为「在 Linux 真机安装 CANN 9.0.0 → 编译直调模板 → 跑单算子测试 → 排查错误」准备一份可操作手册。
> 约束：本机为 macOS，无 CANN、无 NPU，**以下所有命令与结论均未在本机验证**，全部为公开资料整理（证据等级 A=官方文档 / B=官方工具与仓库 / C=社区帖子 / D=摘要未核验）。
> 关联模板：`/Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/`（run.sh / CMakeLists.txt / main.asc / kernel.asc / scripts/*）

---

## 0. 模板工程在真机上的编译/运行链路（先理解目标）

模板 `run.sh` 在真机上的完整动作（据 run.sh 源码逐行确认）：

```bash
[1/4] 检查 ${ASCEND_HOME_PATH} 非空 → 报错提示 source /usr/local/Ascend/ascend-toolkit/set_env.sh
[2/4] source "${ASCEND_HOME_PATH}/set_env.sh" → rm -rf build; mkdir build; cd build
      cmake ..            # 依赖 find_package(ASC REQUIRED)
      make -j4            # ASC 语言编译（--npu-arch=${SOC_ARCH}，默认 dav-2201）
[3/4] python3 ../scripts/gen_data.py    # 生成 case0 输入
[4/4] 拷贝 input/case0 与 output/golden_case0 → 运行 ./add_rms_norm_bias_custom
      → python3 ../scripts/verify_result.py 0
```

关键编译事实（模板 CMakeLists.txt 与 CANN 官方 asc-devkit CMake 指南一致）：
- `find_package(ASC REQUIRED)`：查找并配置 Ascend C（ASC 语言）编译工具链的 CMake 包（B）。
- `project(... LANGUAGES ASC CXX)`：ASC = 由毕昇编译器（bisheng）编译的 Ascend C 语言；.asc 文件默认按 C++17 编译（无需显式 `--std=c++17`）（B）。
- `--npu-arch=${SOC_ARCH}` 默认 `dav-2201`：`dav-` 后为 NPU 架构版本号；**910B（Atlas A2 训练/推理系列）对应 2201**（B/A）。
- 链接库 `tiling_api register platform unified_dlog dl m graph_base`：与官方 asc-devkit 文档"高阶 API 必须链接 libtiling_api.a、libregister.so、libgraph_base.so、libplatform.so"一致（B）。
- `-DNPU_ARCH=xxx` 可覆盖默认 SoC（CMakeLists 中 `if(DEFINED NPU_ARCH) set(SOC_ARCH ${NPU_ARCH})`），真机 SoC 与模板不符时可这样指定。

---

## 1. Linux DO 真实命中清单（≥2 条，全部含链接，未编造）

| # | 帖子标题 | 作者 | 日期 | 链接 | 与真机环境的相关点 |
|---|---|---|---|---|---|
| L1 | 内网ARM服务器部署Qwen3.5 122B模型实录（8×昇腾910B4，NPU驱动25.2.3，ARM，内网离线） | North_warm | 2026-04-06 | https://linux.do/t/topic/1904864 | 真机前置检查（uname -m、npu-smi info）、docker 设备挂载（/dev/davinci0-7、davinci_manager、devmm_svm、hisi_hdc）、驱动目录挂载（/usr/local/Ascend/driver:ro）、环境变量 SOC_VERSION=ascend910b4、LD_LIBRARY_PATH 指向 driver/lib64 |
| L2 | 昇腾910B本地部署DeepSeek-V4-Flash(w8a8量化版)测试 | tan90 | 2026-04-24 | https://linux.do/t/topic/2047891 | 910B 真机部署实测（73 赞），10 并发 290~480 tokens/s；侧面印证 910B 是社区最常用真机平台 |
| L3 | 【cann】昇腾算子怎么这么难？（QuantBatchMatmulV3 K-C 量化性能优化跑了 370 次未过平台 case） | qwx | 2026-06-16 | https://linux.do/t/topic/2418153 | 昇腾算子竞赛排障讨论：平台 case 反复不过的排查经验、官方 deveco code/agent 工具推荐（C 级佐证"竞赛类算子开发在真机上的坑非常多"） |
| L4 | 想咨询一下使用华为昇腾搭建算力中心的问题（100 卡规模） | duelist | 2026-06-23 | https://linux.do/t/topic/2461391 | 昇腾真机采购/部署成本与可行性讨论（36 回复），佐证昇腾算力中心多为 Atlas 910B/910C 一体机 |

> 检索方式说明：Linux DO 搜索需登录、搜索引擎对 linux.do 索引不全，最终通过 Bing `site:linux.do 昇腾` 命中并逐一抓取帖子页确认作者/日期/内容。以上 4 条均为真实帖子。

---

## 2. V2EX 真实命中清单（≥2 条，全部含链接，未编造）

| # | 帖子标题 | 作者 | 日期 | 链接 | 与真机环境的相关点 |
|---|---|---|---|---|---|
| V1 | 国产显卡有没有好的安装驱动"姿势"（云平台 zstack + 昇腾 Atlas 300I Duo） | couture | 2025-12-01 | https://v2ex.com/t/1176018 | 昇腾驱动安装极难：文档未列出的内核需反编译才能装上驱动；回复共识：按官方兼容性对照表装、固定内核版本、防止 yum/apt 自动升级内核（dododada），非支持列表内核基本无解（jim9606、nigga） |
| V2 | 华为欧拉跟 centos 的差异？（昇腾 910B/810 只能 docker 里装 Ubuntu 玩） | joetao123 | 2024-11-07 | https://v2ex.com/t/1087530 | 欧拉 host 上昇腾生态受限，社区普遍做法：host 装驱动 + Docker 里跑 Ubuntu/CANN 容器（frayesshi1、jlkm2010） |
| V3 | 中国的算力缺口这么大嘛？看到 2025 华为昇腾出货 81 万块 | lynn1su | 2026-04-22 | https://v2ex.com/t/1207741 | 昇腾可用性/生态讨论：软件生态（算子适配）是最大短板（cvbnt），适配算子工作量巨大（skuuhui） |
| V4 | DeepSeek V4 终于出来了（昇腾首发） | tianjiyao | 2026-04-24 | https://v2ex.com/t/1208225 | 佐证昇腾生态近期（2026）加速适配，CANN/昇腾真机资源供给增加 |

---

## 3. CANN 安装 / 多版本共存 / Docker / 远程 NPU

### 3.1 版本事实（A/B 级）

- CANN 9.0.0 配套 Ascend HDK：**Ascend HDK 26.0.RC1 / 25.5.2 / 25.5.1**（B：cann/release-management 版本说明）。安装前先确认宿主机驱动版本，驱动/固件/CANN 三者必须按配套表匹配（CANN 9.0.0 对应 HDK 26.0.RC1 这一档）。
- 安装包（社区版，910B / Atlas A2）：
  - `Ascend-cann-toolkit_9.0.0_linux-aarch64.run`（或 x86_64 变体）
  - `Ascend-cann-910b-ops_9.0.0_linux-aarch64.run`（910C/A3 用 `Ascend-cann-A3-ops_*`）
  - 下载入口：昇腾社区资源下载中心 https://www.hiascend.com/developer/download/community（需登录后按产品型号筛选，`cann=9.0.0` 直达链接形如 `...result?module=cann&cann=9.0.0`）。
- 官方镜像中心：ascendhub（https://www.hiascend.com/developer/ascendhub ），CANN 基础/开发镜像 tag 规范 `<cann版本>-<芯片系列>-<操作系统>-<python版本>[-devel]`，如 `9.0.0-910b-ubuntu22.04-py3.10`、`9.1.0-910b-openeuler24.03-py3.12-devel`（A/B）。
- 直调/算子工程开发所需的最小包是 **toolkit（编译+运行）** + **ops（运行态算子库，由 set_env.sh 自动配置 ASCEND_OPP_PATH）**；本模板是直调工程（不依赖框架层），装 toolkit + 910b-ops 即可（B）。

### 3.2 安装步骤（A 级：官方安装指南；C 级：社区实操）

```bash
# 1) 先装 NPU 驱动与固件（首次安装：驱动 → 固件；覆盖安装/升级：固件 → 驱动）
#    需要 root；需先建驱动运行用户 HwHiAiUser（uid/gid=1000）
useradd HwHiAiUser          # 缺省会报“创建 uid 和 gid 为 1000 的驱动运行用户 HwHiAiUser 失败”
# 依赖（Ubuntu/Debian 系）：
apt-get update && apt-get install -y dkms gcc linux-headers-$(uname -r) pciutils net-tools
# 驱动/固件包（名称随 HDK 版本）：
chmod +x Ascend-hdk-910b-npu-driver_*.run && ./Ascend-hdk-910b-npu-driver_*.run --full --install-for-all
chmod +x Ascend-hdk-910b-npu-firmware_*.run && ./Ascend-hdk-910b-npu-firmware_*.run --full
# 安装后必须 reboot，然后 npu-smi info 应能列出设备

# 2) 装 CANN（先 toolkit 再 ops，顺序不能反）
chmod +x Ascend-cann-toolkit_9.0.0_linux-<arch>.run
./Ascend-cann-toolkit_9.0.0_linux-<arch>.run --check          # 校验 SHA256
yes | ./Ascend-cann-toolkit_9.0.0_linux-<arch>.run --install  # 默认 root:/usr/local/Ascend，非 root:${HOME}/Ascend
# 可选自定义路径：--install-path=/home/xxx/Ascend/cann-9.0.0
chmod +x Ascend-cann-910b-ops_9.0.0_linux-<arch>.run
yes | ./Ascend-cann-910b-ops_9.0.0_linux-<arch>.run --install

# 3) 激活环境
source /usr/local/Ascend/ascend-toolkit/set_env.sh   # 或自定义路径对应的 set_env.sh
# 验证：which atc；echo $ASCEND_HOME_PATH；cat /usr/local/Ascend/ascend-toolkit/latest/<arch>-linux/ascend_toolkit_install.info
```

### 3.3 卸载（A 级）

```bash
# 卸载顺序与安装相反：先 CANN，再固件，最后驱动
cd /usr/local/Ascend/ascend-toolkit/<version> && bash cann_uninstall.sh   # 或 ./Ascend-cann-toolkit_*.run --uninstall
cd /usr/local/Ascend/firmware/script && bash uninstall.sh                # 先固件
cd /usr/local/Ascend/driver/script && bash uninstall.sh                  # 后驱动
```

### 3.4 多版本共存与切换（B 级 + C 级实践）

- 默认安装布局：`/usr/local/Ascend/ascend-toolkit/latest` 是指向当前版本的软链；`/usr/local/Ascend/ascend-toolkit/<version>` 是各版本实体目录。**共存 = 用 `--install-path` 装到不同目录，或用默认布局保留多个 `<version>` 目录，切换靠 `source 对应目录/set_env.sh`**（A 级官方安装指南 + C 级 CSDN 实操）。
- 关键：**切换版本必须在每个新 shell 里重新 `source` 目标版本的 set_env.sh**；`ASCEND_HOME_PATH` 由 set_env.sh 写入，模板 run.sh 依赖它。
- 社区踩坑（C）：CANN 8.2.RC1 及以下不支持 `--npu-arch`（报 `unsupported option '--npu-arch=...'`），8.3.RC1 起支持——即**本模板必须用 ≥8.3（推荐 9.0.0）的 CANN 编译**（A 级案例：hiascend 开发者博客「CANN版本问题导致bisheng编译报错」）。
- 宏/API 版本差异提醒（C 级）：CANN 8.0 前后 Ascend C 部分接口有变更（如 TilingContext `GetDeviceInfo→GetChipInfo`、`__NPU_ARCH__`/Kernel 语法演进）；8.x 的老 Ascend C（kernel_operator.h/TPipe/TQue）与 9.x 的 ASC 语言（.asc、`__global__ __vector__`、`find_package(ASC)`）不同，模板是 9.x 风格，勿用 8.x 教程逐字对照。

### 3.5 Docker 容器与远程 NPU（A/B 级 + 社区帖）

- 容器内不需要重装驱动：挂载宿主机驱动与设备节点即可。
- 官方推荐挂载（综合 ascendhub 官方文档与社区实操，命令细节一致）：
```bash
docker run -it --name cann_container --device /dev/davinci0 \
  --device /dev/davinci_manager --device /dev/devmm_svm --device /dev/hisi_hdc \
  -v /usr/local/dcmi:/usr/local/dcmi \
  -v /usr/local/bin/npu-smi:/usr/local/bin/npu-smi \
  -v /usr/local/Ascend/driver/lib64/:/usr/local/Ascend/driver/lib64/ \
  -v /usr/local/Ascend/driver/version.info:/usr/local/Ascend/driver/version.info \
  -v /etc/ascend_install.info:/etc/ascend_install.info \
  <ascendhub 镜像> bash
# 多卡追加 --device /dev/davinciN；部分方案还挂 /usr/local/Ascend/add-ons、/usr/local/sbin、/etc/hccn.conf
```
- 容器内环境变量：`source /usr/local/Ascend/ascend-toolkit/set_env.sh`（若镜像自带 CANN），并在容器内再跑 `npu-smi info` 验证设备可见（V1/V2 帖佐证：容器内不能用 npu-smi 多半是没挂 `/usr/local/bin/npu-smi` 与 driver 目录）。
- 远程 NPU / 免费真机渠道（C 级）：GitCode Notebook（1×910B 免费额度，镜像 euler2.9-py38-torch2.1.0-cann8.0 等）、OpenI 启智（910/910B）、昇思大模型平台 xihe、华为云 ModelArts；CANNLab/HiDevLab 云 WebIDE（CANN 算子开发专用，预装 CANN 9.x 与算子插件）。这些可作为「没有本地真机」时的替代跑通渠道（注意：竞赛判题环境不等于这些免费环境，需以判题平台公告为准）。
- 内核版本风险（C 级，V1 帖共识 + 官方 FAQ）：驱动与内核强绑定，**装驱动前固定内核版本、关闭自动升级**（`apt-mark hold linux-image-*` / `yum versionlock`），否则 `yum update` 后驱动失效。

---

## 4. 驱动与设备识别 / 环境变量

### 4.1 设备识别（A 级 + C 级）

```bash
npu-smi info                 # 核心命令：列出每张卡（Chip/Hw）健康状态 Health、温度、HBM 内存、芯片型号、驱动版本
npu-smi info -t board -i 0   # 查看单卡板级信息（含 Software/Firmware Version）
lspci | grep -i ascend       # 910B 系列 PCI vendor:device = 19e5:xxxx（如 19e5:d802 为 Atlas A2/910B）
cat /usr/local/Ascend/driver/version.info    # 驱动版本（如 Version=25.2.x + compatible_version）
cat /usr/local/Ascend/firmware/version.info  # 固件版本（含 compatible_version_drv）
dmesg | grep -i -E "ascend|davinci|19e5"     # 内核日志排查设备枚举/驱动加载
ls -l /dev/davinci*          # 设备节点：/dev/davinci0..N、/dev/davinci_manager、/dev/devmm_svm、/dev/hisi_hdc
```
- 权限模型：驱动创建运行用户 **HwHiAiUser（uid/gid=1000）**；/dev/davinci* 归属该用户/组，普通用户需加入 HwHiAiUser 组或用 root 运行（A 级安装指南 + C 级实操）。
- 常见识别问题（C 级 FAQ）：
  - `npu-smi: command not found` → 驱动未装或 PATH 未配；`/usr/local/bin/npu-smi` 是驱动安装生成的软链。
  - `npu-smi info` 报 `DrvMngGetConsoleLogLevel failed / dcmi 初始化错误(-8005)` → 驱动与固件版本不配套，按「先固件后驱动」重装。
  - lspci 显示 `Unknown device 19e5:...` → 未装驱动/内核过旧/BIOS 未使能 PCIe（C 级 ask.csdn 分析，供参考）。

### 4.2 环境变量作用表（A/B 级为主）

| 变量 | 来源/配置方式 | 作用 | 必需场景 |
|---|---|---|---|
| `ASCEND_HOME_PATH` | `source ${CANN}/set_env.sh` 写入 | CANN toolkit 根目录；编译/运行/模板 run.sh 判定的基准 | 编译+运行 |
| `ASCEND_OPP_PATH` | 由 set_env.sh 自动设置（装 ops 后） | 算子库（OPP）路径 | 仅运行 |
| `DDK_PATH` | 手动 export | 开发包路径，编译时按 `${DDK_PATH}/runtime/include/acl` 找 acl.h | 编译（旧式样例/部分工程） |
| `NPU_HOST_LIB` | 手动 export | Host 链接库路径：`${DDK_PATH}/runtime/lib64/stub` 或 `${INSTALL_DIR}/{arch-os}/devlib` | 编译链接（msopst/样例） |
| `ASCEND_CANN_PACKAGE_PATH` | 写入算子工程 `CMakePresets.json` | 算子工程（msopgen 生成的 build.sh 工程）编译时定位 CANN 包 | 编译（msopgen 工程） |
| `LD_LIBRARY_PATH` | set_env.sh + 手动追加 | 动态库搜索路径（含 `${ASCEND_HOME_PATH}/lib64`、driver/lib64、自定义算子 `${ASCEND_HOME_PATH}/opp/vendors/<vendor>/op_api/lib`） | 编译+运行 |
| `ASCEND_HOME` / `PYTHONPATH` | 手动 export（C 级实操） | 老式写法：toolkit 根路径 / `${ASCEND_HOME}/python/site-packages` | 运行 |
| `ASCEND_GLOBAL_LOG_LEVEL` | 手动 export | CANN 日志级别：0=debug 1=info 2=warning 3=error | 排障 |
| `ASCEND_SLOG_PRINT_TO_STDOUT=1` | 手动 export | 日志打屏，排障必开 | 排障 |
| `ASCEND_RT_VISIBLE_DEVICES` | 手动 export | 指定可见设备（如 `=0`），容器/多卡场景 | 运行 |
| `SOC_VERSION` | 手动 export | vLLM 等上层组件指定芯片型号（如 ascend910b4） | 上层框架 |

> 注意（B 级，cannbot-skills 官方配置指南）：**不要手动 export ASCEND_OPP_PATH**，应 `source set_env.sh`，否则会漏配其它变量导致错误 561107；自定义算子包安装后还需把 `opp/vendors/${vendor_name}/op_api/lib` 追加进 LD_LIBRARY_PATH，否则运行报 561003（Kernel 查找失败）。

---

## 5. 编译链

### 5.1 编译器与参数（A/B 级）

- 编译器：**bisheng（毕昇编译器）**，随 CANN toolkit 安装，位于 `${ASCEND_HOME_PATH}/compiler/`；老文档中 `ccec`（`ccec_compiler/bin`）是 8.x 及更早的 AI Core 编译器，9.x 直调工程统一走 bisheng + ASC 语言。
- 基本命令：
```bash
# 直调单文件：host+device 异构混合编译
bisheng hello_world.asc --npu-arch=dav-2201 -o demo
# 分离编译：.asc 编译为 .o，再链接
bisheng -c add_kernel.asc -o add_kernel.o --npu-arch=dav-2201
bisheng -c main.cpp -o main.o -I${INSTALL_DIR}/include
bisheng add_kernel.o main.o -o main
```
- `--npu-arch`（B 级官方文档，8.3.RC1+ 支持；本模板必须用它）取值 `dav-<架构版本号>`：
  - Atlas A2/A3 训练与推理系列（含 910B）：`dav-2201`
  - Atlas 200I/500 A2 推理：`dav-3002`
  - Atlas 推理系列（310P 等）：`dav-2002`
  - Atlas 训练系列（910/910A）：`dav-1001`
  - 910B 的 `__NPU_ARCH__` = 2201（A 级官方文档）。
- `--npu-soc`：可选，指定 SoC 型号（如 ascend910b1/b2/b3/b4），与 --npu-arch 同用优先 arch（B）。
- 老式 ccec 的 `-c ai_core-<soc>` 命名（如 `ai_core-Ascend910B1`、`ai_core-ascend910b`）仍见于 msopgen/老样例；**ASC 语言（bisheng）用 `--npu-arch`，两者等价目标不同写法**（B 级官方 + C 级 OpenI 实操帖：`bisheng -O2 --cce-soc-version=Ascend910B1 ...` 为旧式示例）。
- `.asc` 文件默认 C++17（asc 后缀 std 默认为 c++17）；`-x asc/cce/aicpu` 显式指定语言；`-std=c++17` 对老 `.cpp` 才需要显式加（B 级）。

### 5.2 CMake（find_package(ASC)）（B 级官方）

```cmake
cmake_minimum_required(VERSION 3.16)
find_package(ASC REQUIRED)            # 查找 Ascend C 编译工具链（模板第 3 行）
project(demo LANGUAGES ASC CXX)       # ASC 语言支持
add_executable(demo main.asc kernel.asc ...)
target_compile_options(demo PRIVATE
    $<$<COMPILE_LANGUAGE:ASC>:--npu-arch=dav-2201>)   # 仅对 ASC 语言生效
```
- 模板 CMakeLists 与官方示例逐项一致（tiling_api/register/platform/unified_dlog/graph_base 均为官方列出的高阶 API 依赖库；`dl`、`m` 为系统库）。若报找不到 ASCConfig.cmake/ASC 包，多半是**没 source set_env.sh（ASCEND_HOME_PATH 未设置）或 CANN 版本过旧**。
- 用 `-DNPU_ARCH=<dav-xxxx>` 覆盖模板默认 SoC。

### 5.3 常见编译/链接错误速查（A/B + C 级）

| 报错 | 可能原因 | 处置 |
|---|---|---|
| `bisheng: error: unsupported option '--npu-arch=...'` | CANN ≤8.2.RC1 不支持该选项 | 升级 CANN ≥8.3（推荐 9.0.0） |
| `fatal error: kernel_operator.h: No such file or directory` / `acl/acl.h` 找不到 | 未 source set_env.sh；DDK_PATH/include 路径不对 | `source ${ASCEND_HOME_PATH}/set_env.sh`；确认 `${ASCEND_HOME_PATH}/include` 与 `runtime/include` 存在 |
| `cannot find -lascendcl` / `-ltiling_api` / `-lregister` | NPU_HOST_LIB 未设或指错；lib 目录不在链接路径 | `export NPU_HOST_LIB=${ASCEND_HOME_PATH}/lib64/stub`（同架构）或 `${ASCEND_HOME_PATH}/<arch>-linux/devlib`；或 `-L${ASCEND_HOME_PATH}/lib64` |
| `find_package(ASC) ... not found` | 环境未激活 / CMake 找不到 ASC 工具链 | source set_env.sh；确认 `ASCEND_HOME_PATH` 指向 9.x toolkit（含 cmake/ASC 相关 .cmake） |
| ABI/版本不匹配、`atc` 版本不对 | toolkit 与 ops 版本不一致 | 统一到 9.0.0；`cat ascend_toolkit_install.info` 核对 |
| 链接时缺 `unified_dlog`/`graph_base` 等 | 直调工程漏链高阶 API 库 | 模板已链，勿删；若自建工程按 asc-devkit 官方链接表补齐 |
| E80000/E80012 等 E5~EB 编译错误 | TBE/编译前后端校验报错（参数非法、维度过高） | 按错误码查昇腾文档「故障处理」章节 |

---

## 6. 运行期故障排查表（症状 / 可能原因 / 排查命令 / 参考）

> 通用排障三连（A/B 级官方 FAQ）：先开日志打屏再复现：
> ```bash
> export ASCEND_GLOBAL_LOG_LEVEL=1
> export ASCEND_SLOG_PRINT_TO_STDOUT=1
> ```
> 然后 `grep -rn "errorStr" /var/log/npu/slog` 定位真正失败指令。

| 症状 | 可能原因 | 排查命令/手段 | 参考 |
|---|---|---|---|
| `aclrtSynchronizeStreamWithTimeout` 超时（模板 main.asc 第 94 行，ret≠0） | kernel 在 NPU 上执行失败/挂起：UB 越界、地址未对齐、死锁（多核同步等待）、SoC 型号错配导致指令不支持 | 开日志（ASCEND_SLOG_PRINT_TO_STDOUT=1）；`grep -rn errorStr /var/log/npu/slog`；`grep -i "fault kernel_name"` 日志；核对 `--npu-arch` 与真机 SoC（`npu-smi info` 芯片型号）；先跑通 block=1 单核 | hiascend 官方论坛 S2 赛季 FAQ；官方博客「ACL stream synchronize failed」 |
| `Synchronize stream failed. error code is 507015 / 507035` | 507015≈UB 地址分配/切分问题（MPU address access is invalid）；507035≈流同步超时/设备异常；常见：GM/UB 越界、shape 给错 | 日志 grep errorStr；printf 打印 tiling 与地址偏移；CPU 侧孪生调试定位行号；检查初始化/内存大小是否匹配 | 官方论坛 FAQ Q3/Q5 |
| `errorStr: When the D-cache reads and writes data to the UB, the response value returned by the bus is a non-zero value` | GM 或 UB 越界访问（D 非 32 倍数尾块处理错误、跨行覆盖） | 在 CPU 侧打印各使用地址值判断越界；核对 DataCopy 长度/对齐；尾块用 DataCopyPad 并确认不越界 | 官方论坛 FAQ（A9） |
| `errorStr: instruction address misalign(ADDR_MISALIGN)` / `The UB address accessed by the VEC instruction is not aligned` | 地址未满足对齐约束 | 检查 API 对齐要求；printf 输出各 API 地址偏移；CPU 孪生调试 | 官方论坛 FAQ Q5 |
| `aclrtFree: free device memory failed 507899` | 输出内存被 kernel 越界写坏或重复释放 | 检查各输出内存使用；printf 打印地址；CPU 孪生调试 | 官方论坛 FAQ Q4 |
| kernel 结果全错/部分错（verify_result 失败） | 精度问题或数据错误：同步缺失、tiling 错误、尾块边界、dtype 不符 | verify_result.py 放宽/分析误差来源；printf 关键中间值；DumpTensor；检查是否缺同步（非范式代码需手动插同步） | 官方论坛 FAQ Q1 |
| `DataCopyPad`/某指令编译或运行不支持 | SoC 型号错配（编译用 dav-xxx 与真机 SoC 不符）或该 SoC 指令集限制 | `npu-smi info` 查芯片型号 → 核对 `--npu-arch` 映射；换 `-DNPU_ARCH=` 重新编译；查询目标 SoC 指令支持表 | A 级文档（npu-arch 对照表） |
| 多核结果互踩/死锁 | 多核（block>1）时 GM 地址划分错误、同步缺失、跨核覆盖相邻行 | 单核先跑通；打印 block_idx 与各核地址区间；检查 `availableCoreNum` 与 launch blockNum 是否超出 | 官方论坛 FAQ Q2；模板 main.asc 用 `aclrtGetDeviceInfo(ACL_DEV_ATTR_VECTOR_CORE_NUM)` 取核数 |
| OOM / 设备内存不足（aclrtMalloc 失败） | HBM 被占满（残留进程）、分配过大 | `npu-smi info` 看 HBM 占用；`ps -ef | grep acl`/`npu-smi info -t process` 找残留进程 kill；检查 aclrtMalloc 大小是否异常 | C 级实操 |
| 容器内 `npu-smi` 报错/看不到卡 | 容器未挂载设备节点或驱动目录 | 按 3.5 节补齐 `--device` 与 `-v` 挂载；容器内 `ls /dev/davinci*` | V2 帖 + B 级容器指南 |
| 驱动安装报 `The current installation package and environment is not match` / dkms 编译失败 | 内核版本不在支持列表 / 缺 gcc、kernel-headers | `uname -r` 对照官方 OS-内核兼容表；装 `dkms gcc kernel-headers-$(uname -r)`；固定内核版本 | 官方安装指南附录 C 故障表 |

---

## 7. 真机操作清单（从零到跑通模板 run.sh）

> 前提：有一台装了昇腾卡（Atlas A2/910B 系列）的 Linux 服务器（x86_64 或 aarch64），root 权限；操作系统在 CANN 9.0.0 支持列表内（Ubuntu 20.04/22.04、openEuler 20.03/22.03、Kylin V10 等）。

1. **确认硬件与内核**：`lspci | grep -i ascend`、`uname -r`、`cat /etc/os-release`，与官方「CANN 9.0.0 配套表（Ascend HDK 26.0.RC1/25.5.2/25.5.1）+ OS 内核兼容表」核对；**固定内核版本，关闭自动升级**。
2. **装依赖**：Ubuntu 系 `apt-get install -y gcc g++ make cmake zlib1g zlib1g-dev openssl libsqlite3-dev libssl-dev libffi-dev unzip pciutils net-tools libblas-dev gfortran libblas3 dkms linux-headers-$(uname -r)`；openEuler/CentOS 系对应 yum 包；Python 3.7.5~3.11.x 与 pip3。
3. **建驱动用户**：`useradd HwHiAiUser`（uid/gid=1000，驱动要求）。
4. **装驱动固件**：先驱动后固件（首次），`--full --install-for-all` 与 `--full`；**reboot**；`npu-smi info` 确认设备在位、Health OK；记录驱动版本。
5. **装 CANN 9.0.0**：`Ascend-cann-toolkit_9.0.0_linux-<arch>.run --check && --install`，再装 `Ascend-cann-910b-ops_9.0.0_linux-<arch>.run --install`。
6. **激活环境（每个新 shell 都要做）**：`source /usr/local/Ascend/ascend-toolkit/set_env.sh`；验证 `echo $ASCEND_HOME_PATH`、`which bisheng`、`atc` 版本为 9.0.0。
7. **核对 SoC**：`npu-smi info` 看芯片型号 → 映射 `--npu-arch`（910B/A2 = `dav-2201`）；若与模板默认不一致，编译时加 `-DNPU_ARCH=<dav-xxxx>`。
8. **上传模板并跑**：
   ```bash
   scp -r addrmsnormbias_problem_1742_template user@host:~/ && ssh user@host
   source /usr/local/Ascend/ascend-toolkit/set_env.sh   # 先 export ASCEND_HOME_PATH
   cd ~/addrmsnormbias_problem_1742_template && bash run.sh
   # 期望输出：[1/4]→[2/4] cmake/make 通过 →[3/4] 生成数据 →[4/4] ./add_rms_norm_bias_custom 运行 → verify_result.py 0 → "=== PASSED ==="
   ```
9. **失败排查**：按第 6 节表逐项查（先开 `ASCEND_SLOG_PRINT_TO_STDOUT=1` 复现，再 `grep errorStr /var/log/npu/slog`）。
10. **多 case 验证**（真机可选）：确认 input/caseN、output/golden_caseN 存在后，可逐个 case 跑二进制再 `verify_result.py N`（模板 run.sh 默认只跑 case0；15 个测试点的完整验证以判题平台为准）。
11. **容器化替代路径**（如果服务器只给 Docker）：拉 ascendhub `9.0.0-910b-*` 镜像，按 3.5 节挂载设备与驱动，容器内 `source set_env.sh` 后同样执行 run.sh。

---

## 8. 本轮新增来源清单（证据等级 + 是否本机验证）

> 全部标注「是否在本机验证：否」（本机 macOS 无 CANN/NPU，未执行任何安装/编译/运行）。

**社区帖子（已在第 1、2 节列出，这里给原始链接汇总）**
- Linux DO：1904864 / 2047891 / 2418153 / 2461391（C，未验证）
- V2EX：1176018 / 1087530 / 1207741 / 1208225（C，未验证）

**官方文档/工具（A/B，未验证）**
1. 昇腾社区《安装CANN软件包》https://www.hiascend.com/document/detail/zh/canncommercial/800/softwareinst/instg/instg_0008.html （A）
2. CANN 8.0.RC2.2《软件安装指南》（含驱动固件安装/容器场景/故障处理附录）https://www.hiascend.com/doc_center/source/zh/canncommercial/80RC22/softwareinst/instg/CANN%208.0.RC2.2%20%E8%BD%AF%E4%BB%B6%E5%AE%89%E8%A3%85%E6%8C%87%E5%8D%97%2001.pdf （A）
3. CANN 社区版 8.5.0.alpha002《软件安装指南》https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/850alpha002/softwareinst/instg/...pdf （A）
4. 官方安装文档《安装NPU驱动固件》（首次驱动>固件、覆盖固件>驱动）https://www.hiascend.com/doc_center/source/zh/CANNCommunityEdition/800alpha001/softwareinst/instg/instg_0005.html （A）
5. asc.gitcode.com 官方指南：《AI Core算子编译基本用法》https://asc.gitcode.com/guide/编程指南/编译与运行/算子编译/AI-Core算子编译基本用法.html （B）
6. asc.gitcode.com 官方指南：《AI CPU算子编译》https://asc.gitcode.com/guide/编程指南/编译与运行/算子编译/AI-CPU算子编译基本用法.html （B）
7. asc.gitcode.com《SIMD BuiltIn关键字》（__NPU_ARCH__ 说明）https://asc.gitcode.com/guide/编程指南/语言扩展层/SIMD-BuiltIn关键字.html （B）
8. hiascend.com Ascend C 主页（.asc、<<<>>>直调、孪生调试）https://www.hiascend.com/cann/ascend-c （A）
9. CANN 发布管理仓库（gitcode.com/cann/release-management）9.1.0 版本说明（**CANN 9.0.0 ↔ Ascend HDK 26.0.RC1/25.5.2/25.5.1 配套表**）https://gitcode.com/cann/release-management （B）
10. ascendhub CANN 镜像详情（tag 规范与 Dockerfile）https://www.hiascend.com/developer/ascendhub/detail/17da20d1c2b6493cb38765adeba85884 （A/B）
11. CANN 官方 msopgen/msopst 文档《Ascend C自定义算子开发实践》https://www.hiascend.com/document/detail/zh/canncommercial/80RC2/devaids/auxiliarydevtool/atlasopdev_16_0027.html （A）
12. CANN 官方《算子工程参考示例》（ASCEND_CANN_PACKAGE_PATH/ASCEND_COMPUTE_UNIT 配置、msopst run）https://www.hiascend.com/document/detail/zh/CANNCommunityEdition/80RC2alpha003/devaids/auxiliarydevtool/atlasopdev_16_0117.html （A）
13. CANN 故障案例《编译运行应用样例报错，提示找不到头文件或库文件》（DDK_PATH/NPU_HOST_LIB 作用）https://www.hiascend.cn/document/detail/zh/CANNCommunityEdition/80RC3alpha001/devguide/maintenref/troubleshooting/troubleshooting_0216.html （A）
14. 昇腾官方论坛 S2 赛季《常见算子报错和复现样例的 FAQ》（507015/507035、errorStr、UB 越界/对齐）https://www.hiascend.com/forum/thread-02100159784900000007-1-1.html （A/B）
15. 昇腾官方博客《使用K8s部署…ACL stream synchronize failed》（fault kernel_name 排查）https://www.hiascend.com/developer/blog/details/0272186082476802015 （B）
16. 昇腾官方博客《CANN版本问题导致bisheng编译报错 unsupported option '--npu-arch=...'》（8.3.RC1 起支持；910B=2201）https://www.hiascend.com/developer/blog/details/0289201670005562056 （A/B）
17. CANN/cannbot-skills 官方配置指南（ASCEND_HOME_PATH/ASCEND_OPP_PATH/LD_LIBRARY_PATH，561107/561003 错误）https://gitcode.com/cann/cannbot-skills （B）

**社区文章（C，未验证）**
18. CSDN《华为昇腾910B GPU服务器初始化准备》（Atlas 800I A2 驱动/固件/CANN 安装与卸载、npu-smi、hccn_tool）https://blog.csdn.net/qq_39146974/article/details/155023733 （C）
19. CSDN《Ascend昇腾npu设备上安装cann和torch…版本对应关系》（HDK↔CANN↔torch_npu 配套、容器挂载问题）https://blog.csdn.net/m0_52182894/article/details/156616648 （C）
20. CSDN《搭建第一个CANN开发环境：避坑指南》（找不到 libascendcl.so/import acl 失败/npu-smi 找不到/版本不匹配）https://blog.csdn.net/qq_41397792/article/details/153776541 （C）
21. CSDN《从环境搭建到算子调试：CANN 9.0 + ops-cv 全流程实战指南测评》（9.0.0 下载/卸载旧版/--install-path/--force/set_env.sh）https://blog.csdn.net/weixin_52908342/article/details/159955428 （C）
22. CSDN《如何零基础搭建昇腾NPU开发环境：CANNLab/Docker/手动安装3种方式实战》（ascendhub 镜像与设备挂载）https://blog.csdn.net/gitblog_00174/article/details/156175508 （C）
23. OpenI 实操《使用免费的OpenI启智平台开发昇腾NPU算子》（bisheng -O2 --cce-soc-version=Ascend910B1 旧式编译示例）https://blog.csdn.net/qq_41823532/article/details/158773506 （C）
24. 昇腾FAQ-A01-硬件相关（npu-smi dcmi 错误、内核不匹配、HwHiAiUser）https://blog.csdn.net/jieph01/article/details/149277585 （C）
25. 华为云博客《CANN学习资源开源仓的算子开发一》（--npu-arch 对应关系表：A2/A3=2201、200I/500 A2=3002、推理=2002、训练=1001）https://bbs.huaweicloud.cn/blogs/475681 （C）
26. GitCode 开源工程 FlagTree《User manual for ascend》（CANN 9.0.0 安装命令：toolkit + 910b-ops/A3-ops，docker 设备挂载）https://github.com/flagos-ai/FlagTree/wiki/User-manual-for-ascend （C）
27. GitCode 开源工程 asc-devkit（Ascend C 语言核心仓，CMake 编译指南与链接库表）https://github.com/hicann/asc-devkit （B）
28. MindSpore 官方教程《CANN常见错误分析》（ASCEND_GLOBAL_LOG_LEVEL、E80000 等编译错误码）https://www.mindspore.cn/tutorials/zh-CN/stable/debug/error_analysis/cann_error_cases.html （B）

---

## 9. 结论与风险提示

1. **本模板（ASC 语言 + find_package(ASC) + --npu-arch=dav-2201）是 CANN 8.3+（推荐 9.0.0）的新式直调工程**，不要用 8.2 及以下环境、也不要用老 ccec/kernel_operator.h 教程逐字套用。
2. 真机第一步永远是 `npu-smi info`（驱动/固件是否就绪）+ `source set_env.sh`（ASCEND_HOME_PATH 是否就绪），run.sh 已内置这两层检查。
3. **判题环境 SoC 未最终确认**：模板默认 `dav-2201`（910B/A2 系），若判题机为其它型号，须用 `-DNPU_ARCH=` 调整；这是提交前必须与判题平台核对的事项。
4. 所有结论基于公开资料，未在本机验证；命令细节（尤其 9.0.0 安装包确切文件名/镜像 tag）以昇腾社区下载页当时展示为准。
