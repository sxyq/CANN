ssh cann-server3 'printf "HOST="; hostname; date -Is; npu-smi info -t usages -i 3; printf "N03_RUNNER_PROCESSES\n"; pgrep -af "N03-VECTOR-ILP-DUALCHAIN-X.*/V010/204775a3|paired_runner_v016" || true'
