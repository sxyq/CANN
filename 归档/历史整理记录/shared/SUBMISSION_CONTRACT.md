# CANNJudge 提交要求

线上提交与 Git 写入由 Main 串行执行，Child 不执行线上提交。

入口：

```text
/Users/sunyiyang/Desktop/Project/cann/脚本/cannjudge-submit.mjs
```

提交前需要完成 device compile、submission compile 和 full link，并分别保留结果。source 文件应包含合法 Vector Kernel 入口及可调用的 `run_kernel`，路径位于对应 Phase4 workspace。Main 先把本版源码和编译证据提交并推送到 `origin exp/independent-breadth`。

Main 使用：

```text
npm run cannjudge:submit -- --yes --source /absolute/path/to/source
```

判题器预先定义 `TensorInfo` 和 `TensorGroupInfo`。最终提交文件不得重复定义这两个类型；本地编译辅助文件可以提供它们，再包含提交源码。

判题器 dtype 编码是 `0=FP32`、`1=FP16`、`2=BF16`。内部编码如有差异，入口必须显式转换。

`run_kernel` 的参数次序为 x 及其 TensorGroupInfo、residual 及其 TensorGroupInfo、gamma 及其 TensorGroupInfo、bias 及其 TensorGroupInfo、output 及其 TensorGroupInfo、availableCoreNum、stream、epsilon。

每份线上结果保留全部 15 点状态、耗时、Official Score 和错误信息。结果保存在对应 `phase4/online/<candidate>/<revision>/result.json`，并追加 `phase4/control/results.tsv`：

```text
candidate\trevision\tcommit\tmain_change\tcompile\tsubmission_id\tpass_count\tofficial_score\tstatus\tnotes
```

Main 保存线上结果后立即单独提交并推送，再将本路线数据交给对应 Child。不得将其它路线实现或分析传入 Fresh 路线。
