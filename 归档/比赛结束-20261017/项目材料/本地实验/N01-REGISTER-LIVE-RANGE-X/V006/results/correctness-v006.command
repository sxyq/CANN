REVISION=V006
MODE=CORRECTNESS
COMMAND=source /usr/local/Ascend/ascend-toolkit/set_env.sh; LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6 /home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V006/6cc244b0/build/paired_runner_v016 --correctness
DEVICE_ID=5
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
CANDIDATE_SOURCE_COMMIT=6cc244b025049552159903387f9f77dc668cdce9
CANDIDATE_SOURCE_SHA256=5116d0e69f1fdc4dcb200a56f1882d259ca81b0a77ec6cbb818c1e13118f3211
CANDIDATE_REMOTE_PATH=/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V006/6cc244b0/candidate/R31B-V016-WIDE-TILE-SEED_kernel.asc
RUNNER_TARGET=paired_runner_v016
RUNNER_BINARY_SHA256=239a8cd96dd3ac40b00e8657fc0e0d11ca0b19793ff8719d1155f9752338111a
RUNNER_LABEL_PARENT=V011
RUNNER_LABEL_CANDIDATE=V001
PREFLIGHT=HBM 91 percent; AICore 0 percent; VLLMWorker_TP PID 3355594 with 56354 MB; pre-correctness-attempt-02-device.txt
CORRECTNESS_CASES=4/4 PASS
RUNNER_EXIT_CODE=0
ATTEMPT_01_EXIT_CODE=127
ATTEMPT_01_CAUSE=libgraph.so unavailable because CANN runtime environment was not loaded
ATTEMPT_01_RAW=correctness-v006-attempt-01.raw.log
ATTEMPT_02_COMMAND=source /usr/local/Ascend/ascend-toolkit/set_env.sh; LD_PRELOAD=/usr/lib/aarch64-linux-gnu/libstdc++.so.6 /home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V006/6cc244b0/build/paired_runner_v016 --correctness
ATTEMPT_02_RAW=correctness-v006-attempt-02.raw.log
ATTEMPT_02_EXIT_CODE=0
