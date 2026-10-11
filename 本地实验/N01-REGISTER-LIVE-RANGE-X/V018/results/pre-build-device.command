REVISION=V018
HOST=hwnput3
DEVICE_ID=5
COMMAND=ssh cann-server3 'date -u; hostname; npu-smi info -t usages -i 5; npu-smi info -t memory -i 5; npu-smi info -t proc-mem -i 5; pgrep -af "[N]01-REGISTER-LIVE-RANGE-X-RESET-20261011/V018/" || true'
EXIT_CODE=0
SNAPSHOT=pre-build-device.txt
