# manifests

本目录存放绑定清单：源码 SHA-256 ↔ git commit ↔ 线上 submission id / Official Score 的对应关系。

约定：

- 每个版本一份清单，文件名随版本（例如 `V005.md`、`FULL-R013-V001.md`）。
- 清单只登记已确认的绑定；任何一环缺失就写“未绑定”，不推断、不补全。
- 历史分数大多没有 submission / commit 绑定，见 `文档/代码与结果溯源.md`；存量缺口不做倒推修补。
- 新提交按 `文档/实验纪律.md`：候选阶段即绑定 SHA-256 与 git commit，提交后补记 submission id。

当前状态：目录刚建立，尚无清单文件。
