# CANNJudge 工具

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
