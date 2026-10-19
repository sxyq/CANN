# Agent08：Linux / 环境 / 真机工程

> 调研日期：2026-09-11  
> 范围：CANN 安装与 set_env、驱动与 npu-smi、bisheng/ccec/msopgen/msopst、Docker/远程 NPU、多版本共存、常见编译/链接/运行错误；Linux DO 与 V2EX 真实命中单列。  
> 本机：macOS，无 CANN、无 Ascend C 编译器、无昇腾 NPU。**本文任何步骤都没有在本机执行，也不能在本机执行。**  
> 用途：拿到一台昇腾 Linux 机器后，从环境核对到模板 `run.sh` 跑通的操作清单与风险清单。

---

## 0. 一句话结论

模板 `run.sh` 的硬前提是 **Linux + 已装驱动的昇腾卡 + CANN Toolkit + `source set_env.sh` 后 `ASCEND_HOME_PATH` 可用**；`CMakeLists.txt` 再通过 `find_package(ASC)` 拉起 ASC 语言编译器，默认 `--npu-arch=dav-2201`。缺驱动、缺 toolkit、没 source 环境、arch 选错，都会在 `run.sh` 第 1 步或 cmake 阶段失败，而不是 kernel 阶段。真机第一步永远是 `npu-smi info` 与 `echo $ASCEND_HOME_PATH`。

---

## 1. 环境依赖表

| 依赖 | 作用 | 本题/模板中的位置 | 证据 |
| --- | --- | --- | --- |
| Linux x86_64 / aarch64 | CANN 仅提供 Linux 安装包 | Toolkit 文件名 `*_linux-x86_64.run` / `*_linux-aarch64.run` | A：cann-samples README |
| 昇腾 NPU + 驱动 + 固件 | 运行 kernel；驱动版本需与 CANN 配套 | `npu-smi info`；社区帖常见驱动 25.2.3 / 25.5.1 | A/B + C：LD 帖正文 |
| CANN Toolkit ≥ 9.0.0（题目目标 9.0.0） | 提供 set_env、ASC 编译器、ACL、算子库 | `source ${install}/ascend-toolkit/set_env.sh` | A：cann-samples / 官网目录 |
| `ASCEND_HOME_PATH` | `run.sh` 硬性前置；为空直接 exit 1 | `run.sh` L9–14 | A：本地模板 |
| `set_env.sh` | 写入 ASCEND_*、PATH、LD_LIBRARY_PATH 等 | `run.sh` L17 再次 source | A：模板 + cann-samples |
| CMake ≥ 3.16 | 模板 `cmake_minimum_required(3.16)`；`find_package(ASC)` | 模板 CMakeLists L1–3 | A：本地模板 |
| Python 3（gen_data / verify） | 生成 case0 数据、比对结果 | `run.sh` L29、L37；cann-samples 要求 ≥3.8（自检脚本写 ≥3.10） | A：模板 + cann-samples |
| `npu-smi` | 查卡、驱动、固件、占用 | 环境自检项；缺则无法确认设备 | B：check_env.py |
| bisheng / ccec（ASC） | 毕昇编译器链路；模板 `LANGUAGES ASC` | CMake `find_package(ASC)` + `--npu-arch=` | A：模板；A：官网编译器目录入口 |
| `msopgen` | 从 json 生成算子工程骨架 | 主线 msopgen 参考路径，非判题直调路径 | A：源码/build.sh + 官网工具目录 |
| `msopst` | 算子 ST / 仿真自测 | 官网算子开发工具集；本题直调模板不调用 | A：官网工具目录入口；**CLI 细节本轮未逐条核验** |
| 链接库 | `tiling_api` `register` `platform` `unified_dlog` `dl` `m` `graph_base` | 模板 CMakeLists L17–25 | A：本地模板 |
| ACL | `acl/acl.h`、`aclrt*` | `main.asc` | A：本地模板 |
| timeout 120s | `run.sh` 用 `timeout 120 ./add_rms_norm_bias_custom` | 超时即 FAILED | A：本地模板 |
| Stream 同步 3000ms | `aclrtSynchronizeStreamWithTimeout(..., 3000)` | kernel 卡死会在 host 侧报错返回 | A：本地模板 main.asc |

### NPU_ARCH 映射（编译选项，不是驱动命令）

| 产品线 | `--npu-arch` | 来源 |
| --- | --- | --- |
| Ascend 950PR / 950DT | `dav-3510` | A：asc-devkit examples README |
| Atlas A2/A3 训练与推理系列（910B/C） | `dav-2201` | A：asc-devkit；模板默认值 |
| Atlas 推理系列 AI Core | `dav-2002` | A：asc-devkit examples README |

模板：

```cmake
if(DEFINED NPU_ARCH)
    set(SOC_ARCH ${NPU_ARCH})
else()
    set(SOC_ARCH "dav-2201")
endif()
# ...
$<$<COMPILE_LANGUAGE:ASC>:--npu-arch=${SOC_ARCH}>
```

**提交前必须与判题平台核对 SoC**；本地能编过不等于 arch 对。

---

## 2. 拿到昇腾机器后：安装与验证步骤（操作清单）

以下顺序按「先设备、后 toolkit、再模板」排列。每步给出预期现象；失败时停在该步，不要跳到编译。

### 2.1 设备与驱动层

1. 确认发行版与 CPU 架构  
   - `uname -m` → `x86_64` 或 `aarch64`（决定下载哪个 `.run`）  
   - 社区实录示例：Huawei Cloud EulerOS、ARM 内网机（LD 1904864）。
2. 看卡是否在  
   ```bash
   npu-smi info
   ```  
   - 期望：能看到设备型号、固件、驱动版本、占用。  
   - `command not found`：驱动/工具未装，或 PATH 未含 `npu-smi`（常见路径与驱动包一起安装）。  
   - 有命令但报错/空设备：驱动与固件不匹配，或容器未透传设备。
3. 记录驱动/固件版本，后续与 CANN 版本对照（社区帖出现过驱动 25.2.3、25.5.1；**精确配套表以官网版本说明为准，本轮未从官网 SPA 页抓到完整矩阵**）。

### 2.2 安装 CANN Toolkit（以 9.0.0 为例）

cann-samples 已验证包（A 级，仓库 README）：

- 9.0.0 时间戳 `20260422000325096`：  
  `Ascend-cann-toolkit_9.0.0_linux-x86_64.run` / `..._linux-aarch64.run`  
- 9.1.0 多个 weekly 亦 PASS（本题目标仍以 **9.0.0** 为准）。

安装骨架（A：cann-samples）：

```bash
chmod +x Ascend-cann-toolkit_9.0.0_linux-${arch}.run
./Ascend-cann-toolkit_9.0.0_linux-${arch}.run --install --force --install-path=${install_path}
# 默认常用 install_path=/usr/local/Ascend 或 ${HOME}/Ascend
```

注意：

- 需要完整 Toolkit，不是只装 run 包；cann-samples 明确：版本过旧、仅 Run 包、环境未 source → 头文件缺失、符号未定义、编译选项报错。  
- 本机 **禁止** 执行任何安装。

### 2.3 环境变量（set_env.sh）

```bash
source ${install_path}/ascend-toolkit/set_env.sh
echo "ASCEND_HOME_PATH=${ASCEND_HOME_PATH}"
which ccec || true
# 若环境提供 msopgen / msopst：
which msopgen msopst || true
```

`check_env.py`（B：gitcode cann-samples）核对项可直接当清单用：

| 核对项 | 通过条件 |
| --- | --- |
| cmake / python3 / pip3 / git / zip | 在 PATH；cmake≥3.16 |
| `ASCEND_HOME_PATH` | 非空 |
| `set_env.sh` | 存在于 `$ASCEND_HOME_PATH/set_env.sh` 或邻近 `ascend-toolkit/` |
| `LD_LIBRARY_PATH` | 含 ascend 路径 |
| `npu-smi` | 有则 PASS；无则 WARN（可编译不可跑） |

多版本共存（工程实践，**非本轮 A 级官方原文**）：

- 不同版本装到不同 `--install-path`（例如 `/usr/local/Ascend` 与 `${HOME}/Ascend-9.0`）。  
- 切换 = 重新 `source` 对应 `set_env.sh`；同一 shell 内避免混用两个版本的 PATH/LD_LIBRARY_PATH。  
- 驱动/固件只有一套，升级 CANN 前先看驱动兼容；不要假设「多 toolkit 任意 source」一定安全。  
- 验证是否切对：`echo $ASCEND_HOME_PATH` + 能否 `find_package(ASC)` + 编译出的 kernel 行为。

### 2.4 放入模板并跑通

```bash
# 假设模板在 /path/to/addrmsnormbias_problem_1742_template
cp 提交/V00X/kernel.asc /path/to/addrmsnormbias_problem_1742_template/kernel.asc
cd /path/to/addrmsnormbias_problem_1742_template
source ${install_path}/ascend-toolkit/set_env.sh
./run.sh
```

`run.sh` 四步（A：本地模板）：

| 步骤 | 命令 | 失败含义 |
| --- | --- | --- |
| 1/4 | source `$ASCEND_HOME_PATH/set_env.sh` | 环境未准备好 |
| 2/4 | `rm -rf build && cmake .. && make -j4` | 编译/链接/ASC 工具链问题 |
| 3/4 | `python3 ../scripts/gen_data.py` | Python 或路径问题 |
| 4/4 | `timeout 120 ./add_rms_norm_bias_custom` + `verify_result.py 0` | 运行时崩溃、超时、精度 |

模板本地样例仅覆盖 **FP16、shape `[1,64]`、eps=1e-5、case0**。`run.sh` PASSED ≠ 15 个判题点通过。

### 2.5 非对齐 D / 多 dtype 的真机最小实验（建议）

在模板通过后再扩（仍是同一可执行文件，改 gen_data 或自备 input）：

1. D 非 32 倍数：67 / 129 / 1000，核对输出尾字节是否覆盖相邻行。  
2. BF16、FP32。  
3. 多核：改 `availableCoreNum` 逻辑或加大 outer，确认 32B cache line 对齐策略。  
4. 记录：编译命令、`npu-smi` 摘要、耗时、最大误差。

---

## 3. 工具链：bisheng / ccec / msopgen / msopst

| 工具 | 角色（用于本题） | 确认程度 |
| --- | --- | --- |
| **毕昇编译器（Bisheng）** | CANN 9.0.0 文档「编译器」入口；把 Ascend C/ASC 编成设备侧二进制 | A：官网目录存在该章节；**具体 ccec 命令行参数本轮未从正文抓全** |
| **ccec / ASC** | 模板 `project(... LANGUAGES ASC CXX)`；CMake 通过 `find_package(ASC)` 注册语言与工具 | A：模板 CMake 行为；驱动可执行文件名在真机 `which ccec` 验证 |
| **msopgen** | `msopgen gen -i xxx.json -c ai_core-<soc> -lang cpp -out <dir>` 生成 op_host/op_kernel/CMake 骨架 | A：本仓库 `源码/build.sh`、文档/source-build.md；官网算子开发工具目录 |
| **msopst** | 算子系统测试/仿真入口（工具集内） | A：官网工具集列出 msOpST；**本题判题为直调 `kernel.asc`，不依赖 msopst**；CLI 细节未核 |
| **msKPP / msSanitizer / msDebug / msProf** | 设计、消毒、调试、性能 | A：官网开发工具目录；与 run.sh 无直接关系 |

本题两条入口不要混：

1. **判题主路径（直调）**：`kernel.asc` + 模板 CMake/run.sh。  
2. **参考路径（msopgen）**：json → 骨架 → 覆盖 op_kernel/op_host → cmake（见 `文档/source-build.md`）。

---

## 4. Docker / 远程 NPU 开发

### 4.1 Docker（社区与 issue 侧，官方镜像名未在本轮 A 级页面核验）

- 常见需求：容器内看到 NPU（`npu-smi`）、ACL 能 `aclrtSetDevice`。  
- 真实 issue：vllm-ascend「容器中获取不到 NPU 信息」（C：GitHub Issue #632，2025-2026 存活链接，说明容器设备透传是高频坑）。  
- 工程要点（通用实践，需在目标机验证）：  
  - 宿主机驱动已装好；容器内通常仍要 toolkit 或运行时库。  
  - 设备节点 / `--device`、特权或厂商 runtime；未透传时表现为 `npu-smi` 无设备或 ACL 初始化失败。  
  - 路径：挂载 `/usr/local/Ascend` 或在容器内再装 toolkit 并 `source set_env.sh`。  
- **本轮未能从 hiascend 正文抓到官方 Docker 镜像完整命令**；真机以现场文档与 `npu-smi` 为准。

### 4.2 远程开发

- 典型：SSH 到 910B/C 机器 → `source set_env.sh` → 同步 `kernel.asc` → `./run.sh`。  
- 社区实录（LD 1904864）：内网 ARM + 8×910B4 + 离线，先 `uname -m`、驱动版本，再部署模型——算子开发同理，先锁环境再改代码。  
- macOS 本机编辑 + rsync/scp 到 Linux 是现实路径；**编译与运行只能在 NPU 机**。  
- VS Code Remote-SSH 可用，但扩展/语言服务器未必认识 `.asc`；以远端 `cmake/make` 日志为准。

---

## 5. 常见错误（编译 / 链接 / 运行 / 超时 / 越界）

### 5.1 环境与配置

| 现象 | 可能原因 | 处置 |
| --- | --- | --- |
| `ERROR: ASCEND_HOME_PATH is not set` | 未 source 或路径错 | `source .../ascend-toolkit/set_env.sh` |
| `find_package(ASC REQUIRED)` 失败 | 未 source、非完整 Toolkit、CMake 缓存旧 | 清空 build，source 后重配；确认 toolkit 版本 |
| `--npu-arch` 报错或生成错误代码 | arch 与真机/判题不符 | 查 `npu-smi` 型号；用 `-DNPU_ARCH=` 覆盖 |
| 头文件/符号找不到 | 只装 run 包、LD_LIBRARY_PATH 未含 ascend | 完整 toolkit + set_env |
| `npu-smi` 不存在 | 驱动未装或容器未透传 | 装驱动或修 Docker 设备映射 |

### 5.2 编译期（与本仓库历史相关）

| 现象 | 事实 | 处置 |
| --- | --- | --- |
| 15/15 CE | V001：`pipe_` 宏/标识符与头文件或模板冲突（项目已记录） | 避免占用 `pipe_` 等保留名；与 `kernel_operator.h` 冲突时改私有成员名 |
| ASC 编译选项不识别 | CANN < 8.3 无 `--npu-arch` 类直调链路（项目文档） | 使用 CANN 9.0.0 |
| `DataCopyPadExtParams<T>` 实参顺序 | 裸聚合初始化会把 left/right 填反（V002 红线） | 具名初始化锁死字段 |

### 5.3 链接期

| 现象 | 可能原因 |
| --- | --- |
| `tiling_api` / `platform` / `unified_dlog` / `graph_base` 未找到 | 库路径未进 link；set_env 未生效；toolkit 不完整 |
| ACL 相关 undefined | 未链接运行时或 `LD_LIBRARY_PATH` 无 `libascendcl` 等 |

### 5.4 运行期（kernel 崩溃 / 越界 / 超时）

| 现象 | 风险点 | 本题对应 |
| --- | --- | --- |
| 进程非零退出 / 段错误 | 空指针、shape 未校验、核数为 0 | `main.asc` 已查 `availableCoreNum<=0` |
| `aclrtSynchronizeStreamWithTimeout` ret≠0 | kernel 挂死、非法访存、设备错误 | 同步超时 3000ms |
| `timeout 120` 杀掉 | 死循环、核间死锁、等待 flag 未成对 | 核对 SetFlag/WaitFlag 配对、循环上界 |
| 输出污染相邻行 | UB→GM DataCopyPad 按 32B burst 写回（社区争议） | D 非 32 倍数真机逐字节验证 |
| 多核偶发错误 | 行切分破坏 32B cache line 对齐 | 按 `k*D*sizeof(T) ≡ 0 (mod 32)` 分配 |
| 大 shape 地址错乱 | `uint32_t` 行基址溢出 | 统一 `uint64_t` |
| 精度 WA | 低精度中间累加、Cast 舍入 | 归约 FP32；fp32→bf16 用 CAST_RINT |

### 5.5 与「CE / WA / RE / TLE」对照

| 判题表现 | 环境侧常见根因 |
| --- | --- |
| CE | 宏冲突、API 不存在（版本/arch 不对）、编译选项 |
| RE | 越界、空指针、非法同步、设备未就绪 |
| TLE | 死循环、错误循环上界、多核退化；**本地 timeout 120 仅为模板保护** |
| WA | 尾块、舍入、多核踩踏、eps 位置 |

---

## 6. 社区真实命中（专节）

### 6.1 获取方式与限制

- `linux.do` 对本环境 **search / latest / .json 返回 403**；topic 正文页可 GET 200。  
- `v2ex.com` 站内 search 对不同关键词返回**同一份无关列表**（不可用）；**官方 API** `https://www.v2ex.com/api/topics/show.json?id=` 可验证标题/作者/时间。  
- 以下条目均为 **Brave 检索到 URL 后，再打开原文页或 API 核验**。未核验的不写。

### 6.2 Linux DO（真实命中）

| 标题 | 作者 | 日期 | 链接 | 与本任务关系 | 证据 |
| --- | --- | --- | --- | --- | --- |
| 搞了一台华为910C的四节点集群 按照官方教程自部署K3报错 各位佬能帮忙看下吗 | 正文页未暴露稳定作者字段 | 页面可访问（抓取日 2026-09-11）；描述含 NPU 驱动 25.5.1 | https://linux.do/t/topic/2710138 | 集群/驱动/固件版本现场问题 | C：标题+描述已核验 |
| 用Agent在垂域进行基于国产平台的自动算子开发和优化有前景吗？ | `musk_elon`（QAPage name） | 2026-07-23（ld+json datePublished） | https://linux.do/t/topic/2640471 | 提到 Ascend C、CANN 与自动算子 | C：ld+json |
| 新人刚进，给大家分享最近部署的一点点经验，关于qwen3.5 122b部署在昇腾910B4服务器上… | 正文页未暴露稳定作者字段 | 抓取日 2026-09-11 可访问 | https://linux.do/t/topic/1904864 | **8×910B4、驱动 25.2.3、ARM、内网离线、环境确认步骤** | C：title+description |
| [开源] 天下苦昇腾久矣，torch-npu/mindspore？来看看candle吧 | 正文页未暴露稳定作者字段 | 抓取日可访问 | https://linux.do/t/topic/1802595 | 昇腾软件栈生态讨论 | C：title+description |

说明：Discourse 页面作者/楼层依赖前端 JSON；`.json` 被 403，故部分作者字段标为「未暴露」。**禁止脑补作者名。**

### 6.3 V2EX（真实命中，API 核验）

| 标题 | 作者 | 日期（CST） | 链接 | 回复 | 与本任务关系 |
| --- | --- | --- | --- | --- | --- |
| 昇腾 怎么感觉是虚假宣传吗？ | DeYiAo | 2026-07-31 | https://www.v2ex.com/t/1231122 | 40 | 生态适配节奏；非编译手册 |
| 公司有一台闲置的昇腾 300I 8GPU 的服务器，有什么好的用法呢 | sohk | 2026-07-12 | https://www.v2ex.com/t/1226680 | 6 | 300I Duo 整机配置（Kunpeng 920 等） |
| 昇腾是目前性价比最高的推理 GPU 吗？ | letmatte | 2026-05-20 | https://www.v2ex.com/t/1214031 | 8 | 选型讨论 |
| 中国的算力缺口这么大嘛？看到 2025 华为昇腾出货 81 万块… | lynn1su | 2026-04-22 | https://www.v2ex.com/t/1207741 | 45 | 行业规模，非环境 |
| 想做个调研，看看国产 AI 芯片到什么程度了 | pigpigxia | 2025-11-06 | https://www.v2ex.com/t/1170850 | 15 | 含昇腾/思元成本与维护 |
| 有用昇腾部署 DeepSeek-R1-32B 的吗？ | shenwy | 2025-04-28 | https://www.v2ex.com/t/1128631 | 1 | 明确问「有哪些坑」；训练/推理卡分工 |

补充：上述 V2EX 帖以**选型、闲置机器、生态吐槽**为主，**没有**可直接当 `run.sh` 手册的逐步安装长文。环境细节仍以官方 Toolkit + cann-samples 为准。

### 6.4 其他社区（补充，非 Linux DO/V2EX）

| 来源 | 条目 | 链接 | 备注 |
| --- | --- | --- | --- |
| GitCode cann-samples Issues | 环境自检、NPU_ARCH、样例编译 | https://gitcode.com/cann/cann-samples/issues | B/C |
| GitCode asc-devkit Issues | AscendC 文档/接口/挂死类反馈 | https://gitcode.com/cann/asc-devkit/issues | B/C |
| GitHub vllm-ascend | 容器中获取不到 NPU | https://github.com/vllm-project/vllm-ascend/issues/632 | C：Docker 坑 |
| Stack Overflow / Unix&Linux / Ask Ubuntu / Server Fault / Phoronix / HPCwire | 本轮 Brave 对 `site:stackoverflow.com Ascend CANN` 返回 429；未得到可核验的高相关帖子 | — | **本轮未找到可引用的 SO/UL/AU/SF 帖** |

---

## 7. 已确认 / 未确认

### 7.1 已确认（有文件或可访问页面支撑）

1. 模板 `run.sh` 强制校验并 `source` `$ASCEND_HOME_PATH/set_env.sh`。  
2. 模板 `CMakeLists.txt`：`find_package(ASC REQUIRED)`、`LANGUAGES ASC CXX`、默认 `SOC_ARCH=dav-2201`、链接 `tiling_api/register/platform/unified_dlog/dl/m/graph_base`。  
3. `main.asc` 使用 ACL，同步超时 3000ms；本地 case 为 FP16 `[1,64]`。  
4. cann-samples：Toolkit `.run` 安装与 `source set_env.sh` 流程；已验证 CANN 9.0.0/9.1.0 包时间戳。  
5. npu-arch：950→dav-3510，910B/C→dav-2201，推理 Core→dav-2002。  
6. 本仓库 V001 因 `pipe_` 冲突 15/15 CE（项目文档）。  
7. Linux DO / V2EX 存在上表真实帖（链接可打开或 API 返回一致）。  
8. 本机无 CANN/NPU，不能做真机验证。

### 7.2 未确认 / 无法在本机确认

1. 判题机 SoC 是否为 `dav-2201`（须平台确认）。  
2. 驱动 ↔ CANN 9.0.0 的**完整官方配套矩阵**（hiascend 正文为前端渲染，本轮未抓到表体）。  
3. 官方 Docker 镜像名、完整 `docker run` 设备参数。  
4. 多版本 CANN 并行切换的官方唯一推荐命令。  
5. `msopst` 具体子命令与本题无关性的官方原文。  
6. Linux DO 部分主题的楼主用户名（`.json` 403）。  
7. 任何「本机已编译/已跑通/精度通过」陈述——**均不成立**。

---

## 8. 真机 run.sh 前 10 条速查

```text
1. uname -m 与 npu-smi info 正常
2. 驱动/固件版本已记录，与 CANN 9.0.0 兼容（以现场文档为准）
3. 已安装完整 Ascend-cann-toolkit_9.0.0_linux-*.run
4. source ${install}/ascend-toolkit/set_env.sh
5. echo $ASCEND_HOME_PATH 非空，且指向该 toolkit
6. cmake>=3.16、python3 可用
7. SoC 确认后决定是否 -DNPU_ARCH=（默认 dav-2201）
8. kernel.asc 无 pipe_ 等冲突名；DataCopyPadExtParams 具名初始化
9. ./run.sh：build → gen_data → timeout 120 运行 → verify case0
10. case0 PASSED 后再扩 D 非对齐 / BF16 / FP32 / 多核
```

---

## 9. 来源清单

| # | 标题 | URL | 等级 | 用途 | 访问日 | 状态 |
| --- | --- | --- | --- | --- | --- | --- |
| S1 | CANN 产品文档目录（9.0.0） | https://www.hiascend.com/cann 与 https://www.hiascend.com/document/detail/zh/canncommercial/900/ | A | 安装/编译器/工具/环境变量入口 | 2026-09-11 | partial（目录可见，正文 SPA 未完整抓取） |
| S2 | cann-samples README（环境部署 / set_env / NPU_ARCH / 9.0.0 包） | https://gitcode.com/cann/cann-samples ；raw：https://raw.gitcode.com/cann/cann-samples/raw/master/README.md | A | 安装命令、依赖、arch 表 | 2026-09-11 | verified |
| S3 | cann-samples scripts/check_env.py | https://gitcode.com/cann/cann-samples/blob/master/scripts/check_env.py | B | 环境自检项 | 2026-09-11 | verified |
| S4 | asc-devkit examples README（npu-arch 映射） | https://gitcode.com/cann/asc-devkit/tree/master/examples | A | dav-2201/3510/2002 | 2026-09-11 | verified |
| S5 | 本题模板 run.sh / CMakeLists.txt / main.asc | `/Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/` | A | ASCEND_HOME_PATH、链接库、timeout | 2026-09-11 | verified |
| S6 | 文档/source-build.md（真机步骤与红线） | 本仓库 `文档/source-build.md` | B | 与 Agent08 交叉的安装要点 | 2026-09-11 | verified |
| S7 | Linux DO 910C 集群 K3 | https://linux.do/t/topic/2710138 | C | 驱动/集群现场 | 2026-09-11 | verified（标题+描述） |
| S8 | Linux DO Agent+AscendC/CANN | https://linux.do/t/topic/2640471 | C | 生态/工具链讨论 | 2026-09-11 | verified（ld+json） |
| S9 | Linux DO 910B4 部署实录 | https://linux.do/t/topic/1904864 | C | 驱动 25.2.3、环境确认 | 2026-09-11 | verified（描述） |
| S10 | V2EX 昇腾话题 API | https://www.v2ex.com/api/topics/show.json?id=1231122 等 | C | 标题/作者/时间 | 2026-09-11 | verified |
| S11 | vllm-ascend Issue #632 容器 NPU | https://github.com/vllm-project/vllm-ascend/issues/632 | C | Docker 坑 | 2026-09-11 | partial（检索命中，未通读全文） |
| S12 | Gitee Ascend/samples 迁移说明 | https://gitee.com/ascend/samples | B | 样例已迁 gitcode.com/cann | 2026-09-11 | verified |

---

## 10. 边界声明

- 未修改 `源码/`、`提交/`、`文档/` 主线；仅新增本调研文件。  
- 未安装任何软件；未上传 CANNJudge。  
- Linux DO / V2EX 条目均可打开核验；无编造帖。  
- 不能把 macOS 上的 CPU 参考计算或普通 C++ 编译当成 Ascend C / NPU 通过。
