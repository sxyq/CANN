# V001 build blocker evidence

ROUTE=WRITEBACK-TRANSACTION-X
REVISION=V001
BUILD_STATUS=BLOCKED_ENVIRONMENT

SERVER3_COMMAND=`ssh cann-server3 'hostname; df -h /home/data4t2; find /home/data4t2/lelinfeng -maxdepth 5 -type f ( -name compile.sh -o -name build.sh -o -name CMakeLists.txt -o -name run_correctness.sh ) -print'`
SERVER3_RESULT=`Temporary failure in name resolution`

LOCAL_COMMAND=`本地实验/WRITEBACK-TRANSACTION-X/V001/support/build-local.sh`
LOCAL_RESULT=`/usr/local/Ascend/ascend-toolkit/8.5.0.alpha002/aarch64-linux/script/set_env.sh: line 1: LD_LIBRARY_PATH: unbound variable`
LOCAL_RC=1

CORRECTNESS=NOT_STARTED
LOCAL=NOT_STARTED
ONLINE=FORBIDDEN
DECISION=STOP_V001_PENDING_ENVIRONMENT_RETRY
