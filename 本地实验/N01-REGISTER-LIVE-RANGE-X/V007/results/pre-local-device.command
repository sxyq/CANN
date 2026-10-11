REVISION=V007
HOST=hwnput3
DEVICE_ID=5
COMMANDS=date -u; hostname; npu-smi info -t usages -i 5; npu-smi info -t memory -i 5; npu-smi info -t proc-mem -i 5; pgrep -af '/V007/a22a9fb3/build/paired_runner_v016' || true
SNAPSHOT=pre-local-device.txt
