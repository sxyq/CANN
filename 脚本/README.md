# CANNJudge 工具

## 登录

```bash
npm run cannjudge:login
```

首次使用时，在打开的浏览器窗口中完成登录。

## 提交

```bash
npm run cannjudge:submit -- --yes --source /absolute/path/kernel.txt
```

`--yes` 确认一次外部提交，`--source` 指定待提交的源码文件。脚本会提交源码并轮询平台结果。

## 帮助

```bash
npm run cannjudge:submit -- --help
```

浏览器会话由项目运行环境管理，工具不读取或导出登录凭据。
