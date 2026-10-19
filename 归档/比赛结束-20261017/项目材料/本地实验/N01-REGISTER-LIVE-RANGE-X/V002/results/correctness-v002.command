REVISION=V002
MODE=CORRECTNESS
ENV_COMMAND=set +u; source /usr/local/Ascend/ascend-toolkit/set_env.sh
ENV_EXIT_CODE=0
COMMAND=LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6 /home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V002/737143f3e5b0780a0cd96ec31ddfc2f5476e50e5/build/paired_runner_v016 --correctness
DEVICE=7
EXIT_CODE=0
