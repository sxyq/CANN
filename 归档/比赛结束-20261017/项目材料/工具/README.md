# CANNJudge 工具

本页记录已有本地工具的使用入口、资源信息和结果保存位置。

## server3 资源

server3-resource-policy.sh 是活动 server3 脚本使用的设备选择 helper，设备资源参考值为 MIN_FREE_HBM_MB=100。

FREE_HBM >= 100 MB 时，Compile、Correctness、Local、Profile 可以直接执行。AICore utilization、VLLM、其他用户进程、device 非 idle、system load 和 lease 作为上下文保存。lease 只用于协调和记录，不删除、覆盖、伪造或重写他人的 lease。

示例：

    source 工具/server3-resource-policy.sh
    DEVICE="$(choose_eligible_device 0 1 2 3 4 5 6 7)" || echo "所有可用 NPU FREE_HBM < 100MB"
    record_load_context "$DEVICE"

## 登录

    npm run cannjudge:login

首次使用时，在打开的浏览器窗口中完成登录。浏览器会话由项目运行环境管理，工具不读取或导出登录凭据。

## 线上提交

使用已有入口：

    npm run cannjudge:submit -- --yes --source /absolute/path/kernel.asc

提交响应、源码路径、源码身份、平台状态和结果文件保存到既有线上结果目录。平台实际要求和脚本返回值按原样记录。

## 只读查询

    python3 工具/cannjudge.py poll <submissionId> --once
    python3 工具/cannjudge.py problem

## 帮助

    npm run cannjudge:submit -- --help
