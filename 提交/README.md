# 提交工作区

本目录只保存经过源码审阅、真机编译和精度验证后准备上传的唯一候选文件，以及逐次结果记录。

当前候选文件为 `首版/kernel.asc`。它来自平台提供的直调模板，只包含 `kernel.asc`，没有 Host 代算、空 Kernel 或测试数据表。当前机器没有 CANN 和昇腾 NPU，因此它仍属于“源码候选”，不能写成已编译、已通过精度或已通过性能。

## 真机生成第一个候选版本

将候选文件放入平台模板目录后，在模板目录内执行模板提供的 `run.sh`：

```bash
cp /Users/sunyiyang/Desktop/Project/cann/提交/首版/kernel.asc \
   /Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template/kernel.asc
cd /Users/sunyiyang/Downloads/addrmsnormbias_problem_1742_template
source /usr/local/Ascend/ascend-toolkit/set_env.sh
./run.sh
```

上面的安装路径和 `NPU_ARCH` 需要按真机实际环境替换。模板自带用例只有一个 FP16 `[1, 64]` 样例，跑通它只能说明直调入口可运行，不能代表平台 15 个测试点全部通过。

## 上传前提

CANNJudge 的上传字段、目标 SoC、完整测试点和性能结果仍需在登录提交页或真机环境确认。完成源码审阅、构建、精度与性能记录后，先由用户确认；当前不执行最终上传。

建议文件：

- `结果-YYYY-MM-DD.md`：逐测试点误差、耗时和判定结果。
- `历史-YYYY-MM-DD.md`：提交时间、包版本、平台返回和后续处理。
- 经验证后的唯一提交包，文件名和结构以平台要求为准。
