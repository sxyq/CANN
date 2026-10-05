# CANNJudge 工具

## server3 资源准入

`server3-resource-policy.sh` 是活动 server3 脚本统一引用的设备选择 helper，唯一准入常量为`MIN_FREE_HBM_MB=100`。

```bash
source 工具/server3-resource-policy.sh
DEVICE="$(choose_eligible_device 0 1 2 3 4 5 6 7)" || echo "所有可用 NPU FREE_HBM < 100MB"
record_load_context "${DEVICE}"
```

`FREE_HBM >= 100 MB` 时，Compile、Correctness、Local、Profile 均允许立即执行。AICore utilization、VLLM、其他用户进程、device 非 idle、system load、lease 和 exclusive authorization 都不是执行 Gate；`record_load_context` 只记录这些事实。

lease 只是协调元数据，不是执行权限。不得为了运行实验删除、覆盖、伪造或重写他人的 lease。

## 登录

```bash
npm run cannjudge:login
```

首次使用时，在打开的浏览器窗口中完成登录。

## 正式提交（唯一入口）

```bash
npm run cannjudge:submit -- --yes --source /absolute/path/kernel.asc
```

强制规则（脚本已内嵌，违反即退出码 1）：

1. 必须带 `--source <file>`。没有 `--source` 直接拒绝。
2. 禁止 clipboard / terminal paste / `--source -`（stdin）作正式提交。
3. 提交前打印：
   - source path
   - line count
   - byte count
   - SHA256
   - first non-empty line
   - last non-empty line
   - 同目录 `submission.sha256`（若存在）
4. 同目录存在 `submission.sha256` 时自动核对；SHA 不一致立即停止（`SOURCE_SHA_MISMATCH`）。
5. 提交后读取 Judge `files[].content`，重算 remote kernel SHA。
   - 要求 `LOCAL_SHA == REMOTE_SHA`
   - 否则标记 `INPUT_IDENTITY_MISMATCH`，退出码 3
   - 该 submission **不得**作为正式实验结果（`formalResultEligible=false`）。

禁止：

```bash
npm run cannjudge:submit -- --yes
```

## 只读查询

```bash
python3 脚本/cannjudge.py preflight <path>
python3 脚本/cannjudge.py poll <submissionId> --once
python3 脚本/cannjudge.py problem
```

## 帮助

```bash
npm run cannjudge:submit -- --help
```

浏览器会话由项目运行环境管理，工具不读取或导出登录凭据。
