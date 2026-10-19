REVISION=V021
ATTEMPT=04
HOST=hwnput3
REMOTE_RUN=/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V021/c50d6f4a
BUILD_DIRECTORY=/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V021/c50d6f4a/build
RUNNER_TARGET=paired_runner_v016
RUNNER_SOURCE_LOCAL=本地实验/N01-REGISTER-LIVE-RANGE-X/V021/results/paired_runner-v021.cpp
RUNNER_SOURCE_REMOTE=/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V021/c50d6f4a/support/paired/paired_runner.cpp
RUNNER_SOURCE_SHA256=f077541d73c4819d8a8299c85fd489b70c26a44373e396880d1f1664ac9f979e
CANDIDATE_SOURCE_SHA256=d4f9994b9eb43e75b454143cbdb3f2b8a2222545d6ad4496fc6022c1682e5134
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
UPLOAD_COMMAND=scp '本地实验/N01-REGISTER-LIVE-RANGE-X/V021/results/paired_runner-v021.cpp' cann-server3:/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V021/c50d6f4a/support/paired/paired_runner.cpp
BUILD_COMMAND=source /usr/local/Ascend/ascend-toolkit/set_env.sh; export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/aarch64-linux-gnu; cmake --build /home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V021/c50d6f4a/build --target paired_runner_v016 -- -j2
PRE_BUILD_FREE_KIB=6840932
EXIT_CODE=0
RUNNER_BINARY_SHA256=b3bd4a4b852647893f74911b9ac6fa0fa00261172fbef9f0246879111675d9ab
BUILD_LOG=results/fp32-adapter-runner-build-v021-attempt-04.log
POST_BUILD_IDENTITY=results/fp32-adapter-build-identity-v021-attempt-04.txt
