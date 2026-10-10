# N05 V006 Compile

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V006
PARENT=V004
PARENT_COMMIT=0020dafb77638d4cd4283037d240f4bce8d29eb8
COMPILE=PASS
CONFIGURE_RC=0
BUILD_RC=0
FINAL_RC=0
SSH_RC=0
REMOTE_HOST=cann-server3
REMOTE_DIR=/tmp/n05-scalar-address-generation-v006.1k14VR
SOC_VERSION=Ascend910B3
ENV_SOURCE=/usr/local/Ascend/ascend-toolkit/set_env.sh
CMAKE_ENTRY=本地实验/R31B/V016/support/CMakeLists.txt
CMAKE_INPUTS=existing V016 CMakeLists.txt,device.cpp,adapter.cpp,link_one.sh,runtime_probe.cpp; V006 candidate uploaded as R31B-V016-WIDE-TILE-SEED_kernel.asc
TARGETS=device,submission,full_link
CANDIDATE_LOCAL_BYTES=193973
CANDIDATE_LOCAL_SHA256=81e8b3e953321c43d4e0f4838558221497d377ddf21e6104016f6dbdaf9aa149
CANDIDATE_REMOTE_ALIAS=R31B-V016-WIDE-TILE-SEED_kernel.asc
CANDIDATE_REMOTE_ALIAS_SHA256=81e8b3e953321c43d4e0f4838558221497d377ddf21e6104016f6dbdaf9aa149
RAW_LOG=compile-attempt-01.log
RESOURCE_LOG=compile-attempt-01.log
FIRST_FAILURE=NONE
RUNTIME_RUN=runtime-run-npu5.log
REMOTE_TEMP_CLEANUP=PASS; REMOTE_TEMP_CLEANUP_RC=0
ARTIFACTS=device.alink,submission.alink,device.cpp.o,adapter.cpp.o
DEVICE_ALINK_SHA256=0e2474fc03ffe2483e25467a1be7610fa2d04aa88e27a33bfcbc91b1f2a3e112; SIZE=192040
SUBMISSION_ALINK_SHA256=5c38646f92017d8a45b563b9365cd0b9fae5dea80cf23fd6ea0b26414a4d1a81; SIZE=192048
DEVICE_OBJECT_SHA256=77fae3e8d18fcd80a1fbf3844d8b439a93bc43b7bda532695d8f6269fd700638; SIZE=354808
SUBMISSION_OBJECT_SHA256=fd8a43793e56999f61ecdc30c9e706cdcf8beb2fff1e3d19b389980af4d1714c; SIZE=354808

COMMAND=source /usr/local/Ascend/ascend-toolkit/set_env.sh; export SOC_VERSION=Ascend910B3; cmake -S /tmp/n05-scalar-address-generation-v006.1k14VR -B /tmp/n05-scalar-address-generation-v006.1k14VR/build -DCMAKE_BUILD_TYPE=Release; cmake --build /tmp/n05-scalar-address-generation-v006.1k14VR/build --target device submission full_link -j2
