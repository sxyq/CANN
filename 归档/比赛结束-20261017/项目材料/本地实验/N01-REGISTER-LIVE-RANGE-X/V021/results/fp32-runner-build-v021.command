REVISION=V021
PURPOSE=add one FP32-only case to the existing paired runner target
DEVICE_KERNEL_SOURCE_SHA256=d4f9994b9eb43e75b454143cbdb3f2b8a2222545d6ad4496fc6022c1682e5134
MEASUREMENT_PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
RUNNER_TARGET=paired_runner_v016
RUNNER_SOURCE_REMOTE=/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V021/c50d6f4a/support/paired/paired_runner.cpp
RUNNER_SOURCE_ORIGINAL_SHA256=4f6701ea7edbbcfd3e410e22c0c3ea39a25bc4996b9bf05be4c422827c56f789
RUNNER_SOURCE_FP32_SHA256=7836fb57a342642132a3345209796665b7fa900b599d3842f37560b446b5ba3b
UPLOAD_COMMAND=scp 本地实验/N01-REGISTER-LIVE-RANGE-X/V021/results/paired_runner-v021.cpp cann-server3:/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V021/c50d6f4a/support/paired/paired_runner.cpp
BUILD_COMMAND=ssh cann-server3 'source /usr/local/Ascend/ascend-toolkit/set_env.sh && cmake --build /home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V021/c50d6f4a/build --target paired_runner_v016 -- -j2'
UPLOAD_EXIT_CODE=0
BUILD_ATTEMPT_01_EXIT_CODE=2
BUILD_RETRY_01_COMMAND=ssh cann-server3 'source /usr/local/Ascend/ascend-toolkit/set_env.sh && export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/aarch64-linux-gnu && cmake --build /home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V021/c50d6f4a/build --target paired_runner_v016 -- -j2'
BUILD_RETRY_01_EXIT_CODE=2
RUNNER_BINARY_SHA256=bbd673d981aaa371e1d65a288c7b7939b69c2afcffa1a8d9e6e0e0e47e794bbf (unchanged original binary)
BUILD_STOP_REASON=No space left on device; /home/data4t2 mounted volume has 0 bytes available
LOG=results/fp32-runner-build-v021.log,results/fp32-runner-build-v021-retry-01.log
