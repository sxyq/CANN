# N05 V002 Compile

ROUTE=N05-SCALAR-ADDRESS-GENERATION-X
REVISION=V002
PARENT=V001
COMPILE=PASS
REMOTE_HOST=hwnput3
REMOTE_ALIAS=cann-server3
REMOTE_DIR=/tmp/n05-scalar-address-generation-v002.vjJtPm
SOC_VERSION=Ascend910B3
ENV_SOURCE=/usr/local/Ascend/ascend-toolkit/set_env.sh
ENV_SOURCE_RC=0
ASCEND_HOME_PATH=/usr/local/Ascend/ascend-toolkit/latest
TARGETS=device,submission,full_link
REMOTE_TEMP_CLEANUP=PASS

## Attempt 1

COMMAND=source /usr/local/Ascend/ascend-toolkit/set_env.sh; export SOC_VERSION=Ascend910B3; cmake -S /tmp/n05-scalar-address-generation-v002.vjJtPm -B /tmp/n05-scalar-address-generation-v002.vjJtPm/build -DCMAKE_BUILD_TYPE=Release
CONFIGURE_RC=1
REASON=The existing CMakeLists.txt declares probe_v016 and probe_v011, but runtime_probe.cpp was missing from the temporary input directory.
RAW_LOG=compile-attempt-01.log

## Attempt 2

COMMAND=source /usr/local/Ascend/ascend-toolkit/set_env.sh; export SOC_VERSION=Ascend910B3; cmake -S /tmp/n05-scalar-address-generation-v002.vjJtPm -B /tmp/n05-scalar-address-generation-v002.vjJtPm/build -DCMAKE_BUILD_TYPE=Release; cmake --build /tmp/n05-scalar-address-generation-v002.vjJtPm/build --target device submission full_link -j2
CONFIGURE_RC=0
BUILD_RC=0
FINAL_RC=0
SSH_RC=0
RAW_LOG=compile-attempt-02.log

The second attempt reused the existing V001 CMake input set and added only the existing `runtime_probe.cpp` required during CMake generation. No probe target was built or run.

PRE_BUILD_HBM_MB=NPU0 38513; NPU1 38852; NPU2 3426; NPU3 3429; NPU4 59214; NPU5 59737; NPU6 59735; NPU7 28632
POST_BUILD_HBM_MB=NPU0 38513; NPU1 38852; NPU2 3425; NPU3 3429; NPU4 59214; NPU5 59736; NPU6 59735; NPU7 28632
PRE_POST_NPU_PROCESS_TABLE=UNCHANGED_IN_NPU_SMI_SNAPSHOT
FILESYSTEM=/home/data4t2; 95G free; 98% used before and after

Build output:

- `device.alink`: 183144 bytes; SHA256 `befdc9303b27d3f6b36dd25c2cc302da3c9952bc51d0db9ea374fedf9e295e66`
- `submission.alink`: 183144 bytes; SHA256 `df77a2297a73771e83b276f8a8c6c2d8d43424cb889243902f9d6f93ef75a921`
- `device.cpp.o`: 334088 bytes; SHA256 `9cb9d4b67017d3c5fd68e2e0c9df1b5f330e0b70aabb658f229929ede1e6bb5c`
- `adapter.cpp.o`: 334088 bytes; SHA256 `ba4561f57ad9642d596048e528268e9aa47ae55cfb986437ed74c8269e2590e2`

The temporary directory was removed after the raw logs and artifact facts were saved.
