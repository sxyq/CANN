# N05 V004 Compile

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V004
PARENT=V003
COMPILE=PASS
CONFIGURE_RC=0
BUILD_RC=0
FINAL_RC=0
SSH_RC=0
REMOTE_HOST=hwnput3
REMOTE_ALIAS=cann-server3
REMOTE_DIR=/tmp/n05-scalar-address-generation-v004.6C56bn
SOC_VERSION=Ascend910B3
ENV_SOURCE=/usr/local/Ascend/ascend-toolkit/set_env.sh
ENV_SOURCE_RC=0
ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest
CMAKE_ENTRY=本地实验/R31B/V016/support/CMakeLists.txt
CMAKE_INPUTS=existing V016 CMakeLists.txt,device.cpp,adapter.cpp,link_one.sh,runtime_probe.cpp; V004 candidate uploaded as R31B-V016-WIDE-TILE-SEED_kernel.asc
TARGETS=device,submission,full_link
COMMAND=source /usr/local/Ascend/ascend-toolkit/set_env.sh; export SOC_VERSION=Ascend910B3; cmake -S /tmp/n05-scalar-address-generation-v004.6C56bn -B /tmp/n05-scalar-address-generation-v004.6C56bn/build -DCMAKE_BUILD_TYPE=Release; cmake --build /tmp/n05-scalar-address-generation-v004.6C56bn/build --target device submission full_link -j2
RAW_LOG=compile-attempt-01.log
RESOURCE_LOG=compile-attempt-01.log; pre/post npu-smi snapshots are preserved in the raw log
FIRST_FAILURE=NONE
RUNTIME_RUN=NONE
REMOTE_TEMP_CLEANUP=PASS; REMOTE_TEMP_CLEANUP_RC=0

The existing V016 CMake entry was reused without adding a Runner or changing the device runtime path. `runtime_probe.cpp` was supplied as an existing CMake input; its executable target was not built or run. No device reset was called.

Pre-build and post-build snapshots both showed no running processes on NPU2 and NPU3. Existing processes on NPU0, NPU1, NPU4, NPU5, NPU6, and NPU7 were left untouched.

Build output evidence is in `build-artifacts.txt`. The complete remote command output, CMake output, linker output, snapshots, return codes, and artifact metadata are in `compile-attempt-01.log`.
