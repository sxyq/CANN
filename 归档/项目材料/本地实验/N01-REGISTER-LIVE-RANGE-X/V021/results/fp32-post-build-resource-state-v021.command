REVISION=V021
HOST=hwnput3
DEVICE_ID=5
COMMAND=ssh cann-server3 'date -u; df -h /tmp /home/data4t2; df -B1 /home/data4t2; df -i /tmp /home/data4t2; pgrep -af "[p]aired_runner_v016" || true; npu-smi info -t usages -i 5; npu-smi info -t proc-mem -i 5; sha256sum /home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V021/c50d6f4a/build/paired_runner_v016'
PURPOSE=read-only resource and device state after FP32 runner build failure; no FP32 device test started
EXIT_CODE=PENDING
SNAPSHOT=fp32-post-build-resource-state-v021.txt
