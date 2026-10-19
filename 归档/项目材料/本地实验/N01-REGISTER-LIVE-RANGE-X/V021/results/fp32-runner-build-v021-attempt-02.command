REVISION=V021
ATTEMPT=02
HOST=hwnput3
REMOTE_RUN=/home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V021/c50d6f4a
RUNNER_TARGET=paired_runner_v016
RUNNER_SOURCE_SHA256=7836fb57a342642132a3345209796665b7fa900b599d3842f37560b446b5ba3b
CANDIDATE_SOURCE_SHA256=d4f9994b9eb43e75b454143cbdb3f2b8a2222545d6ad4496fc6022c1682e5134
PARENT_SOURCE_SHA256=a8c19a1972207acc67e3fb0cd393cc70b0a4b183d1eaf5610edf80c2879b15e3
PRE_BUILD_SNAPSHOT=results/fp32-pre-build-v021.txt
BUILD_COMMAND=ssh cann-server3 'source /usr/local/Ascend/ascend-toolkit/set_env.sh && export CPLUS_INCLUDE_PATH=/usr/include/c++/11:/usr/include/aarch64-linux-gnu/c++/11:/usr/include/aarch64-linux-gnu && cmake --build /home/data4t2/lelinfeng/cann/server_runs/N01-REGISTER-LIVE-RANGE-X-RESET-20261011/V021/c50d6f4a/build --target paired_runner_v016 -- -j2'
EXIT_CODE=0
RUNNER_BINARY_SHA256=88025ed8e024a4b223f289799a038d82b1837daf26bc97bffc6a337f06436730
BUILD_LOG=results/fp32-runner-build-v021-attempt-02.log
