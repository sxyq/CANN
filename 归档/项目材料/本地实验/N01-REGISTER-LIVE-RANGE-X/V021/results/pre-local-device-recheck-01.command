REVISION=V021
HOST=hwnput3
DEVICE_ID=5
COMMAND=ssh cann-server3 'date -u; hostname; npu-smi info -t usages -i 5; npu-smi info -t memory -i 5; npu-smi info -t proc-mem -i 5; pgrep -af "[p]aired_runner_v016" || true'
PURPOSE=live pre-local resource and process state before the paired measurement
EXIT_CODE=0
SNAPSHOT=pre-local-device-recheck-01.txt
