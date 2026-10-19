ssh cann-server3 'printf "HOST="; hostname; date -Is; npu-smi info -t usages -i 3; npu-smi info -t proc-mem -i 3; printf "N03_RUNNER_PROCESSES\\n"; pgrep -x paired_runner_v016 || true'
