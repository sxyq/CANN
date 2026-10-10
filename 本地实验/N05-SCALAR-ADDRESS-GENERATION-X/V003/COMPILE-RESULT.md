# N05 V003 Compile

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V003
PARENT=V002
COMPILE=PASS
CONFIGURE_RC=0
BUILD_RC=0
FINAL_RC=0
SSH_RC=0
REMOTE_HOST=hwnput3
REMOTE_ALIAS=cann-server3
REMOTE_DIR=/tmp/n05-scalar-address-generation-v003.fW3RKE
SOC_VERSION=Ascend910B3
ENV_SOURCE=/usr/local/Ascend/ascend-toolkit/set_env.sh
ENV_SOURCE_RC=0
ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest
TARGETS=device,submission,full_link
RAW_LOG=compile-attempt-01.log
RESOURCE_LOG=resource-snapshot-20261010T1430Z.log
FIRST_FAILURE=NONE
REMOTE_TEMP_CLEANUP=PASS

The existing V002 CMake input set and compile targets were reused. `runtime_probe.cpp` was included because the existing CMake file declares probe targets; no probe target was built or run.

PRE_BUILD_HBM_MB=NPU0 38512; NPU1 38852; NPU2 3426; NPU3 3429; NPU4 59213; NPU5 59737; NPU6 59735; NPU7 28631
POST_BUILD_HBM_MB=NPU0 38512; NPU1 38852; NPU2 3426; NPU3 3429; NPU4 59213; NPU5 59737; NPU6 59735; NPU7 28631
NPU2_3_PROCESS_STATE=No running processes in pre/post npu-smi snapshots
NPU0_1_4_5_6_7_PROCESS_STATE=Existing processes in pre/post npu-smi snapshots
FILESYSTEM=/home/data4t2; 95G free; 98% used before and after

Build output:

- `device.alink`: 183144 bytes; SHA256 `56d5674b155cd3c43d4e0f16bb1b3db8d3c94ceb727783c2c6be37c2151e71ec`
- `submission.alink`: 183144 bytes; SHA256 `8b4d95d685b253faa0b3369627bb53f2f90f0bcbf22f6c8b07c16264e07ad9d2`
- `device.cpp.o`: 334128 bytes; SHA256 `e830355fa104fa2bd06744d9822b3bcc576288e1153c96a498e5725a51a4b3fd`
- `adapter.cpp.o`: 334128 bytes; SHA256 `8cc6acf5b7fab509abe5e7e0f737b2ae0165b54a830ab60776b17916c4130ea8`

The temporary directory was removed after the raw log and artifact facts were saved.
