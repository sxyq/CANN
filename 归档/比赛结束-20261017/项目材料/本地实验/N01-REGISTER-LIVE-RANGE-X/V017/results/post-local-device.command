REVISION=V017
HOST=hwnput3
DEVICE_ID=5
DEVICE_SNAPSHOT_COMMAND={ date -u; hostname; npu-smi info -t usages -i 5; npu-smi info -t memory -i 5; npu-smi info -t proc-mem -i 5; pgrep -af "[p]aired_runner_v016 --paired" || true; }
SNAPSHOT=post-local-device.txt
