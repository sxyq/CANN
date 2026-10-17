REVISION=V009
HOST=hwnput3
DEVICE_ID=5
COMMANDS=date -u; hostname; npu-smi info -t usages -i 5; npu-smi info -t memory -i 5; npu-smi info -t proc-mem -i 5; pgrep -af '/V009/4298b511/build/paired_runner_v016' || true
SNAPSHOT=pre-correctness-device.txt
