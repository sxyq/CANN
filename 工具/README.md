# 保留工具（比赛结束）

工具目录仅保留已有源码和最小配置，不包含编译产物；比赛已结束，不自动运行实验、Sweep 或 Judge。

| 入口 | 历史用途 |
|---|---|
| `cannjudge-submit.mjs`、`cannjudge.py` | 原平台提交、查询与真实回执保存 |
| `n06-n10-canonical-sweep.mjs` | 原 Candidate Build → Correctness → Canonical Local 调用链 |
| `server3-resource-policy.sh` | 原设备资源选择，历史 MIN_FREE_HBM_MB=100 |
| `提取/` | 原資料提取工具及其测试、配置 |
| `package.json`、`package-lock.json`（根目录） | 原依赖与入口定义 |

本次没有新增 Runner、Scorer、调度器或第二套执行链，也没有改评分公式；Kernel 和既有工具正文保持来源身份。历史工作树内的 Runner 源码、CMake 与适配输入保存在比赛结束归档的源码对象索引中，Runner ELF 和构建缓存已移到仓外恢复目录。

原脚本内的 `本地实验/`、`线上结果/`、`worktrees/` 等路径是比赛期间的执行约定，不代表归档后可直接复跑；如未来用户另行明确授权实验，应先核实已有入口与必要依赖，不自行批量测试。

归档结果现在位于 `归档/比赛结束-20261017/实验记录/`。登录凭据和浏览器会话不纳入仓库，不输出 Token，不在 server3 上发起 Official 提交。

查看 [比赛结束归档说明](../归档/比赛结束-20261017/README.md) 与 [项目结构](../项目结构.md) 获取保存和恢复位置。
