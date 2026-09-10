# 提交核对单（Submission Checklist）

> 使用方式：每项完成打 [x] 并写日期/备注。真机验证前置条件未满足的条目保持 [ ] 并记录 blocker。

## 1. 源码文件清单（../源码/）

| 文件 | 状态 | 备注 |
| --- | --- | --- |
| ../源码/add_rms_norm_bias.json | [x] 已提供 | msopgen 原型定义（dtype 列表含 fp16/bf16/fp32；attr epsilon） |
| ../源码/op_kernel/add_rms_norm_bias.cpp | [x] 已提供 | Ascend C 核函数（Kernel 模板） |
| ../源码/op_host/add_rms_norm_bias_tiling.h | [x] 已提供 | Tiling 数据结构 |
| ../源码/op_host/add_rms_norm_bias.cpp | [x] 已提供 | 算子注册/InferShape/Tiling |
| ../源码/CMakeLists.txt / CMakePresets.json / build.sh | [x] 已提供 | 源码清单与真机构建入口；CANN 工程需由 msopgen 生成 |
| source-build.md | [x] 已提供 | 生成与编译步骤 |
| ../提交/首版/kernel.asc | [x] 源码候选 | 已按平台直调模板组织；尚未在 CANN/NPU 编译和运行 |
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
- SoC 待确认：aclnn/引擎题面是 vector→大概率 Atlas A2 训练（ascend910b?）或 800I A2 推理；**判题型号未知时先以 910B/910A2 双测**（blog 中 AddCustom 样例用 ascend910b）。

## 3. 输入输出接口（与题面对齐）

- 输入：x[(...D)] fp16/bf16/fp32、residual[(...D)] 同型、gamma[(D)] 同型、bias[(D)] 同型；attr epsilon(float, 默认1e-5)。
- 输出：output[(...D)] 同型。
- 内部实现：Kernel 收 GM_ADDR x/r/g/b/output/workspace/tiling；epsilon、outer、D、dtype 通过 tiling 下传。
- 严禁：Host 计算输出、写死常量表。

## 4. 本地验证项目（真机阶段）

- [ ] 用 msopst 或自建用例（pytest+昇腾 ND 样例）跑通 15 个测试点。
- [ ] 先在平台直调模板中跑通 `提交/首版/kernel.asc` 的默认 FP16 `[1,64]` 用例。
- [x] CPU 语义模拟：117 组中 115 组通过本地诊断，2 组 BF16 D=32768 边界差异；详见 `../调研/validation-host-notes.md`。
- [ ] 数值对照基准：在 NPU 上按题面实际参考实现确认 FP32、FP16、BF16 的误差口径。
- 覆盖矩阵（记录到 ../提交/result-<date>.md）：

| 维度 | 取值 |
| --- | --- |
| dtype | fp32 / fp16 / bf16 |
| rank | 2D / 3D / 4D |
| D | 64, 67(非32倍数), 96, 129, 192, 576, 1000, 1024, 4096, 32768 |
| outer | 1, 8, 512, 8192（与 D 组合控制总量 ≤ 判题上限） |
| 特殊值 | 全 0、NaN、±Inf、混合大/小值（防溢出验证） |

## 5. 15 个测试点记录位置

- `../提交/result-<日期>.md`：逐点记录 dtype/shape/误差(相对+绝对)/耗时/通过。
- 模板（每点一行）：

```markdown
| 测试点 | shape | dtype | 相对误差 | 绝对误差 | 耗时(us) | 通过 |
| --- | --- | --- | --- | --- | --- | --- |
| T01 | [2,4] fp32 | fp32 | ... | ... | ... | ✅/❌ |
```

## 6. 上传前内容核对

- [ ] 源码内无凭据/Token/Cookie/授权头（全局 grep：`password|token|secret|api[_-]?key|cookie|authorization`，大小写不敏感）。
- [ ] 仅含平台要求的必要文件：直调模式通常为 `kernel.asc`，msopgen 模式按页面字段上传；不含 build_out/.git/临时产物、不含本机绝对路径。
- [ ] 无写死测试输入/输出；无 Host 代算代码。
- [ ] 公开 Skill 记录了 `tiling_h`、`tiling_key_h`、`host_cpp`、`kernel_cpp` 字段；登录提交页确认本题实际字段和上传格式与平台示例一致。

## 7. 提交前人工确认项

- [ ] 15 点全部通过且误差达标（fp32<1e-4；f16/bf16<1e-3，双误差）；
- [ ] 性能记录完成（题面要求优化）；
- [ ] 用户已阅毕汇报且**明确同意上传/提交**；
- [ ] 当日提交次数余量 > 0。

## 8. 提交后结果记录方式

- 记录到 `../提交/history-<日期>.md`：提交时间、包版本（V<编号>）、子账号、判题结果、错误信息截图/文本、修复内容。
- 若失败：先看判题返回的用例/错误类型（编译错误/精度/超时/崩溃），再定向修复；不盲目重复提交浪费额度。

## 9. 每日 50 次额度记录

| 日期 | 已用 | 剩余 | 备注 |
| --- | --- | --- | --- |
| 2026-09-10 | 0 | 50 | 尚未首次提交 |
| （真机启动后逐日维护） | | | |

> 提醒：额度每天重置；提交命中 50 次上限后当天停止，次日再试。
