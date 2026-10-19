# MIX-A 同设备本地配对 runner

runner 固定测 FP32、`rows=1`、`D=64`，覆盖 MIX-A 当前 tiny 单行替换分支；基线程序从 V003 编译。两边使用相同确定性输入、同一设备和相同 launch/sync 次数。

在 server3 空闲的 910B3 上运行：

```bash
cd /home/data4t2/lelinfeng/phase4-workspaces/MIX-A/runner
DEVICE_ID=4 bash run_pair.sh
```

每个程序先 warmup 2 次，再测 9 次，报告主机 launch 到 stream 同步完成的中位耗时。各自会与 FP32 参考结果比对；`validate_pair.py` 再核对两边 64 个输出均为有限值且逐元素差值不超过 `1e-4`。

`build/` 保留可执行文件和构建目录；`artifacts/` 保留构建日志、两边的耗时/正确性 TSV 和配对 guard 输出。测试输出二进制在脚本退出时删除。runner 不重置设备，避免影响共享设备上的其他进程。
